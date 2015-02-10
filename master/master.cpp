#include "master.h"
#include <climits>

bool Master::createMaster() {
    string fullpath;
    int code = zk->create(MASTERPATH + "/master-", "", ZOO_OPEN_ACL_UNSAFE,
            ZOO_SEQUENCE | ZOO_EPHEMERAL, &fullpath, true);

    NOTOK_RETURN(code);

    LOG_INFO("create node %s", fullpath.c_str());
    m_master_node = get_file_name(fullpath);

    return true;
}

bool Master::checkMaster() {
    vector<string> children;
    int code = zk->getChildren(MASTERPATH, false, &children);

    NOTOK_RETURN(code);

    if (children.empty()) {
        LOG_ERROR("not found any master node");
        return false;
    }

    sort(children.begin(), children.end());
    if (m_master_node == children.front()) {
        runAsMaster();
    } else {
        vector<string>::iterator it = find(children.begin(), children.end(), m_master_node);
        if (it == children.end()) {
            LOG_ERROR("not found master:%s", m_master_node.c_str());
        }

        m_watch_node = MASTERPATH+"/"+*(--it);
        LOG_INFO("watch node:%s", m_watch_node.c_str());
        code = zk->exists(m_watch_node, true, NULL);
        NOTOK_RETURN(code);
    }

    return true;
}

void Master::deleted(const string &path) {
    LOG_INFO("delete event on path:%s", path.c_str());
    if (path == m_watch_node) {
        checkMaster();
    }
}

void Master::runAsMaster() {
    LOG_INFO("run as master %s", m_master_node.c_str());

    initWorkers();
    initTasks();

    workerWatch();
    taskWatch();
}

bool Master::initWorkers() {
    vector<string> workers;
    int code = zk->getChildren(WORKERPATH, false, &workers);
    NOTOK_RETURN(code);

    for (int i = 0; i < workers.size(); ++i) {
        vector<string> tasks;
        int code = zk->getChildren(ASSIGNPATH+"/"+workers[i], false, &tasks);
        NOTOK_RETURN(code);

        m_worker[workers[i]] = tasks.size();
        for (int j = 0; j < tasks.size(); ++j) {
            //may get one task assigned to multi workers condition
            if (!m_assign[tasks[j]].empty()) {
                LOG_ERROR("task %s assigned to two worker %s --- %s", tasks[j].c_str(), 
                        m_assign[tasks[j]].c_str(), workers[i].c_str());
            }
            m_assign[tasks[j]] = workers[i];
        }
    }
}

bool Master::initTasks() {
    vector<string> tasks;
    int code = zk->getChildren(TASKPATH, false, &tasks);
    NOTOK_RETURN(code);

    for (int i = 0; i < tasks.size(); ++i) {
        if (m_assign.find(tasks[i]) == m_assign.end()) {
            m_assign[tasks[i]] = string();
            LOG_INFO("init add task %s", tasks[i].c_str());
            addTask(tasks[i]);
        }
    }
    
    return true;
}

bool Master::taskWatch() {
    int code = zk->getChildren(TASKPATH, true, NULL);
    NOTOK_RETURN(code);

    return true;
}

bool Master::workerWatch() {
    int code = zk->getChildren(WORKERPATH, true, NULL);
    NOTOK_RETURN(code);

    return true;
}

void Master::childChange(const string &path) {
    if (path == WORKERPATH) {
        updateWorkers();
    } else if (path == TASKPATH) {
        updateTasks();
    }
}

bool Master::updateWorkers() {
    vector<string> children;
    int code = zk->getChildren(WORKERPATH, true, &children);
    NOTOK_RETURN(code);

    //find deleted worker
    set<string> workers(children.begin(), children.end());
    for (map<string, int>::iterator it = m_worker.begin(); it != m_worker.end(); ) {
        if (workers.find(it->first) == workers.end()) {
            LOG_INFO("delete worker %s", it->first.c_str());
            deleteWorker(it->first);
            m_worker.erase(it++);
        } else {
            ++it;
        }
    }

    //find added worker
    for (int i = 0; i < children.size(); ++i) {
        if (m_worker.find(children[i]) == m_worker.end()) {
            LOG_INFO("add worker %s", children[i].c_str());
            m_worker[children[i]] = 0;
        }
    }

    return true;
}

bool Master::deleteWorker(const string &work) {
    vector<string> children;
    int code = zk->getChildren(ASSIGNPATH+"/"+work, false, &children);
    NOTOK_RETURN(code);

    for (int i = 0; i < children.size(); ++i) {
        m_assign[children[i]] = string();
        addTask(children[i]);
    }
    
    return true;
}

bool Master::updateTasks() {
    vector<string> children;
    int code = zk->getChildren(TASKPATH, true, &children);
    NOTOK_RETURN(code);

    //find deleted tasks
    set<string> tasks(children.begin(), children.end());
    for (map<string, string>::iterator it = m_assign.begin(); it != m_assign.end(); ) {
        if (tasks.find(it->first) == tasks.end()) {
            LOG_INFO("delete task %s", it->first.c_str());

            deleteTask(it->first, it->second);
            //m_worker[it->second]--;
            m_assign.erase(it++);
        } else {
            ++it;
        }
    }

    for (int i = 0; i < children.size(); ++i) {
        if (m_assign.find(children[i]) == m_assign.end()) {
            LOG_INFO("add task %s", children[i].c_str());

            m_assign[children[i]] = string();
            addTask(children[i]);
        }
    }

    return true;
}

bool Master::deleteTask(const string &task, const string &worker) {
    if (m_worker.find(worker) == m_worker.end())  
        return true;

    int code = zk->remove(ASSIGNPATH+"/"+worker+"/"+task, -1);
    NOTOK_RETURN(code);

    m_worker[worker]--;
    return true;
}

bool Master::addTask(const string &task) {
    if (m_worker.empty()) {
        LOG_ERROR("no worker to assign task %s", task.c_str());
        return false;
    }

    //find minimal load worker to assign task
    int minLoad = INT_MAX;
    string minWorker;
    for (map<string, int>::iterator it = m_worker.begin(); it != m_worker.end(); ++it) {
        if (it->second < minLoad) {
            minWorker = it->first;
            minLoad = it->second;
        }
    }

    int code = zk->create(ASSIGNPATH+"/"+minWorker+"/"+task, "", ZOO_OPEN_ACL_UNSAFE, 0, NULL);
    NOTOK_RETURN(code);

    m_worker[minWorker]++;
    m_assign[task] = minWorker;

    return true;
}

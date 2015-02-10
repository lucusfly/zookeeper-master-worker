#include "worker.h"
#include <boost/lexical_cast.hpp>

bool Worker::createWorkspace() {
    string fullpath;
    int code = zk->create(ASSIGNPATH + "/work-", "", ZOO_OPEN_ACL_UNSAFE,
            ZOO_SEQUENCE, &fullpath, true);

    LOG_INFO("worker node:%s", fullpath.c_str());
    m_worker_node = get_file_name(fullpath);

    if (code == ZOK) {
        m_assign_dir = ASSIGNPATH+"/"+m_worker_node;
        return true;
    } else {
        return false;
    }
}

bool Worker::createWorker() {
    int code = zk->create(WORKERPATH+"/"+m_worker_node, "", ZOO_OPEN_ACL_UNSAFE,
            ZOO_EPHEMERAL, NULL, true);

    return code == ZOK;
}

bool Worker::getTasks() {
    vector<string> children;
    int code = zk->getChildren(m_assign_dir, true, &children);

    //LOG_INFO("got children:%d task num:%d", children.size(), m_tasks.size());

    if (code != ZOK) return false;

    //find deleted task
    set<string> tasks(children.begin(), children.end());
    for (map<string, Task*>::iterator it = m_tasks.begin(); it != m_tasks.end(); ) {
        if (tasks.find(it->first) == tasks.end()) {
            //deleted task
            delete it->second;
            LOG_INFO("delete task:%s", it->first.c_str());
            m_tasks.erase(it++);
        } else {
            ++it;
        }
    }

    //find added task
    for (int i = 0; i < children.size(); ++i) {
        if (m_tasks.find(children[i]) == m_tasks.end()) {
            Task *info = getTaskInfo(children[i]);
            if (info != NULL) {
                //new task
                LOG_INFO("add task:%s", children[i].c_str());
                m_tasks[children[i]] = info;
                RunTask(children[i]);
            }
        }
    }

    return true;
}

Task *Worker::getTaskInfo(const string &task) {
    string info;
    int code = zk->get(TASKPATH+"/"+task, false, &info, NULL);

    if (code != ZOK || info.size() == sizeof(Task)) {
        return NULL;
    }

    Task *taskInfo = new Task();
    memcpy(taskInfo, info.data(), info.size());
    return taskInfo;
}

bool Worker::setLoad(int load) {
    int code = zk->set(WORKERPATH+"/"+m_worker_node, boost::lexical_cast<string>(load), -1);

    return code == ZOK;
}

void Worker::RunTask(const string &taskNode) {
    LOG_INFO("run task:%s, task info:%s", taskNode.c_str(), m_tasks[taskNode]->info);
}

void Worker::childChange(const string &path) {
    if (path == m_assign_dir) {
        getTasks();
    }
}

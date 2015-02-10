#ifndef _MASTER_H_
#define _MASTER_H_

#include "watcher.h"
#include "zookeeper.h"
#include "common.h"
#include <map>

using namespace std;

class Master : public Watcher {
public:
    Master(ZooKeeper *zk) : Watcher(zk) {}

    bool createMaster();
    bool checkMaster();

    void runAsMaster();

    bool updateWorkers();
    bool updateTasks();

    bool addWorker(const string &worker);
    bool deleteWorker(const string &worker);
    bool addTask(const string &task);
    bool deleteTask(const string &task, const string &worker);

private:
    bool initTasks();
    bool initWorkers();

    void deleted(const string &path);
    void childChange(const string &path);

    bool taskWatch();
    bool workerWatch();

private:
    map<string, string> m_assign; //map<task, worker>
    map<string, int> m_worker;    //map<worker, load>
    string m_master_node;
    string m_watch_node;
};

#endif

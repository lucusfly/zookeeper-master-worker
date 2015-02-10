#include <string>
#include "worker.h"
#include "common.h"

using namespace std;

int main(int argc, char **argv) {
    if (process(argc, argv)) {
        return 1;
    }

    log_init(CLOG_LEVEL_INFO, "log-worker");

    string host = "192.168.85.132:2181,192.168.85.132:2182,192.168.85.132:2183";
    
    ZooKeeper zk(host, 10000);

    Worker w(&zk);
    w.startWatchThread();

    while(!w.isConnected()) {
        sleep(1);
    }

    w.createWorkspace();
    w.createWorker();
    w.getTasks();

    while(!w.isExpired()) {
        sleep(1);
    }

    return 0;
}

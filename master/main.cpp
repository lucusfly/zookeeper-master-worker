#include <string>
#include "master.h"

using namespace std;

int main(int argc, char **argv) {
    if (process(argc, argv)) {
        return 1;
    }

    log_init(CLOG_LEVEL_INFO, "log-master");

    string host = "192.168.85.132:2181,192.168.85.132:2182,192.168.85.132:2183";
    ZooKeeper zk(host, 10000);

    Master m(&zk);
    m.startWatchThread();

    while(!m.isConnected()) {
        sleep(1);
    }

    m.createMaster();
    m.checkMaster();

    while(!m.isExpired()) {
        sleep(1);
    }

    return 0;
}

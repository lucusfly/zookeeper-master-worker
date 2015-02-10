#include "zookeeper.h"
#include "common.h"
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "please input dir" << endl;
        cout << "\t usage:./clearDir dir" << endl;
        return 0;
    }

    string host = "192.168.85.132:2181,192.168.85.132:2182,192.168.85.132:2183";
    
    ZooKeeper *zk = new ZooKeeper(host, 10000);

    int code = zk->removeDir(string(argv[1]));

    if (code == ZOK) {
        cout << "clear /assign success" << endl;
    } else {
        cout << "clear /assign error: " << zerror(code) << endl;
    }
}

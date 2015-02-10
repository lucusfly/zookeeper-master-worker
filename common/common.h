#ifndef _COMMON_H_
#define _COMMON_H_

#include <zookeeper/zookeeper.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "daemon.h"

using std::cout;
using std::endl;
using std::ifstream;

static const std::string MASTERPATH = "/masters";
static const std::string WORKERPATH = "/workers";
static const std::string ASSIGNPATH = "/assign";
static const std::string TASKPATH = "/tasks";

typedef struct Task {
    char info[20];
}Task;

//copy String_vector to stl vector
inline void copy_vector(const struct String_vector *vector, std::vector<std::string> &vs) {
    for (int i = 0; i < vector->count; ++i) {
        vs.push_back(std::string(vector->data[i]));
    }
}

//parse file path to path name and file name
inline std::pair<std::string, std::string> parse_path(const std::string &path) {
    size_t index = path.find_last_of('/');
    
    if (index == std::string::npos) {
        return std::make_pair("", path);
    }

    return std::make_pair(path.substr(0, index+1), path.substr(index+1));
}

inline std::string get_file_name(const std::string &path) {
    return parse_path(path).second;
}

inline std::string get_dir_name(const std::string &path) {
    return parse_path(path).first;
}

#define NOTOK_RETURN(errorCode) if (errorCode != ZOK) { \
    LOG_ERROR("zookeeper got a Error:%s", zerror(errorCode)); \
    return false; \
}

inline bool process(int argc, char **argv) {
    if (argc == 2) {
        if (!strcmp(argv[1],"-stop")) {
            ifstream ifs(".daemon.pid");
            int pid;
            ifs >> pid;
            cout << "process id :" << pid << endl;
            if(!kill(pid, 9)) {
                cout << "stop sucess" << endl;
            } else {
                cout << "stop failed, check if the process is running" << endl;
            }
        } else {
            cout << "usage:./master -stop" << endl;
        }
        return true;
    }

    daemonize();
    if (isrunning()) {
        cout << "already running" << endl;
        return true;
    }

    return false;
}

#endif

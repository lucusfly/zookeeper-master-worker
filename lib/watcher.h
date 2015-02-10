#ifndef _WATCH_H_
#define _WATCH_H_

#include <stdint.h>
#include "zookeeper.h"
#include <boost/thread/thread.hpp>
#include "clog.h"

class Watcher
{
public:
    Watcher(ZooKeeper *zk):is_connected_(false), is_expired_(true), zk(zk) {
    }

    bool isConnected() {
        return is_connected_;
    }

    bool isExpired() {
        return is_expired_;
    }

    //handle message
    void operator()() {
        WatchMsg *msg;
        while((msg = zk->waitWatch()) != NULL) {
            process(msg->type, msg->state, msg->path);

            delete msg;
        }
    }

    void startWatchThread() {
        boost::thread thread1(boost::ref(*this));
    }

protected:
    virtual void connected() {
        LOG_WARN("got a connected event");//if not handle,record it
    }

    virtual void reconnecting() {
        LOG_WARN("got a reconnecting event");//if not handle,record it
    }

    virtual void expired() {
        LOG_WARN("got a expired event");//if not handle,record it
    }

    virtual void childChange(const std::string& path) {
        LOG_WARN("got a child update in path:%s", path.c_str());//if not handle,record it
    }

    virtual void dataChange(const std::string &path) {
        LOG_WARN("got a data change event in path:%s", path.c_str());//if not handle,record it
    }

    virtual void created(const std::string &path) {
        LOG_WARN("got a created event in path:%s", path.c_str());//if not handle,record it
    }
    
    virtual void deleted(const std::string &path) {
        LOG_WARN("got a deleted event in path:%s", path.c_str());//if not handle,record it
    }

    virtual void process(int type, int state, const std::string& path) {
        if (type == ZOO_SESSION_EVENT) {
            if (state == ZOO_CONNECTED_STATE) {
                connected();
                is_connected_ = true;
                is_expired_ = false;
            } else if (state == ZOO_CONNECTING_STATE) {
                reconnecting();
                is_connected_ = true;
            } else if (state == ZOO_EXPIRED_SESSION_STATE) {
                expired();
                is_connected_ = false;
                is_expired_ = true;
            } else {
                LOG_ERROR("Unhandled ZooKeeper state (%d) for ZOO_SESSION_EVENT", state);
            }
        } else if (type == ZOO_CHILD_EVENT) {
            childChange(path);
        } else if (type == ZOO_CHANGED_EVENT) {
            dataChange(path);
        } else if (type == ZOO_CREATED_EVENT) {
            created(path);
        } else if (type == ZOO_DELETED_EVENT) {
            deleted(path);
        } else {
            LOG_ERROR("Unhandled ZooKeeper event (%d)  in state (%d)", type, state);
        }
    }

protected:
  ZooKeeper *zk;

private:
  bool is_connected_;
  bool is_expired_;
};

#endif // _WATCHHANDLER_H_

/**
 * ZooKeeper C++ API.
 *
 * author: lucusfly
 *
 * date:2015-01-30
 */

#include "zookeeper.h"

#include <boost/thread/future.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include "clog.h"
//#include "path.h"

using namespace boost;
using namespace std;

ZooKeeper::ZooKeeper(const string& servers, int timeout) : zh(NULL) {
    //try 10 times
    for (int i = 0; i < 10; ++i) {
        zh = zookeeper_init(servers.c_str(), event, timeout, NULL, &msgQ, 0);

        // Unfortunately, EINVAL is highly overloaded in zookeeper_init
        // and can correspond to:
        // (1) Empty / invalid 'host' string format.
        // (2) Any getaddrinfo error other than EAI_NONAME,
        //     EAI_NODATA, and EAI_MEMORY are mapped to EINVAL.
        if (zh == NULL && errno == EINVAL) {
            LOG_INFO("zookeeper_init failed:%s; retrying in 1 second", strerror(errno));
            sleep(1);
            continue;
        }

        break;
    }

    if (zh == NULL) {
        LOG_ERROR("Failed to create ZooKeeper, zookeeper_init");
    }
}

ZooKeeper::~ZooKeeper() {
    int ret = zookeeper_close(zh);
    if (ret != ZOK) {
        LOG_ERROR("Failed to cleanup ZooKeeper, zookeeper_close: %s", zerror(ret));
    }
}

int ZooKeeper::getState() {
    return zoo_state(zh);
}

int64_t ZooKeeper::getSessionId() {
    return zoo_client_id(zh)->client_id;
}

int ZooKeeper::getSessionTimeout() const {
    return zoo_recv_timeout(zh);
}

int ZooKeeper::authenticate(const string& scheme, const string& credentials) {
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*>* args = new tuple<promise<int>*>(pi);

    int ret = zoo_add_auth(zh, scheme.c_str(), credentials.data(),
            credentials.size(), voidCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        authenticate(scheme, credentials);
    } else {
        return fi.get();
    }
}

int ZooKeeper::_create(const string& path, const string& data, const ACL_vector& acl,
        int flags, string* result) {
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*, string*>* args =
        new tuple<promise<int>*, string*>(pi, result);

    int ret = zoo_acreate(zh, path.c_str(), data.data(), data.size(),
            &acl, flags, stringCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        _create(path, data, acl, flags, result);
    } else {
        return fi.get();
    }
}

int ZooKeeper::create(const string& path, const string& data, const ACL_vector& acl,
        int flags, string* result, bool recursive) {
    if (!recursive) {
        return _create(path, data, acl, flags, result);
    }

    string parent = path.substr(0, path.find_last_of("/"));
    if (!parent.empty()) {
        int code = exists(parent, -1, NULL);
        if (code == ZNONODE) {
            int create_code = create(parent, "", acl, 0, NULL, recursive);
            if (create_code != ZOK) {
                return create_code;
            }
        }
    }

    return _create(path, data, acl, flags, result);
}

int ZooKeeper::remove(const string& path, int version)
{
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*>* args = new tuple<promise<int>*>(pi);

    int ret = zoo_adelete(zh, path.c_str(), version, voidCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        remove(path, version);
    } else {
        return fi.get();
    }
}

int ZooKeeper::removeDir(const string& path) {
    vector<string> children;
    int code = getChildren(path, false, &children);

    if (code == ZOK) {
        if (children.empty()) {
            return remove(path, -1);
        } else {
            for (int i = 0; i < children.size(); ++i) {
                int remove_code = removeDir(path + "/" + children[i]);

                if (remove_code != ZOK)
                    return remove_code;
            }

            return remove(path, -1);
        }
    } else {
        return code;
    }
}

int ZooKeeper::exists(const string& path, bool watch, Stat* stat) {
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*, Stat*>* args =
        new tuple<promise<int>*, Stat*>(pi, stat);

    int ret = zoo_aexists(zh, path.c_str(), watch, statCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        exists(path, watch, stat);
    } else {
        return fi.get();
    }
}

int ZooKeeper::get(const string& path, bool watch, string* result, Stat* stat)
{
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*, string*, Stat*>* args =
        new tuple<promise<int>*, string*, Stat*>(pi, result, stat);

    int ret = zoo_aget(zh, path.c_str(), watch, dataCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        get(path, watch, result, stat);
    } else {
        return fi.get();
    }
}

int ZooKeeper::getChildren(const string& path, bool watch, vector<string>* results)
{
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*, vector<string>*>* args =
        new tuple<promise<int>*, vector<string>*>(pi, results);

    int ret = zoo_aget_children(zh, path.c_str(), watch, stringsCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    int code = fi.get();
    if(retryable(code)) {
        LOG_WARN("got a retry cause %s", zerror(code));
        getChildren(path, watch, results);
    } else {
        return code;
    }
}

int ZooKeeper::set(const string& path, const string& data, int version)
{
    promise<int>* pi = new promise<int>();
    unique_future<int> fi = pi->get_future();

    tuple<promise<int>*, Stat*>* args =
        new tuple<promise<int>*, Stat*>(pi, NULL);

    int ret = zoo_aset(zh, path.c_str(), data.data(), data.size(),
            version, statCompletion, args);

    if (ret != ZOK) {
        delete pi;
        delete args;
        return ret;
    }

    if(retryable(fi.get())) {
        LOG_WARN("got a retry cause %s", zerror(fi.get()));
        set(path, data, version);
    } else {
        return fi.get();
    }
}

WatchMsg *ZooKeeper::waitWatch() {
    return msgQ.pop(true);
}

void ZooKeeper::event(zhandle_t* zh, int type, int state, const char* path,
        void* context)
{
    locking_queue<WatchMsg*> *msgQueue = (locking_queue<WatchMsg*>*)context;
    WatchMsg *msg = new WatchMsg(type, state, path);
    msgQueue->push(msg);
}

void ZooKeeper::voidCompletion(int ret, const void *data)
{
    const tuple<promise<int>*>* args = 
        reinterpret_cast<const tuple<promise<int>*>*>(data);

    promise<int>* pi = tuples::get<0>(*args);

    pi->set_value(ret);

    delete pi;
    delete args;
}

void ZooKeeper::stringCompletion(int ret, const char* value, const void* data)
{
    const tuple<promise<int>*, string*> *args =
        reinterpret_cast<const tuple<promise<int>*, string*>*>(data);

    promise<int>* pi = tuples::get<0>(*args);
    string* result = tuples::get<1>(*args);

    if (ret == 0) {
        if (result != NULL) {
            result->assign(value);
        }
    }

    pi->set_value(ret);

    delete pi;
    delete args;
}

void ZooKeeper::statCompletion(int ret, const Stat* stat, const void* data)
{
    const tuple<promise<int>*, Stat*>* args =
        reinterpret_cast<const tuple<promise<int>*, Stat*>*>(data);

    promise<int>* pi = tuples::get<0>(*args);
    Stat *stat_result = tuples::get<1>(*args);

    if (ret == 0) {
        if (stat_result != NULL) {
            *stat_result = *stat;
        }
    }

    pi->set_value(ret);

    delete pi;
    delete args;
}

void ZooKeeper::dataCompletion(int ret, const char* value, int value_len,
        const Stat* stat, const void* data)
{
    const tuple<promise<int>*, string*, Stat*>* args =
        reinterpret_cast<const tuple<promise<int>*, string*, Stat*>*>(data);

    promise<int>* pi = tuples::get<0>(*args);
    string* result = tuples::get<1>(*args);
    Stat* stat_result = tuples::get<2>(*args);

    if (ret == 0) {
        if (result != NULL) {
            result->assign(value, value_len);
        }

        if (stat_result != NULL) {
            *stat_result = *stat;
        }
    }

    pi->set_value(ret);

    delete pi;
    delete args;
}

void ZooKeeper::stringsCompletion(int ret, const String_vector* values,
        const void* data)
{
    const tuple<promise<int>*, vector<string>*>* args =
        reinterpret_cast<const tuple<promise<int>*, vector<string>*>*>(data);

    promise<int>* pi = tuples::get<0>(*args);
    vector<string>* results = tuples::get<1>(*args);

    if (ret == 0) {
        if (results != NULL) {
            results->clear();
            for (int i = 0; i < values->count; i++) {
                results->push_back(values->data[i]);
            }
        }
    }

    pi->set_value(ret);

    delete pi;
    delete args;
}

string ZooKeeper::message(int code) const
{
    return string(zerror(code));
}

bool ZooKeeper::retryable(int code)
{
    switch (code) {
        case ZCONNECTIONLOSS:
        case ZOPERATIONTIMEOUT:
        case ZSESSIONMOVED:
            return true;

        case ZOK: // No need to retry!

        case ZSESSIONEXPIRED:

        case ZSYSTEMERROR: // Should not be encountered, here for completeness.
        case ZRUNTIMEINCONSISTENCY:
        case ZDATAINCONSISTENCY:
        case ZMARSHALLINGERROR:
        case ZUNIMPLEMENTED:
        case ZBADARGUMENTS:
        case ZINVALIDSTATE:

        case ZAPIERROR: // Should not be encountered, here for completeness.
        case ZNONODE:
        case ZNOAUTH:
        case ZBADVERSION:
        case ZNOCHILDRENFOREPHEMERALS:
        case ZNODEEXISTS:
        case ZNOTEMPTY:
        case ZINVALIDCALLBACK:
        case ZINVALIDACL:
        case ZAUTHFAILED:
        case ZCLOSING:
        case ZNOTHING: // Is this used? It's not exposed in the Java API.
            return false;

        default:
            LOG_ERROR("Unknown ZooKeeper code: %d", code);
            return false;
            //UNREACHABLE(); // Make compiler happy.
    }
}

/**
 * ZooKeeper C++ API.
 *
 * author: lucusfly
 *
 * date:2015-01-30
 *
*/
#ifndef _ZOOKEEPER_H_
#define _ZOOKEEPER_H_

#include <string>
#include <vector>

#include <zookeeper/zookeeper.h>
#include "locking_queue.h"

using std::string;
using std::vector;

using boost::locking_queue;

typedef struct WatchMsg {
    int type;
    int state;
    string path;

    WatchMsg(int t, int s, const char *p):type(t), state(s), path(p) {}
} WatchMsg;

//this is a zookeeper c++ client implement. it bases zookeeper 
//c-binding client and boost. 
//comparing with c-binding client, some convenience being added:
//(1) all client requests send and get synchronously. 
//(2) if return code is retryable, it will auto retry
//(3) one class instance handle all interactions with zookeeper server
// 
//but the lose benifit is asynchronization, which will cause application
//slow if network is not good
class ZooKeeper
{
public:
  ZooKeeper(const string& servers, int timeout);
  ~ZooKeeper();

  int getState();
  int64_t getSessionId();
  int getSessionTimeout() const;

  int authenticate(const string& scheme, const string& credentials);

  /*
   * @param acl is always ZOO_OPEN_ACL_UNSAFE
   * @parma flags can be ZOO_SEQUENCE or ZOO_EPHEMERAL
   *
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the parent node does not exist.
   * ZNODEEXISTS the node already exists
   * ZNOAUTH the client does not have permission.
   * ZNOCHILDRENFOREPHEMERALS cannot create children of ephemeral nodes.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int create(const string& path, const string& data, const ACL_vector& acl,
          int flags, string* result, bool recursive = false);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADVERSION expected version does not match actual version.
   * ZNOTEMPTY children are present; node cannot be deleted.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int remove(const string& path, int version);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADVERSION expected version does not match actual version.
   * ZNOTEMPTY children are present; node cannot be deleted.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int removeDir(const string &path);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int exists(const string& path, bool watch, Stat* stat);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int get(const string& path, bool watch, string* result,
      Stat* stat);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed successfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - state is ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int getChildren(const string& path, bool watch, 
          vector<string>* results);

  /*
   * @return one of the following values is returned:
   * ZOK operation completed succesfully
   * ZNONODE the node does not exist.
   * ZNOAUTH the client does not have permission.
   * ZBADVERSION expected version does not match actual version.
   * ZBADARGUMENTS - invalid input parameters
   * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
   * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
   */
  int set(const string& path, const string& data, int version);

  //return a message describing the return code, similar with zerrror
  string message(int code) const;

  //return bool indicating whether operation can be retried.
  bool retryable(int code);

  WatchMsg* waitWatch();

private:
  int _create(const string& path, const string& data, const ACL_vector& acl,
          int flags, string* result);

  // This method is push a watcher message in locking queue
  static void event(zhandle_t* zh, int type, int state, const char* path, void* context);

  static void voidCompletion(int ret, const void *data);
  static void stringCompletion(int ret, const char* value, const void* data);
  static void statCompletion(int ret, const Stat* stat, const void* data);
  static void dataCompletion(int ret, const char* value, int value_len,
          const Stat* stat, const void* data);
  static void stringsCompletion(int ret, const String_vector* values,const void* data);

  //ZooKeeper instances are not copyable
  ZooKeeper(const ZooKeeper& that);
  ZooKeeper& operator = (const ZooKeeper& that);

private:
  zhandle_t* zh; // ZooKeeper connection handle

  locking_queue<WatchMsg*> msgQ;
};


#endif

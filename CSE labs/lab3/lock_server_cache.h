#ifndef lock_server_cache_h
#define lock_server_cache_h

#include <string>
#include <map>
#include <queue>
#include "lock_protocol.h"
#include "rpc.h"
#include "lock_server.h"


class lock_server_cache {
 private:
  int nacquire = 0;

  struct lock_info{
    std::string cid;
    bool locked = false;
    std::queue<std::string> waiting_queue;
  };
  pthread_mutex_t mutex;
  std::map<lock_protocol::lockid_t, lock_info> lock;

public:
  lock_server_cache();
  ~lock_server_cache();
  lock_protocol::status stat(lock_protocol::lockid_t, int &);
  int acquire(lock_protocol::lockid_t, std::string id, int &);
  int release(lock_protocol::lockid_t, std::string id, int &);
};

#endif

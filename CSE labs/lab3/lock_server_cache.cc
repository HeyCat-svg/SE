// the caching lock server implementation

#include "lock_server_cache.h"
#include <sstream>
#include <stdio.h>
#include <queue>
#include <unistd.h>
#include <arpa/inet.h>
#include "lang/verify.h"
#include "handle.h"
#include "tprintf.h"


lock_server_cache::lock_server_cache(){
  pthread_mutex_init(&mutex, NULL);
}

lock_server_cache::~lock_server_cache(){
  pthread_mutex_destroy(&mutex);
}

int lock_server_cache::acquire(lock_protocol::lockid_t lid, std::string id, int &){
  lock_protocol::status ret = lock_protocol::OK;
  // Your lab2 part3 code goes here
  pthread_mutex_lock(&mutex);
  if(lock[lid].locked){
    std::string revokeid = lock[lid].cid;
    // each require operation will revoke its 
    if(!lock[lid].waiting_queue.empty()){
      revokeid = lock[lid].waiting_queue.back();
    }
    lock[lid].waiting_queue.push(id);
    pthread_mutex_unlock(&mutex);

    // send revoke message
    handle h(revokeid);
    int r;
    rpcc *cl = h.safebind();
    if(cl){
      ret = cl->call(rlock_protocol::revoke, lid, r);
      return lock_protocol::RETRY;
    }
    else{
      return lock_protocol::RPCERR;
    }
  }
  lock[lid].cid = id;
  lock[lid].locked = true;
  nacquire++;
  pthread_mutex_unlock(&mutex);
  return ret;
}

int 
lock_server_cache::release(lock_protocol::lockid_t lid, std::string id, int &r){
  lock_protocol::status ret = lock_protocol::OK;
  // Your lab2 part3 code goes here
  pthread_mutex_lock(&mutex);
  if(!lock[lid].waiting_queue.empty()){
    std::string retryid = lock[lid].waiting_queue.front();
    lock[lid].waiting_queue.pop();
    lock[lid].cid = retryid;
    lock[lid].locked = true;
    pthread_mutex_unlock(&mutex);

    // send retry message
    handle h(retryid);
    int r;
    rpcc *cl = h.safebind();
    if(cl){
      ret = cl->call(rlock_protocol::retry, lid, r);
      return lock_protocol::OK;
    }
    else{
      return lock_protocol::RPCERR;
    }
  }
  if(lock[lid].locked){
    nacquire--;
  }
  lock[lid].locked = false;
  lock[lid].cid.clear();
  pthread_mutex_unlock(&mutex);
  return ret;
}

lock_protocol::status
lock_server_cache::stat(lock_protocol::lockid_t lid, int &r){
  tprintf("stat request\n");
  r = nacquire;
  return lock_protocol::OK;
}

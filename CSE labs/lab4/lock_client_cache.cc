// RPC stubs for clients to talk to lock_server, and cache the locks
// see lock_client.cache.h for protocol details.

#include "lock_client_cache.h"
#include "rpc.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "tprintf.h"


int lock_client_cache::last_port = 0;

lock_client_cache::lock_client_cache(std::string xdst, extent_client *_ec,
				     class lock_release_user *_lu)
  : lock_client(xdst), lu(_lu), ec(_ec)
{
  srand(time(NULL)^last_port);
  rlock_port = ((rand()%32000) | (0x1 << 10));
  const char *hname;
  // VERIFY(gethostname(hname, 100) == 0);
  hname = "127.0.0.1";
  std::ostringstream host;
  host << hname << ":" << rlock_port;
  id = host.str();
  last_port = rlock_port;
  rpcs *rlsrpc = new rpcs(rlock_port);
  rlsrpc->reg(rlock_protocol::revoke, this, &lock_client_cache::revoke_handler);
  rlsrpc->reg(rlock_protocol::retry, this, &lock_client_cache::retry_handler);
  rlsrpc->reg(rlock_protocol::flush, this, &lock_client_cache::flush_handler);
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
}

lock_client_cache::~lock_client_cache(){
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
}

lock_protocol::status
lock_client_cache::acquire(lock_protocol::lockid_t lid) {
  int ret = lock_protocol::OK;
  // Your lab2 part3 code goes here
  int r;
  pthread_mutex_lock(&mutex);
  while(lock[lid].lock_status != FREE && lock[lid].lock_status != NONE){
    pthread_cond_wait(&cond, &mutex);
  }
  if(lock[lid].lock_status == FREE){
    lock[lid].lock_status = LOCKED;
    pthread_mutex_unlock(&mutex);
    return ret;
  }
  else{
    lock[lid].lock_status = ACQUIRING;
    pthread_mutex_unlock(&mutex);
    ret = cl->call(lock_protocol::acquire, lid, id, r);
    pthread_mutex_lock(&mutex);
    // server may return RETRY to client
    while(ret == lock_protocol::RETRY && lock[lid].lock_status == ACQUIRING){
      pthread_cond_wait(&cond, &mutex);
    }
    lock[lid].lock_status = LOCKED;
    pthread_mutex_unlock(&mutex);
  }
  return ret;
}

lock_protocol::status
lock_client_cache::release(lock_protocol::lockid_t lid){
  int ret = lock_protocol::OK;
  // Your lab2 part3 code goes here
  int r;
  pthread_mutex_lock(&mutex);
  if(lock[lid].revoked){    // 真的放锁
    std::vector<extent_protocol::extentid_t> flushed_list = ec->flush_cache(lid);
    std::string str = "";
    char tmp[20];
    int size = flushed_list.size();
    for (int i = 0; i < size; ++i) {
      sprintf(tmp, "%d ", (int)flushed_list[i]);
      str += tmp;
    }
    if (!str.empty()) {
      cl->call(lock_protocol::flush, id, str, r); // invalidate inode in other cache
    }

    lock[lid].lock_status = RELEASING;
    pthread_mutex_unlock(&mutex);
    ret = cl->call(lock_protocol::release, lid, id, r);
    pthread_mutex_lock(&mutex);
    lock[lid].revoked = false;
    lock[lid].lock_status = NONE;
  }
  else{
    lock[lid].lock_status = FREE;
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return ret;
}

rlock_protocol::status
lock_client_cache::revoke_handler(lock_protocol::lockid_t lid, int &){
  int ret = rlock_protocol::OK;
  // Your lab2 part3 code goes here
  pthread_mutex_lock(&mutex);
  lock[lid].revoked = true;
  if(lock[lid].lock_status == FREE){
    pthread_mutex_unlock(&mutex);
    release(lid);
  }
  else{
    pthread_mutex_unlock(&mutex);
  }

  return ret;
}

rlock_protocol::status
lock_client_cache::retry_handler(lock_protocol::lockid_t lid, int &){
  int ret = rlock_protocol::OK;
  // Your lab2 part3 code goes here
  pthread_mutex_lock(&mutex);
  lock[lid].lock_status = LOCKED;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return ret;
}

rlock_protocol::status
lock_client_cache::flush_handler(std::string str, int &) {
  const char *cur_pos = str.c_str();
  while (*cur_pos != '\0') {
    int eid = atoi(cur_pos);
    ec->clear_cache(eid);
    for (; *cur_pos != ' '; ++cur_pos);
    ++cur_pos;
  }
  return rlock_protocol::OK;
}

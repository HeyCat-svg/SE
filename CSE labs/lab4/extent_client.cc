// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <ctime>
#include <assert.h>

extent_client::extent_client(std::string dst)
{
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
}

extent_protocol::status
extent_client::create(uint32_t type, extent_protocol::extentid_t &id){
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::create, type, id);
  if (ret == extent_protocol::OK){
    cache_entry_ec c;
    extent_protocol::attr a;
    a.type = type;
    int tm = std::time(0);
    a.atime = tm;
    a.ctime = tm;
    a.mtime = tm;
    a.size = 0;

    c.attr = a;
    c.data = "";
    c.eid = id;
    c.modified = false;
    c.type = BOTH;
    put_cache(id, c);
  }
  return ret;
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  cache_entry_ec *c = find_cache(eid);
  if (c && (c->type == DATA || c->type == BOTH)) {
    buf = c->data;
  }
  else {
    ret = cl->call(extent_protocol::get, eid, buf);
    if (ret == extent_protocol::OK) {
      if (c) {
        c->data = buf;
        c->type = BOTH;
      }
      else{
        cache_entry_ec nc;
        nc.eid = eid;
        nc.data = buf;
        nc.modified = false;
        nc.type = DATA;
        put_cache(eid, nc);
      }
    }
  }
  return ret;
}

extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  cache_entry_ec *c = find_cache(eid);
  if (c && (c->type == ATTR || c->type == BOTH)) {
    attr = c->attr;
  }
  else {
    ret = cl->call(extent_protocol::getattr, eid, attr);
    if (ret == extent_protocol::OK) {
      if (c) {
        c->attr = attr;
        c->type = BOTH;
      }
      else {
        cache_entry_ec nc;
        nc.eid = eid;
        nc.attr = attr;
        nc.type = ATTR;
        nc.modified = false;
        put_cache(eid, nc);
      }
    }
  }
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  cache_entry_ec *c = find_cache(eid);
  if (c) {
    c->data = buf;
    c->modified = true;
    if (c->type == ATTR || c->type == BOTH) {
      c->attr.size = buf.size();
      int tm = std::time(0);
      c->attr.mtime = tm;
      c->attr.ctime = tm;
      c->type = BOTH;
    }
  }
  else {
    cache_entry_ec nc;
    nc.eid = eid;
    nc.data = buf;
    nc.type = DATA;
    nc.modified = true;
    put_cache(eid, nc);
  }
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  clear_cache(eid);
  flushed_list.push_back(eid);
  int r;
  // 直接删 因为flush检查的是write operation 假定不会get不存在的inode
  ret = cl->call(extent_protocol::remove, eid, r);
  return ret;
}


extent_client::cache_entry_ec *extent_client::find_cache(
  extent_protocol::extentid_t eid){
  if(cache[eid].eid != 0) {
    return &(cache[eid]);
  }
  else{
    return NULL;
  }
}

void extent_client::put_cache(extent_protocol::extentid_t eid, cache_entry_ec &c){
  assert(cache[eid].eid == 0);
  cache[eid] = c;
}

void extent_client::clear_cache(extent_protocol::extentid_t eid){
  // assert(cache[eid].eid != 0);   // 不需要 因为删除可能删两遍
  cache[eid].eid = 0;
  cache[eid].modified = false;
  cache[eid].data.clear();
}

std::vector<extent_protocol::extentid_t> extent_client::flush_cache(
  extent_protocol::extentid_t eid){
  // eids needed to tell other extent_client to invalidate
  std::vector<extent_protocol::extentid_t> ret = flushed_list;
  flushed_list.clear(); 
  if (cache[eid].eid != 0 && 
      (cache[eid].type == BOTH || cache[eid].type == DATA) &&
      cache[eid].modified) {
    int byte_written;
    cl->call(extent_protocol::put, eid, cache[eid].data, byte_written);
    ret.push_back(eid);
  }
  return ret;
}
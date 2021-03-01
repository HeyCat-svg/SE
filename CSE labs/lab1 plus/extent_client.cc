// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define CPUFREQ 2208.006        // cycles per us

static inline uint64_t get_cycle_count(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo; 
}

extent_client::extent_client()
{
  es = new extent_server();
}

extent_protocol::status
extent_client::create(uint32_t type, extent_protocol::extentid_t &id)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = es->create(type, id);
  return ret;
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = es->get(eid, buf);
  return ret;
}

extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
  ret = es->getattr(eid, attr);
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = es->put(eid, buf, r);
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
  int64_t start = get_cycle_count();

  extent_protocol::status ret = extent_protocol::OK;
  int r;
  int64_t t1 = get_cycle_count();
  ret = es->remove(eid, r);
  int64_t end = get_cycle_count();

  printf("\t--extent_client::remove():p1 time:%fus\n", (t1 - start) / (double)CPUFREQ);
  printf("\t--extent_client::remove():remove time:%fus\n", (end - t1) / (double)CPUFREQ);
  printf("\t--extent_client::remove() time:%fus\n", (end - start) / (double)CPUFREQ);
  return ret;
}



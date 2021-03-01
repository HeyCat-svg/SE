// extent client interface.

#ifndef extent_client_h
#define extent_client_h

#include <string>
#include <vector>
#include "extent_protocol.h"
#include "extent_server.h"

#define NINODE 1025     // 多一个保险 

class extent_client {
 private:
  rpcc *cl;

  // cache operation
  enum types {ATTR, DATA, BOTH};
  struct cache_entry_ec{
    extent_protocol::extentid_t eid;
    types type;
    extent_protocol::attr attr;
    std::string data;
    bool modified;

    cache_entry_ec(){
      eid = 0;
      modified = false;
    }
  };
  cache_entry_ec cache[NINODE];
  std::vector<extent_protocol::extentid_t> flushed_list;

  cache_entry_ec *find_cache(extent_protocol::extentid_t eid);
  void put_cache(extent_protocol::extentid_t eid, cache_entry_ec &c);
  
  

public:
  extent_client(std::string dst);

  extent_protocol::status create(uint32_t type, extent_protocol::extentid_t &eid);
  extent_protocol::status get(extent_protocol::extentid_t eid, std::string &buf);
  extent_protocol::status getattr(extent_protocol::extentid_t eid, extent_protocol::attr &a);
  extent_protocol::status put(extent_protocol::extentid_t eid, std::string buf);
  extent_protocol::status remove(extent_protocol::extentid_t eid);

  std::vector<extent_protocol::extentid_t> flush_cache(extent_protocol::extentid_t eid);
  void clear_cache(extent_protocol::extentid_t eid);
};

#endif
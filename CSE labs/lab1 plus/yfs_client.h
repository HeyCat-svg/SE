#ifndef yfs_client_h
#define yfs_client_h

#include <string>
//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>


class yfs_client {
  extent_client *ec;
 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };


 private:
  static std::string filename(inum);
  static inum n2i(std::string);

 public:
  yfs_client();
  yfs_client(std::string, std::string);

  bool isfile(inum);
  bool isdir(inum);
  bool issymlink(inum inum);
  int check_type(inum inum);

  int readlink(inum ino, std::string &buf);
  int symlink(inum parent, const char *link, const char *name, inum &ino_out);

  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);

  int setattr(inum, size_t);
  int lookup(inum, const char *, bool &, inum &);
  int create(inum, const char *, mode_t, inum &);
  int readdir(inum, std::list<dirent> &);
  int write(inum, size_t, off_t, const char *, size_t &);
  int read(inum, size_t, off_t, std::string &);
  int unlink(inum,const char *);
  int mkdir(inum , const char *, mode_t , inum &);
  
  /** you may need to add symbolic link related methods here.*/

  /* cache operation */
 private:
  struct cache_entry{
    extent_protocol::extentid_t eid;
    extent_protocol::attr attr;
    std::string data;
    bool modified;
  };
  std::vector<cache_entry> cache;
  std::map<uint32_t, int> using_inodes; // map of alloced inode

  extent_protocol::status ec_create(uint32_t type, extent_protocol::extentid_t& eid);
  extent_protocol::status ec_get(extent_protocol::extentid_t eid, std::string &buf);
  extent_protocol::status ec_getattr(extent_protocol::extentid_t eid, extent_protocol::attr& a);
  extent_protocol::status ec_put(extent_protocol::extentid_t eid, std::string buf);
  extent_protocol::status ec_remove(extent_protocol::extentid_t eid);
};

#endif 

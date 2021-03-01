// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>

#define CPUFREQ 2208.006        // cycles per us

static inline uint64_t get_cycle_count(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo; 
}

yfs_client::yfs_client()
{
    ec = new extent_client();

}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client();
    if (ec_put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

int yfs_client::check_type(inum inum){
    extent_protocol::attr a;
    int ret = 0;
    if(ec_getattr(inum, a) != extent_protocol::OK){
        printf("error getting attr\n");
        return ret;
    }
    ret = a.type;
    return ret;
}

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    if (ec_getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */
bool yfs_client::issymlink(inum inum){
    extent_protocol::attr a;
    if (ec_getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }
    if (a.type == extent_protocol::T_SYMLINK) {
        printf("issymlink: %lld is a symlink\n", inum);
        return true;
    }
    return false; 
}

int yfs_client::readlink(inum ino, std::string& buf){
    int r = OK;
    if((r = read(ino, 2048, 0, buf)) != OK){
        return r;
    }
    return r;
}

int yfs_client::symlink(inum parent, const char *link, const char *name, inum &ino_out){
    int r = OK;
     // find if there is an exisiting file
    bool found = true;
    std::string buf;    // used to store link content
    if((r = lookup(parent, name, found, ino_out)) != OK){
        return r;
    }
    if(found){
        return EXIST;
    }
    // create a symbolic link file
    if((r = ec_create(extent_protocol::T_SYMLINK, ino_out)) != OK){
        return r;
    }
    // get parent dir content
    if((r = ec_get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec_put(parent, buf)) != extent_protocol::OK){
        return r;
    }

    // write path to link file
    size_t tmp_size;
    if((r = write(ino_out, strlen(link), 0, link, tmp_size)) != OK){
        return r;
    }
    return r;
}

bool
yfs_client::isdir(inum inum) {
    // Oops! is this still correct when you implement symlink?
   extent_protocol::attr a;
    if (ec_getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }
    if (a.type == extent_protocol::T_DIR) {
        printf("isdir: %lld is a dir\n", inum);
        return true;
    }
    return false; 
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

    // printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec_getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    // printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    // printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec_getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
yfs_client::setattr(inum ino, size_t size){
    int r = OK;
    /*
     * your code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */
    extent_protocol::attr attr;
    std::string buf;
    if((r = ec_getattr(ino, attr)) != OK){
        return r;
    }
    if((r = ec_get(ino, buf)) != OK){
        return r;
    }
    if(size > attr.size){
        buf.append(size - attr.size, '\0');
    }
    else if(size < attr.size){
        buf.erase(buf.begin() + size, buf.end());
    }
    else{
        return r;
    }
    ec_put(ino, buf);
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out){
    // uint64_t start = get_cycle_count();

    int r = OK;

    // find if there is an exisiting file
    bool found = true;
    std::string buf;    // used to store dir content
    if((r = lookup(parent, name, found, ino_out)) != OK){
        return r;
    }
    if(found){
        return EXIST;
    }
    // create a file
    if((r = ec_create(extent_protocol::T_FILE, ino_out)) != OK){
        return r;
    }
    // get parent dir content
    if((r = ec_get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec_put(parent, buf)) != extent_protocol::OK){
        return r;
    }

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::create() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out){
    // uint64_t start = get_cycle_count();

    int r = OK;
    // find if there is an exisiting file
    bool found = true;
    std::string buf;    // used to store dir content
    if((r = lookup(parent, name, found, ino_out)) != OK){
        return r;
    }
    if(found){
        return EXIST;
    }
    // create a dir
    if((r = ec_create(extent_protocol::T_DIR, ino_out)) != OK){
        return r;
    }
    // get parent dir content
    if((r = ec_get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec_put(parent, buf)) != extent_protocol::OK){
        return r;
    }

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::mkdir() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out){
    // uint64_t start = get_cycle_count();

    // rewrite lookup() find filename directly
    int r = OK;
    std::string buf;
    if((r = ec_get(parent, buf)) != OK){
        return r;
    }
    const char *dir_content = buf.c_str();
    const char *cur_pos = dir_content;
    std::string filename;
    while(true){
        filename.clear();
        while(*cur_pos != '\0'){
            filename.push_back(*cur_pos);
            ++cur_pos;
        }
        ++cur_pos;
        if(filename.empty()){
            found = false;
            break;
        }
        if(!strcmp(filename.c_str(), name)){
            ino_out = *((uint32_t *)cur_pos);
            found = true;
            break;
        }
        cur_pos += 4;
    }

    // ================== old version =====================
    // int r = OK;
    // std::list<dirent> file_list;
    // if((r = readdir(parent, file_list)) != OK){
    //     return r;
    // }
    // found = false;
    // std::list<dirent>::iterator it;
    // for (it = file_list.begin(); it != file_list.end(); ++it){
    //     std::string file_name = (*it).name;
    //     if(!strcmp(file_name.c_str(), name)){
    //         found = true;
    //         ino_out = (*it).inum;
    //         break;
    //     }
    // }
    // ====================================================

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::lookup() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list){
    // uint64_t start = get_cycle_count();

    int r = OK;
    /*
     * your code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    std::string buf;
    r = ec_get(dir, buf);
    if(r != OK){
        return r;
    }
    const char *dir_content = buf.c_str(); // ...name.../0inum

    // build a list
    dirent dir_tmp;
    const char *cur_pos = dir_content;
    std::string name;
    yfs_client::inum inum;
    while(true){
        name.clear();
        while(*cur_pos != '\0'){
            name.push_back(*cur_pos);
            ++cur_pos;
        }
        ++cur_pos;
        if(name.empty()){   // owaridesu
            break;
        }
        inum = *((uint32_t *)cur_pos);
        cur_pos += 4;
        dir_tmp.name = name;
        dir_tmp.inum = inum;
        list.push_back(dir_tmp);
    }
    
    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::readdir() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data){
    // uint64_t start = get_cycle_count();

    int r = OK;
    /*
     * your code goes here.
     * note: read using ec->get().
     */
    std::string buf;
    if((r = ec_get(ino, buf)) != OK){
        return r;
    }
    if(off >= (off_t)buf.size()){
        r = IOERR;
        return r;
    }
    if(off + (off_t)size > (off_t)buf.size()){
        data = buf.substr(off, buf.size() - off);
    }
    else {
        data = buf.substr(off, size);
    }

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::read() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written){
    // uint64_t start = get_cycle_count();

    int r = OK;
    /*
     * your code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    bytes_written = size;
    std::string buf;
    if((r = ec_get(ino, buf)) != OK){
        return r;
    }
    if(off > (off_t)buf.size()){
        buf.append(off - buf.size(), '\0');
        bytes_written += off - buf.size();
    }
    buf.replace(buf.begin() + off, buf.begin() + off + size, data, data + size);
    if ((r = ec_put(ino, buf)) != OK){
        return r;
    }

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::write() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

int yfs_client::unlink(inum parent,const char *name){
    
    int r = OK;
    /*
     * your code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
    inum ino;
    bool found = true;
    std::string buf;
    // uint64_t start = get_cycle_count();
    if((r = lookup(parent, name, found, ino)) != OK){
        return r;
    }
    // uint64_t t1 = get_cycle_count();
    if(!found){
        r = NOENT;
        return r;
    }
    ec_remove(ino);
    // uint64_t t2 = get_cycle_count();
    // unlink dir entry from parent
    if((r = ec_get(parent, buf)) != OK){
        return r;
    }
    // uint64_t t3 = get_cycle_count();
    const char *dir_content = buf.c_str();
    const char *pre_pos = dir_content;
    const char *cur_pos = dir_content;
    std::string file_name;
    while(true){
        file_name.clear();
        while(*cur_pos != '\0'){
            file_name.push_back(*cur_pos);
            ++cur_pos;
        }
        ++cur_pos;
        if(file_name.empty()){   // always false
            printf("ERR! No such file name in parent dir\n");
            r = IOERR;
            return r;
        }
        if(!strcmp(file_name.c_str(), name)){
            buf.erase(pre_pos - buf.c_str(), cur_pos - pre_pos + 4);
            break;
        }
        cur_pos += 4;
        pre_pos = cur_pos;
    }
    // uint64_t t4 = get_cycle_count();
    // put 
    if((r = ec_put(parent, buf)) != OK){
        return r;
    }

    // uint64_t end = get_cycle_count();
    // printf("\t--yfs::unlink():lookup time:%fus\n", (t1 - start) / (double)CPUFREQ);
    // printf("\t--yfs::unlink():remove time:%fus\n", (t2 - t1) / (double)CPUFREQ);
    // printf("\t--yfs::unlink():get time:%fus\n", (t3 - t2) / (double)CPUFREQ);
    // printf("\t--yfs::unlink():iter time:%fus\n", (t4 - t3) / (double)CPUFREQ);
    // printf("\t--yfs::unlink():put time:%fus\n", (end - t4) / (double)CPUFREQ);
    // printf("\t--yfs::unlink() time:%fus\n", (end - start) / (double)CPUFREQ);
    return r;
}

extent_protocol::status yfs_client::ec_create(uint32_t type, extent_protocol::extentid_t& eid){
    // alloc inode
    uint32_t inum;
    for (inum = 2; inum <= 1024; ++inum){   // rootdir has been alloced to 1
        if(!using_inodes[inum]){
            using_inodes[inum] = 1;
            break;
        }
    }
    eid = inum;
    // build cache entry
    cache_entry new_entry;
    new_entry.attr.type = type;
    int tm = std::time(0);
    new_entry.attr.mtime = tm;
    new_entry.attr.ctime = tm;
    new_entry.attr.atime = tm;
    new_entry.attr.size = 0;
    new_entry.eid = inum;
    new_entry.data = "";
    cache.push_back(new_entry);

    return extent_protocol::OK;
}

extent_protocol::status yfs_client::ec_get(extent_protocol::extentid_t eid, std::string &buf){
    // find cache entry
    int size = cache.size();
    cache_entry *entry = NULL;
    for (int i = 0; i < size; ++i){
        if(cache[i].eid == eid){
            entry = &(cache[i]);
            break;
        }
    }
    // get data from cache
    if (entry){
        buf = entry->data;
    }
    else{
        buf = "";
    }
    return extent_protocol::OK;
}

extent_protocol::status yfs_client::ec_getattr(extent_protocol::extentid_t eid, extent_protocol::attr& a){
    // find cache entry
    int size = cache.size();
    cache_entry *entry = NULL;
    for (int i = 0; i < size; ++i){
        if(cache[i].eid == eid){
            entry = &(cache[i]);
            break;
        }
    }

    // get attr from cache
    if(entry){
        a = entry->attr;
    }
    return extent_protocol::OK;
}

extent_protocol::status yfs_client::ec_put(extent_protocol::extentid_t eid, std::string buf){
    // find cache entry
    int size = cache.size();
    cache_entry *entry = NULL;
    for (int i = 0; i < size; ++i){
        if(cache[i].eid == eid){
            entry = &(cache[i]);
            break;
        }
    }

    if(entry){
        entry->data = buf;
        entry->attr.size = buf.size();
        entry->modified = true;
        int tm = std::time(0);
        entry->attr.mtime = tm;
        entry->attr.ctime = tm;
    }
    else{   // init rootdir...
        cache_entry new_entry;
        new_entry.eid = eid;
        new_entry.data = buf;
        new_entry.modified = true;
        int tm = std::time(0);
        new_entry.attr.atime = tm;
        new_entry.attr.ctime = tm;
        new_entry.attr.mtime = tm;
        new_entry.attr.size = 0;
        new_entry.attr.type = extent_protocol::T_DIR;
        cache.push_back(new_entry);
    }
    return extent_protocol::OK;
}

extent_protocol::status yfs_client::ec_remove(extent_protocol::extentid_t eid){
    int size = cache.size();
    for (int i = 0; i < size; ++i){
        if(cache[i].eid == eid){
            cache.erase(cache.begin() + i);
            using_inodes[eid] = 0;
            break;
        }
    }
    return extent_protocol::OK;
}

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

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client(extent_dst);
    // Lab2: Use lock_client_cache when you test lock_cache
    // lc = new lock_client(lock_dst);
    lc = new lock_client_cache(lock_dst, ec);
    if (ec->put(1, "") != extent_protocol::OK)
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

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    lc->acquire(inum);  // lock

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
#ifdef DEBUG
        printf("isfile: %lld is a file\n", inum);
#endif
        lc->release(inum);
        return true;
    }
    lc->release(inum);
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */
bool yfs_client::issymlink(inum inum){
    extent_protocol::attr a;
    lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }
    if (a.type == extent_protocol::T_SYMLINK) {
#ifdef DEBUG
        printf("isfile: %lld is a symlink\n", inum);
#endif
        lc->release(inum);
        return true;
    }
    lc->release(inum);
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
    lc->acquire(parent);

    bool found = true;
    std::string buf;    // used to store link content
    if((r = lookup_nolock(parent, name, found, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    if(found){
        lc->release(parent);
        return EXIST;
    }
    // create a symbolic link file
    if((r = ec->create(extent_protocol::T_SYMLINK, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    // get parent dir content
    if((r = ec->get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        lc->release(parent);
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec->put(parent, buf)) != extent_protocol::OK){
        lc->release(parent);
        return r;
    }

    // write path to link file
    size_t tmp_size;
    if((r = write(ino_out, strlen(link), 0, link, tmp_size)) != OK){
        lc->release(parent);
        return r;
    }
    lc->release(parent);
    return r;
}

bool
yfs_client::isdir(inum inum) {
    // Oops! is this still correct when you implement symlink?
   extent_protocol::attr a;
   lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }
    if (a.type == extent_protocol::T_DIR) {
#ifdef DEBUG
        printf("isdir: %lld is a dir\n", inum);
#endif
        lc->release(inum);
        return true;
    }
    lc->release(inum);
    return false; 
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;
    lc->acquire(inum);
#ifdef DEBUG
    printf("getfile %016llx\n", inum);
#endif
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
#ifdef DEBUG
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);
#endif

release:
    lc->release(inum);
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;
    lc->acquire(inum);

#ifdef DEBUG
    printf("getdir %016llx\n", inum);
#endif
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    lc->release(inum);
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
    lc->acquire(ino);

    extent_protocol::attr attr;
    std::string buf;
    if((r = ec->getattr(ino, attr)) != OK){
        lc->release(ino);
        return r;
    }
    if((r = ec->get(ino, buf)) != OK){
        lc->release(ino);
        return r;
    }
    if(size > attr.size){
        buf.append(size - attr.size, '\0');
    }
    else if(size < attr.size){
        buf.erase(buf.begin() + size, buf.end());
    }
    else{
        lc->release(ino);
        return r;
    }
    ec->put(ino, buf);

    lc->release(ino);
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out){
    int r = OK;
    /*
     * your code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */

    // find if there is an exisiting file
    lc->acquire(parent);

    bool found = true;
    std::string buf;    // used to store dir content
    if((r = lookup_nolock(parent, name, found, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    if(found){
        lc->release(parent);
        return EXIST;
    }
    // create a file
    if((r = ec->create(extent_protocol::T_FILE, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    // get parent dir content
    if((r = ec->get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        lc->release(parent);
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec->put(parent, buf)) != extent_protocol::OK){
        lc->release(parent);
        return r;
    }
    lc->release(parent);
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out){
    int r = OK;
    /*
     * your code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
     // find if there is an exisiting file
    lc->acquire(parent);

    bool found = true;
    std::string buf;    // used to store dir content
    if((r = lookup_nolock(parent, name, found, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    if(found){
        lc->release(parent);
        return EXIST;
    }
    // create a dir
    if((r = ec->create(extent_protocol::T_DIR, ino_out)) != OK){
        lc->release(parent);
        return r;
    }
    // get parent dir content
    if((r = ec->get(parent, buf)) != OK){
        printf("ERR! Can't get parent dir\n");
        lc->release(parent);
        return r;
    }
    // add link to parent dir
    uint32_t tmp = ino_out;
    buf.append(name);
    buf.append(1, '\0');
    buf.append((char *)&tmp, 4);
    // put dir content back to disk
    if((r = ec->put(parent, buf)) != extent_protocol::OK){
        lc->release(parent);
        return r;
    }
    lc->release(parent);
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out){
    int r = OK;
    lc->acquire(parent);
    r = lookup_nolock(parent, name, found, ino_out);
    lc->release(parent);
    return r;
}

int yfs_client::lookup_nolock(inum parent, const char *name, bool &found, inum &ino_out){
    int r = OK;
    /*
     * your code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    std::list<dirent> file_list;
    if((r = readdir_nolock(parent, file_list)) != OK){
        return r;
    }
    found = false;
    std::list<dirent>::iterator it;
    for (it = file_list.begin(); it != file_list.end(); ++it){
        std::string file_name = (*it).name;
        if(!strcmp(file_name.c_str(), name)){
            found = true;
            ino_out = (*it).inum;
            return r;
        }
    }

    return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list){
    int r = OK;
    lc->acquire(dir);
    r = readdir_nolock(dir, list);
    lc->release(dir);
    return r;
}

int yfs_client::readdir_nolock(inum dir, std::list<dirent> &list){
     int r = OK;
    /*
     * your code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    std::string buf;
    r = ec->get(dir, buf);
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
    
    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data){
    int r = OK;
    /*
     * your code goes here.
     * note: read using ec->get().
     */
    lc->acquire(ino);

    std::string buf;
    if((r = ec->get(ino, buf)) != OK){
        lc->release(ino);
        return r;
    }
    if(off >= (off_t)buf.size()){
        r = IOERR;
        lc->release(ino);
        return r;
    }
    if(off + (off_t)size > (off_t)buf.size()){
        data = buf.substr(off, buf.size() - off);
    }
    else {
        data = buf.substr(off, size);
    }

    lc->release(ino);
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written){
    int r = OK;
    /*
     * your code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    lc->acquire(ino);

    bytes_written = size;
    std::string buf;
    if((r = ec->get(ino, buf)) != OK){
        lc->release(ino);
        return r;
    }
    if(off > (off_t)buf.size()){
        buf.append(off - buf.size(), '\0');
        bytes_written += off - buf.size();
    }
    buf.replace(buf.begin() + off, buf.begin() + off + size, data, data + size);
    if ((r = ec->put(ino, buf)) != OK){
        lc->release(ino);
        return r;
    }

    lc->release(ino);
    return r;
}

int yfs_client::unlink(inum parent,const char *name){
    int r = OK;
    /*
     * your code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
    lc->acquire(parent);

    inum ino;
    bool found = true;
    std::string buf;
    if((r = lookup_nolock(parent, name, found, ino)) != OK){
        lc->release(parent);
        return r;
    }
    if(!found){
        r = NOENT;
        lc->release(parent);
        return r;
    }

    lc->acquire(ino);
    ec->remove(ino);
    lc->release(ino);

    // unlink dir entry from parent
    if((r = ec->get(parent, buf)) != OK){
        lc->release(parent);
        return r;
    }
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
    // put 
    if((r = ec->put(parent, buf)) != OK){
        lc->release(parent);
        return r;
    }
    lc->release(parent);
    return r;
}


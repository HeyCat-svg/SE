#include "inode_manager.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

#define CPUFREQ 2208.006        // cycles per us

static inline uint64_t get_cycle_count(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo; 
}

// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf) {
  if (id < 0 || id >= BLOCK_NUM || buf == NULL){
    return;
  }
  memcpy(buf, blocks[id], BLOCK_SIZE);
}

void
disk::write_block(blockid_t id, const char *buf) {
  if (id < 0 || id >= BLOCK_NUM || buf == NULL){
    return;
  }
  memcpy(blocks[id], buf, BLOCK_SIZE);
}

// block layer -----------------------------------------

// Allocate a free disk block.
blockid_t
block_manager::alloc_block() {
  /*
   * your code goes here.
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.
   */
  uint32_t start = sb.nblocks / BPB + INODE_NUM + 2;
  for (uint32_t i = start; i < BLOCK_NUM; ++i){
    if(using_blocks[i] == 0){
      using_blocks[i] = 1;
      return i;
    }
  }
  return 0;
}

void
block_manager::free_block(uint32_t id) {
  /* 
   * your code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  if (id < 0 || id >= BLOCK_NUM){
    return;
  }
  using_blocks[id] = 0;
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager() {
  d = new disk();

  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;
}

void
block_manager::read_block(uint32_t id, char *buf)
{
  d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf)
{
  d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* 
   * your code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().
   */
  static uint32_t id = 1;
  while(using_inodes[id]){
    id = id % INODE_NUM + 1;
  }
  using_inodes[id] = 1;

  inode *ino = (inode *)malloc(sizeof(inode));
  bzero(ino, sizeof(inode));
  ino->type = type;
  int tm = std::time(0);
  ino->mtime = tm;
  ino->ctime = tm;
  ino->atime = tm;
  put_inode(id, ino);
  free(ino);
  return id;


  // struct inode *tmp;
  // struct inode *ino;
  // for (int i = 1; i <= INODE_NUM; ++i) {
  //   if ((tmp = get_inode(i)) == NULL) { // find an empty inode
  //     ino = (struct inode *)malloc(sizeof(struct inode));
  //     bzero(ino, sizeof(struct inode));
  //     ino->size = 0;
  //     ino->type = type;
  //     int tm = std::time(0);
  //     ino->mtime = tm;
  //     ino->ctime = tm;
  //     ino->atime = tm;
  //     put_inode(i, ino);
  //     free(ino);
  //     return i;
  //   }
  //   else {
  //     free(tmp);
  //   }
  // }
  // return 0;
}

void
inode_manager::free_inode(uint32_t inum) {
  /* 
   * your code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   */
  if(using_inodes[inum]){
    inode *ino = (inode *)malloc(sizeof(inode));
    bzero((char *)ino, sizeof(inode));
    put_inode(inum, ino);
    free(ino);
    using_inodes[inum] = 0;
  }

  // struct inode *ino;
  // ino = get_inode(inum);
  // if(ino != NULL) {
  //   bzero((char *)ino, sizeof(struct inode));
  //   put_inode(inum, ino);
  //   free(ino);
  // }
  // return;
}


/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_manager::get_inode(uint32_t inum) {
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];

  // printf("\tim: get_inode %d\n", inum);

  if (inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return NULL;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  // printf("%s:%d\n", __FILE__, __LINE__);

  ino_disk = (struct inode*)buf + inum%IPB;
  if (ino_disk->type == 0) {
    printf("\tim: inode not exist\n");
    return NULL;
  }

  ino = (struct inode*)malloc(sizeof(struct inode));
  *ino = *ino_disk;

  return ino;
}

void
inode_manager::put_inode(uint32_t inum, struct inode *ino) {
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;

  // printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode*)buf + inum%IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf_out, int *size) {
  /*
   * your code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_Out
   */
  struct inode *ino = get_inode(inum);
  // change atime
  int tm = std::time(0);
  ino->atime = tm;
  *size = ino->size;
  *buf_out = (char *)malloc(((*size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE);
  char *cur_pos = *buf_out;

  for(uint32_t i = 0; i < NDIRECT && ino->blocks[i]; ++i){
    bm->read_block(ino->blocks[i], cur_pos);
    cur_pos += BLOCK_SIZE;
  }
  if(ino->blocks[NDIRECT]) {
    uint32_t indirect_block[NINDIRECT];
    bm->read_block(ino->blocks[NDIRECT], (char *)indirect_block);
    for (uint32_t i = 0; i < NINDIRECT && indirect_block[i]; ++i) {
      bm->read_block(indirect_block[i], cur_pos);
      cur_pos += BLOCK_SIZE;
    }
  }
  // update atime
  put_inode(inum, ino);
  free(ino);
}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size) {
  /*
   * your code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode
   */
  
  struct inode *ino = get_inode(inum);
  // update mtime and atime
  int tm = std::time(0);
  ino->atime = tm;
  ino->mtime = tm;
  if(ino->type == extent_protocol::T_DIR){
    ino->ctime = tm;
  }
  ino->size = size;
  const char *cur_pos = buf;
  int remaining_size = size;
  int i = 0;

  for (; i < NDIRECT && remaining_size > 0; ++i){
    if(!ino->blocks[i]){
      ino->blocks[i] = bm->alloc_block();
    }
    bm->write_block(ino->blocks[i], cur_pos);
    cur_pos += BLOCK_SIZE;
    remaining_size -= BLOCK_SIZE;
  }

  if(remaining_size > 0){   // something stil need to be written
    uint32_t indirect_block[NINDIRECT];
    bzero((char *)indirect_block, NINDIRECT * sizeof(uint32_t));
    if (ino->blocks[NDIRECT]){
      bm->read_block(ino->blocks[NDIRECT], (char *)indirect_block);
    }
    else{
      ino->blocks[NDIRECT] = bm->alloc_block();
    }

    uint32_t ii = 0;
    for (; ii < NINDIRECT && remaining_size > 0; ++ii){
      if(!indirect_block[ii]){
        indirect_block[ii] = bm->alloc_block();
      }
      bm->write_block(indirect_block[ii], cur_pos);
      cur_pos += BLOCK_SIZE;
      remaining_size -= BLOCK_SIZE;
    }

    // free remaining origin block
    for(;ii < NINDIRECT && indirect_block[ii]; ++ii){
      bm->free_block(indirect_block[ii]);
      indirect_block[ii] = 0;
    }
    bm->write_block(ino->blocks[NDIRECT], (char *)indirect_block);
  }
  else{
    for (; i < NDIRECT; ++i){
      if (ino->blocks[i]){
        bm->free_block(i);
      }
      ino->blocks[i] = 0;
    }
    if(ino->blocks[NDIRECT]){
      uint32_t indirect_block[NINDIRECT];
      bm->read_block(ino->blocks[NDIRECT], (char *)indirect_block);
      for (uint32_t ii = 0; ii < NINDIRECT && indirect_block[ii]; ++ii){
        bm->free_block(indirect_block[ii]);
        indirect_block[ii] = 0;
      }
      bm->free_block(ino->blocks[NDIRECT]);
      ino->blocks[NDIRECT] = 0;
    }
  }
  put_inode(inum, ino);
  free(ino);
}

void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a) {
  /*
   * your code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
  struct inode *ino = get_inode(inum);
  if(ino == NULL){
    return;
  }
  a.type = ino->type;
  a.atime = ino->atime;
  a.ctime = ino->ctime;
  a.mtime = ino->mtime;
  a.size = ino->size;

  free(ino);
  return;
}

void
inode_manager::remove_file(uint32_t inum) {
  // uint64_t start = get_cycle_count();

  struct inode *ino = get_inode(inum);
  if(ino == NULL) {
    return;
  }
  uint32_t block_num = (ino->size + BLOCK_SIZE -1) / BLOCK_SIZE;

  // uint64_t t1 = get_cycle_count();

  for (uint32_t i = 0; i < NDIRECT && i < block_num; ++i){
    bm->free_block(ino->blocks[i]);
  }

  // uint64_t t2 = get_cycle_count();

  if(block_num > NDIRECT){
    uint32_t indirect_block[NINDIRECT];
    bm->read_block(ino->blocks[NDIRECT], (char *)indirect_block);
    for (uint32_t i = 0; i < block_num - NDIRECT; ++i){
      bm->free_block(indirect_block[i]);
    }
    bm->free_block(ino->blocks[NDIRECT]);
    ino->blocks[NDIRECT] = 0;
  }

  free_inode(inum);
  free(ino);

  // uint64_t end = get_cycle_count();
  // printf("\t--im::remove():get time:%fus\n", (t1 - start) / (double)CPUFREQ);
  // printf("\t--im::remove():free_direct time:%fus\n", (t2 - t1) / (double)CPUFREQ);
  // printf("\t--im::remove():free_indirect time:%fus\n", (end - t2) / (double)CPUFREQ);
  // printf("\t--im::remove() time:%fus\n", (end - start) / (double)CPUFREQ);
  return;
}
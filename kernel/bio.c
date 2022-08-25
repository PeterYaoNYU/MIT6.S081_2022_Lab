// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKET_NUM 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct spinlock bucket_lock[BUCKET_NUM];
  struct buf bucket_head[BUCKET_NUM];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  for (int i = 0; i <BUCKET_NUM; i++){
    initlock(&bcache.bucket_lock[i], "bcache.bucket");
  }

  // Create linked list of buffers
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->timestamp = ticks;
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // acquire(&bcache.lock);

  int index = blockno % BUCKET_NUM;
  acquire(&bcache.bucket_lock[index]);

  for (b = bcache.bucket_head[index].next; b; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->timestamp = ticks;
      // release(&bcache.lock);
      release(&bcache.bucket_lock[index]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  int time = ticks;
  struct buf * min = 0;
  for(b = bcache.buf; b< bcache.buf + NBUF; b++){
    if(b->refcnt == 0) {
      // b->timestamp = ticks;
      if (b ->timestamp <= time)
        min = b;
    }
  }
  if (min){
    min->dev = dev;
    min->blockno = blockno;
    min->valid = 0;
    min->refcnt = 1;

    min->next=bcache.bucket_head[index].next;
    min->prev= &bcache.bucket_head[index];
    bcache.bucket_head[index].next = min;
    min->next->prev = min;
    
    release(&bcache.bucket_lock[index]);
    acquiresleep(&min->lock);
    return min;

  }
  if (!min)
    panic("bget: no buffers");
  return min;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  uint index = b->blockno % BUCKET_NUM;

  acquire(&bcache.bucket_lock[index]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
  }
  
  release(&bcache.bucket_lock[index]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}



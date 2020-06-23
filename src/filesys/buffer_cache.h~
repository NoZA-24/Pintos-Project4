#ifndef BUFFER_CACHE_H
#define BUFFER_CACHE_H

#include "filesys/off_t.h"
#include "devices/block.h"
#include "threads/synch.h"
#include "filesys/inode.h"

struct buffer_head
{
  block_sector_t sector;			/* Address of disk sector. */
  bool dirty;					/* Dirty bit. */
  bool is_used;					/* Indicate this entry is used. */
  bool clock_bit;				/* Clock bit for clock algorithm. */
  void* data;					/* Point buffer cache entry. */
  struct lock buffer_lock;			/* Buffer lock. */
};

void bc_init(void);
void bc_term(void);
bool bc_read (block_sector_t sector_idx, void *buffer, off_t bytes_read, int chunk_size, int sector_ofs);
bool bc_write (block_sector_t sector_idx, void *buffer, off_t bytes_written, int chunk_size, int sector_ofs);


#endif

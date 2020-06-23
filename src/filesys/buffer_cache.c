#include <threads/malloc.h>
#include <stdio.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "filesys/buffer_cache.h"


#define BUFFER_CACHE_ENTRY_NB 64

void *p_buffer_cache;

/* Buffer Cache */
struct buffer_head head_buffer[BUFFER_CACHE_ENTRY_NB];

/* For LRU Algorithm */
int clock;


struct buffer_head* bc_select_victim(void);
struct buffer_head* bc_lookup (block_sector_t sector);
void bc_flush_entry (struct buffer_head *p_flush_entry);
void bc_flush_all_entries (void);

/* Initialize Buffer Cache.
   Allocate Buffer Cache and point it. */
void
bc_init (void)
{
  p_buffer_cache = malloc (BLOCK_SECTOR_SIZE * BUFFER_CACHE_ENTRY_NB);
  if(p_buffer_cache == NULL)
    return;
  int i;
  /* Initial each cache block. */
  for (i = 0; i < BUFFER_CACHE_ENTRY_NB; i++)
  {
    head_buffer[i].dirty = false;
    head_buffer[i].is_used = false;
    head_buffer[i].clock_bit = false;
    head_buffer[i].data = p_buffer_cache + BLOCK_SECTOR_SIZE * i;
    lock_init(&head_buffer[i].buffer_lock);
  }
}

/* Flush all data on cache and move to disk. */
void
bc_term (void)
{
  bc_flush_all_entries ();
  free (p_buffer_cache);
}

/* Select victim cache block by clock algorithm.
   When entry is dirty, update disk too. */
struct buffer_head*
bc_select_victim (void)
{
  int i;

  /* Find Unused entry first */
  for(i = 0; i < BUFFER_CACHE_ENTRY_NB; i++)
    if (head_buffer[i].is_used == false)
      return &head_buffer[i];

  /* Then, find old buffer by clock algorithm */
  while(true)
  {
    /* Make it circular */
    if (clock == BUFFER_CACHE_ENTRY_NB - 1)
      clock = 0;
    else
      clock++;
    if (head_buffer[clock].clock_bit == false)
    {
      /* If dirty, update disk. */
      if (head_buffer[clock].dirty == true)
      {
        bc_flush_entry (&head_buffer[clock]);
      }
      head_buffer[clock].sector = -1;
      head_buffer[clock].is_used = false;
      head_buffer[clock].clock_bit = false;
      return &head_buffer[clock];
    }
    else
      head_buffer[clock].clock_bit = false;
  }
}

/* Find Corresponding Cache
   if there is no corresponding entry, return NULL */
struct buffer_head*
bc_lookup (block_sector_t sector)
{
  int i;
  for(i = 0; i < BUFFER_CACHE_ENTRY_NB; i++)
  {
    if (head_buffer[i].is_used == true && 
      head_buffer[i].sector == sector)
    return &head_buffer[i];
  }
  return NULL;
}

/* Flush given entry. Write Data to disk. */
void
bc_flush_entry (struct buffer_head *p_flush_entry)
{
  block_write (fs_device, p_flush_entry->sector, p_flush_entry->data);
  p_flush_entry->dirty = false;
}

/* Flush all entry. */
void
bc_flush_all_entries (void)
{
  int i;
  for(i = 0; i < BUFFER_CACHE_ENTRY_NB; i++)
  {
    if (head_buffer[i].is_used == true &&
      head_buffer[i].dirty == true)
    bc_flush_entry (&head_buffer[i]);
  }
}

/* Read Data From Buffer Cache
   If there is no corresponding data, get it from disk. */
bool
bc_read (block_sector_t sector_idx, void *buffer,
		off_t bytes_read, int chunk_size, int sector_ofs)
{
  struct buffer_head *sector_buffer = bc_lookup (sector_idx);
  /* If there is no corresponding buffer,
     eject old buffer */
  if (sector_buffer == NULL)
  {
    sector_buffer = bc_select_victim ();	/* Select old cache block. */
    sector_buffer->sector = sector_idx;		/* Update on it. */
    sector_buffer->is_used = true;
    block_read (fs_device,sector_idx,sector_buffer->data); /* Read data from Block. */
  }
  sector_buffer->clock_bit = true;
  memcpy (buffer + bytes_read, sector_buffer->data + sector_ofs, chunk_size);
  
  return true;
}

/* Write Data At Empty Buffer Cache
   If there is no empty buffer, remove old block and replace it */
bool
bc_write (block_sector_t sector_idx, void *buffer,
		off_t bytes_written, int chunk_size, int sector_ofs)
{
  struct buffer_head *sector_buffer = bc_lookup (sector_idx);
  /* If there is no empty buffer */
  if(sector_buffer == NULL)
  {
    sector_buffer = bc_select_victim ();	/* Select old cache block. */
    sector_buffer->sector = sector_idx;		/* Update on it. */	
    sector_buffer->is_used = true;
    block_write (fs_device, sector_idx, sector_buffer->data);
  }
  /* Write on Cache. */
  memcpy (sector_buffer->data + sector_ofs, buffer + bytes_written, chunk_size);
  lock_acquire (&sector_buffer->buffer_lock);
  sector_buffer->dirty = true;
  sector_buffer->clock_bit = true;
  lock_release (&sector_buffer->buffer_lock);
  return true;
}

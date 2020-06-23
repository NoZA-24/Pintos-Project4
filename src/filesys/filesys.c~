#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/buffer_cache.h"
#include "threads/thread.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  /* Initialize Block Cache too. */
  bc_init ();

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();
  
  free_map_open ();
  thread_current ()->dir = dir_open_root ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  /* Terminate Block Cache too. */
  bc_term ();
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size) 
{

  char file_name[strlen (name) + 1];
  block_sector_t inode_sector = 0;
  struct dir *dir = parse_path(name, file_name);

  /* If directory is invalid, */
  if(dir != NULL && inode_removed (dir_get_inode (dir)))
  {
    dir_close (dir);
    return false;
  }
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
		  && inode_sector < 4095	/* Prevent Overflow. # of max sectors = 4096. */
                  && inode_create (inode_sector, initial_size,0)
                  && dir_add (dir, file_name, inode_sector));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);
  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  struct inode *inode = NULL;
  char file_name[strlen (name) + 1];
  struct dir *dir;
  dir = parse_path (name, file_name);

  /* If directory is invalid, */
  if(dir != NULL && inode_removed (dir_get_inode (dir)))
    {
      dir_close (dir);
      return NULL;
    }
  if (dir != NULL)
    dir_lookup (dir, file_name, &inode);
  dir_close (dir);
  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  char file_name[strlen (name) + 1];
  struct dir *dir;

  dir = parse_path (name, file_name);
  
  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir); 

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  struct dir *root_dir = dir_open_root ();
  struct inode *root_inode = dir_get_inode (root_dir);

  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  
  /* Make "..", "." in root directory.
     It point root directory itself*/
  dir_add (root_dir, ".", inode_get_inumber (root_inode));
  dir_add (root_dir, "..", inode_get_inumber (root_inode));
  free_map_close ();
  printf ("done.\n");
}

/* Make absolute path to relative path and return it's directory. */
struct dir*
parse_path (char *path_name, char *file_name)
{
  struct dir *dir;
  /* absolute root start at root directory. */
  if (path_name[0] == '/')
    dir = dir_open_root ();
  /* relative root start at current directory. */
  else
    dir = dir_reopen (thread_current ()->dir);

  /* Invalid case handling. */
  if (path_name == NULL || file_name == NULL)
    return NULL;
  if (strlen (path_name) == 0)
    return NULL;

  struct inode *inode;
  char *token, *nextToken, *savePtr;
  char path_name_copy[strlen (path_name) + 1];

  /* Parse path string. */
  strlcpy (path_name_copy, path_name, strlen (path_name) + 1);
  token = strtok_r (path_name_copy, "/", &savePtr);
  nextToken = strtok_r (NULL, "/", &savePtr);

  /* Parse until there is no "/" left. */
  while (token != NULL && nextToken != NULL)
  {
    inode = NULL;
    if (!dir_lookup (dir, token, &inode))
    {
      dir_close (dir);
      return NULL;
    }
    if (inode_is_dir (inode) == false)
    {
      dir_close (dir);
      return NULL;
    }
    dir_close (dir);
    dir = dir_open (inode);
    token = nextToken;
    nextToken = strtok_r (NULL, "/", &savePtr);
  }

  /* Fill filename. */
  if (token != NULL)
    strlcpy (file_name, token, strlen (token) + 1);

  /* If file end with "/", return last directory itself. */
  else
    strlcpy (file_name, ".", 2);
  return dir;
}

/* Create directory. */
bool
filesys_create_dir (const char *name)
{
  /* Name should be valid. */
  if (name == NULL)
    return false;

  char file_name[strlen (name) + 1];
  struct inode *inode;
  struct dir *dir = parse_path (name, file_name);

  /* If there is already same named file exist, fail. */
  if (dir_lookup (dir, file_name, &inode))
    return false;

  block_sector_t sector_idx;

  /* Except invalid case. */
  if (!free_map_allocate (1, &sector_idx) ||
	sector_idx >= 4095 ||	/* Prevent Overflow. # of max sectors = 4096. */
	!dir_create (sector_idx, 16) ||
	!dir_add (dir, file_name, sector_idx))
    return false;

  /* Add "..", "." in newly created directory. */
  struct dir *dir2 = dir_open (inode_open (sector_idx));
  if (!dir_add (dir2, ".", sector_idx))
    return false;
  if (!dir_add (dir2, "..", inode_get_inumber (dir_get_inode (dir))))
    return false;

  dir_close (dir2);
  return true;
}

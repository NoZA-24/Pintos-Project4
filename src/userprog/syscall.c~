#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void check_address (void *addr);
void get_argument (void *esp, int *arg, int count);
void halt (void);
void exit (int status);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
tid_t exec (const char *cmd_line);
int wait (tid_t tid);
int open (const char *file);
int filesize(int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
bool sys_isdir(int fd);
bool sys_chdir(const char *dir);
bool sys_mkdir(const char *dir);
bool sys_readdir(int fd, char *name);
uint32_t sys_inumber(int fd);



void
syscall_init (void) 
{ 
  /* Init FILESYS_LOCK, this lock's purpose is prevent
     other processes read,write,open some file which 
     read,write,open by other some process. */
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* ARG array save the system calls argument.
     The arguments are stored in user stack.
     The get_argument() use to get argument
     from user stack. */
  int arg[3];
  int syscall_number;
  
  /* For every pointer arguments, I will check the
     address is in user area using check_address(). */
  check_address ((void *)(f->esp));

  /* By SYSCALL_NUMBER, call get_argument() to get
     proper arguments, and call corresponding 
     system call functions. The return value of
     System call need to save in eax register. */
  syscall_number = *(int *)(f->esp);

  switch(syscall_number)
    {
      case SYS_HALT:
        halt();
        break;
      case SYS_EXIT:
        get_argument(f->esp, arg, 1);
        exit(arg[0]);
        break;
      case SYS_EXEC:
        get_argument(f->esp, arg, 1);
        check_address((void *)arg[0]);
        f->eax = exec((const char *)arg[0]);
        break;
      case SYS_WAIT:
        get_argument(f->esp, arg, 1);
        f->eax = wait((tid_t)arg[0]);
        break;
      case SYS_CREATE:
        get_argument(f->esp, arg, 2);
        check_address((void *)arg[0]);
        f->eax = create((const char *)arg[0], (int)arg[1]);
        break;
      case SYS_REMOVE:
        get_argument(f->esp, arg, 1);
        check_address((void *)arg[0]);
        f->eax = remove((const char *)arg[0]);
        break;
      case SYS_OPEN:
        get_argument(f->esp, arg, 1);
        check_address((void *)arg[0]);
        f->eax = open((const char *)arg[0]);
        break;
      case SYS_FILESIZE:
        get_argument(f->esp, arg, 1);
        f->eax = filesize((int)arg[0]);
        break;
      case SYS_READ:
        get_argument(f->esp, arg, 3);
        check_address((void *)arg[1]);
        f->eax = read((int)arg[0], (void *)arg[1], (unsigned)arg[2]);
        break;
      case SYS_WRITE:
        get_argument(f->esp, arg, 3);
        check_address((void *)arg[1]);
        f->eax = write((int)arg[0], (void *)arg[1], (unsigned)arg[2]);
        break;
      case SYS_SEEK:
        get_argument(f->esp, arg, 2);
        seek((int)arg[0], (unsigned)arg[1]);
        break;
      case SYS_TELL:
        get_argument(f->esp, arg, 1);
        f->eax = tell((int)arg[0]);
        break;
      case SYS_CLOSE:
        get_argument(f->esp, arg, 1);
        close((int)arg[0]);
        break;
      case SYS_ISDIR:
        get_argument(f->esp, arg, 1);
        f->eax = sys_isdir((int)arg[0]);
        break;
      case SYS_CHDIR:
        get_argument(f->esp, arg, 1);
        check_address((void *)arg[0]);
        f->eax = sys_chdir((const char*)arg[0]);
        break;
      case SYS_MKDIR:
        get_argument(f->esp, arg, 1);
        check_address((void *)arg[0]);
        f->eax = sys_mkdir((const char*)arg[0]);
        break;
      case SYS_READDIR:
        get_argument(f->esp, arg, 2);
        check_address((void *)arg[1]);
        f->eax = sys_readdir((int)arg[0],(char *)arg[1]);
        break;
      case SYS_INUMBER:
        get_argument(f->esp, arg, 1);
        f->eax = sys_inumber((int)arg[0]);
        break;
      default:
        thread_exit();
        break;
    }  
}

void
check_address (void *addr)
{
  /* Check_address() function checks the given address is
     user area. If not, call exit(-1). */
  if ((unsigned)addr < 0x8048000 || (unsigned)addr >= 0xc0000000)
    exit(-1);
}

void
get_argument (void *esp, int *arg, int count)
{
  /* Get_argument() function extract argument from user stack.
     Read each 4bytes of esp register. */
  int i;
  for(i = 0; i < count; i++)
    {
      check_address(esp + 4 + 4 * i);
      arg[i] = *(int *)(esp + 4 + 4 * i);
    }
}

void
halt (void)
{
  /* Halt() function terminate pintos program.
     It implemented by shutdown_power_off(). */
  shutdown_power_off();
}

void
exit (int status)
{
  /* Exit() funtion terminate current process.
     It saves exit status to EXIT_STATUS and
     call thread_exit(). */
  struct thread *cur = thread_current ();
  cur->exit_status = status;
  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

bool
create (const char *file, unsigned initial_size)
{
  /* Create() function creates given file.
     It implemented by filesys_create(). */
  if(file==NULL)
    return false;
  return filesys_create(file, initial_size);
}

bool
remove (const char *file)
{
  /* Remove() function remove given file.
     It implemented by filesys_remove(). */
  if(file==NULL)
    return false;
  return filesys_remove(file);
}

tid_t
exec (const char *cmd_line)
{
  int tid;
  struct thread *cp;
  /* Make new child process using process_execute(). */
  tid = process_execute (cmd_line);
  cp = get_child_process (tid);
  /* Wait the load() is finish in start_process() using
     sema_down(). The share resource is LOAD semaphore. */
  sema_down (&cp->load);
  /* If fail to make child, return -1. */
  if(cp == NULL)
    return -1;
  /* If fail to load(), reutrn -1. */
  if (cp->is_load == false)
    return -1;
  else
    return tid;
}

int
wait (tid_t tid)
{
  /* Wait the given tid's thread is finish.
     It calls process_wait(). */
  return process_wait (tid);
}

int
open (const char *file)
{
  /* Return -1 for inproper input. */
  if(file==NULL || strlen(file)==0)
    return -1;
  struct file *f;
  /* Get lock to other process can't access the file.
     Before every return release lock. */
  lock_acquire (&filesys_lock);
  f = filesys_open(file);
  if (f == NULL)
    {
      lock_release(&filesys_lock);
      return -1;
    }
  /* If the file is opened, don't allow write that. */
  if(strcmp(file,thread_current()->name)==0)
    file_deny_write(f);
  int results;
  /* Add file to file descriptor table and get the file descriptor. */
  results=process_add_file (f);
  lock_release(&filesys_lock);
  if(results==128)
    return -1;
  return results;
}

int
filesize(int fd)
{
  /* Return the length of given file.
     To get pointer of file, use process_get_file(),
     and call file_length(). */
  struct file *f;
  f = process_get_file (fd);
  if (f == NULL)
    return -1;
  return file_length (f);
}

int
read (int fd, void *buffer, unsigned size)
{
  struct file *f;
  int i;
  /* Get lock to other process can't access the file.
     Before every return release lock. */
  lock_acquire (&filesys_lock);
  f = process_get_file (fd);
  /* If fd is 0, it mean STDIN, so use input_getc() to get input.
     Other case, read the inforamtion of file using file_read().
     Return the size of input or read.*/
  if(fd == 0)
    {
      for (i = 0; i < size; i++)
      {

        *(uint8_t *)(buffer + i) = input_getc ();
      }
      lock_release (&filesys_lock);
      return size;
    }
  else
    {
      int sizes;
      if(f==NULL)
        {
          lock_release (&filesys_lock);
          return -1;
        }
      sizes=file_read(f,buffer,size);
      lock_release (&filesys_lock);
      return sizes;
    }
}

int
write (int fd, void *buffer, unsigned size)
{
  struct file *f;
  struct inode *inode;
  /* Get lock to other process can't access the file.
     Before every return release lock. */
  lock_acquire (&filesys_lock);
  f = process_get_file (fd);
  /* If fd is 1, it mean STDOUT, so use putbuf() to print BUFFER.
     Other case, write the BUFFER to the file using file_write().
     Return the size of print or write.*/
  if(fd == 1)
    {
      putbuf (buffer, size);
      lock_release (&filesys_lock);
      return size;
    }
  else
    {
      int sizes;
      if(f==NULL)
        {
          lock_release (&filesys_lock);
          return -1;
        }
      inode = file_get_inode(f);
      if(inode_is_dir(inode))
        {
          lock_release(&filesys_lock);
          return -1;
        }
      sizes=file_write(f,buffer,size);
      lock_release (&filesys_lock);
      return sizes;
    }
}

void
seek (int fd, unsigned position)
{
  struct file *f;
  /* Move the offset of opened file.
     Call file_seek to do task. */
  f = process_get_file (fd);
  file_seek(f, position);
}

unsigned
tell (int fd)
{
  /* Tell the offest of opened file.
     Call file_tell to do task. */
  struct file *f;
  f = process_get_file (fd);
  return file_tell (f);
}

void
close (int fd)
{
  /* Close the opened file correspond to fd.
     Call process_close_file to do task. */
  process_close_file (fd);
}

/* Check if given file is directory or file. */
bool
sys_isdir (int fd)
{
  struct file *file;
  file = process_get_file (fd);
  if (file == NULL)
    return false;
  return inode_is_dir (file_get_inode (file));
}

/* Change current directory. */
bool
sys_chdir (const char *dir)
{
  struct file *file = filesys_open (dir);
  struct inode *inode;

  /* Given text is single directory name. */
  if (file != NULL)
  {
    dir_close (thread_current ()->dir);
    thread_current()->dir = dir_open (file_get_inode (file));
    return true;
  }

  /* Given text is path name. */
  else if (dir_lookup (thread_current ()->dir, dir, &inode))
  {
    dir_close (thread_current ()->dir);
    thread_current ()->dir = dir_open (inode);
    return true;
  }
  return false;
}

/* Make new directory. */
bool
sys_mkdir (const char *dir)
{
  /* Directory name should be valid. */
  if (dir == NULL || strlen (dir) == 0)
    return false;
  return filesys_create_dir (dir);
}

/* Read next directory from current directory. */
bool
sys_readdir(int fd, char *name)
{
  struct file* file;
  struct dir *dir;
  bool result;
  file = process_get_file (fd);

  /* Check if given directory is directory or file. */
  if (!inode_is_dir (file_get_inode (file)))
    return false;
  dir = (struct dir *)file;
  result = dir_readdir (dir, name);

  /* Skip if next directory is ".." or "." */
  while (strcmp (name, ".") == 0 || strcmp (name, "..") == 0)
  {
    if (result == false) break;
    result = dir_readdir (dir, name);
  }
  return result;
}

/* Get file's block sector number. */
uint32_t
sys_inumber (int fd)
{
  struct file* file;
  file = process_get_file (fd);
  if (file == NULL)
    return -1;
  return inode_get_inumber (file_get_inode (file));
}

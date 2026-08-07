//
// Created by nick on 8/5/26.
//

#ifndef BENCTH_SYSCALL_H
#define BENCTH_SYSCALL_H

#include "bencthc/src/utils/iHateLibC.h"

//0
long b_syscall_read(int fd, void *buf, size_t count);

//1
long b_syscall_write(int fd, const void *buf, size_t count);

//2

//I'm commenting out unused flags

//https://man7.org/linux/man-pages/man2/open.2.html
#define O_RDONLY  0
#define O_WRONLY  1
//#define O_RDWR    2
#define O_CREAT   0100
#define O_TRUNC   01000
//#define O_APPEND  02000
//#define O_EXCL    0200

//claude formatted
//
// owner (user) permissions
//#define S_IRWXU 00700  // read, write, and execute
#define S_IRUSR 00400  // read
#define S_IWUSR 00200  // write
//#define S_IXUSR 00100  // execute

// group permissions
//#define S_IRWXG 00070  // read, write, and execute
#define S_IRGRP 00040  // read
//#define S_IWGRP 00020  // write
//#define S_IXGRP 00010  // execute

// other (everyone else) permissions
//#define S_IRWXO 00007  // read, write, and execute
#define S_IROTH 00004  // read
//#define S_IWOTH 00002  // write
//#define S_IXOTH 00001  // execute

int b_syscall_open(const char* path, int flags, int mode);

//3
int b_syscall_close(int fd);

//5

//https://github.com/torvalds/linux/blob/master/arch/x86/include/uapi/asm/stat.h
struct stat {
  unsigned long st_dev;
  unsigned long st_ino;
  unsigned long st_nlink;

  unsigned int  st_mode;
  unsigned int  st_uid;
  unsigned int  st_gid;
  unsigned int  __pad0;
  unsigned long st_rdev;
  long          st_size;
  long          st_blksize;
  long          st_blocks;

  unsigned long st_atime;
  unsigned long st_atime_nsec;
  unsigned long st_mtime;
  unsigned long st_mtime_nsec;
  unsigned long st_ctime;
  unsigned long st_ctime_nsec;
  long          __unused[3];
};

int b_syscall_fstat(int fd, struct stat* statbuf);

//8

//typedef long __kernel_long_t; //https://elixir.bootlin.com/linux/v7.1.6/source/arch/sparc/include/uapi/asm/posix_types.h#L22
//typedef __kernel_long_t __kernel_off_t; //https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/asm-generic/posix_types.h#L87
//typedef __kernel_off_t off_t; //https://elixir.bootlin.com/linux/v7.1.6/source/include/linux/types.h#L25
typedef long off_t;

//unused flags are commented out

//https://elixir.bootlin.com/linux/v7.1.7/source/include/uapi/linux/fs.h#L52
#define SEEK_SET	0	/* seek relative to beginning of file */
//#define SEEK_CUR	1	/* seek relative to current file position */
//#define SEEK_END	2	/* seek relative to end of file */
//#define SEEK_DATA	3	/* seek to the next data */
//#define SEEK_HOLE	4	/* seek to the next hole */

off_t b_syscall_lseek(int fd, off_t offset, int whence);

//9

//I'm commenting out unused flags

//https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/asm-generic/mman-common.h#L10
#define PROT_READ	      0x1		      /* page can be read */
#define PROT_WRITE	    0x2		      /* page can be written */
//#define PROT_EXEC	      0x4		      /* page can be executed */
//#define PROT_SEM	      0x8		      /* page may be used for atomic ops */
/*			                0x10		     * reserved for arch-specific use */
/*			                0x20		     * reserved for arch-specific use */
//#define PROT_NONE	      0x0		      /* page can not be accessed */
//#define PROT_GROWSDOWN	0x01000000	/* mprotect flag: extend change to start of growsdown vma */
//#define PROT_GROWSUP	  0x02000000	/* mprotect flag: extend change to end of growsup vma */

//https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/linux/mman.h#L17
//#define MAP_SHARED	        0x01		/* Share changes */
#define MAP_PRIVATE	        0x02		/* Changes are private */
//#define MAP_SHARED_VALIDATE 0x03	  /* share + validate extension flags */
//#define MAP_DROPPABLE	      0x08		/* Zero memory under memory pressure. */

//https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/asm-generic/mman-common.h#L20
//#define MAP_TYPE	    0x0f		/* Mask for type of mapping */
//#define MAP_FIXED	    0x10		/* Interpret addr exactly */
#define MAP_ANONYMOUS	0x20		/* don't use a file */

//https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/asm-generic/mman.h#L7
//#define MAP_GROWSDOWN	  0x0100		/* stack-like segment */
//#define MAP_DENYWRITE	  0x0800		/* ETXTBSY */
//#define MAP_EXECUTABLE	0x1000		/* mark it as an executable */
//#define MAP_LOCKED	    0x2000		/* pages are locked */
//#define MAP_NORESERVE	  0x4000		/* don't check for reservations */

//https://elixir.bootlin.com/linux/v7.1.6/source/include/uapi/asm-generic/mman-common.h#L26
//#define MAP_POPULATE		    0x008000	/* populate (prefault) pagetables */
//#define MAP_NONBLOCK		    0x010000	/* do not block on IO */
//#define MAP_STACK		        0x020000	/* give out an address that is best suited for process/thread stacks */
//#define MAP_HUGETLB		      0x040000	/* create a huge page mapping */
//#define MAP_SYNC		        0x080000  /* perform synchronous page faults for the mapping */
//#define MAP_FIXED_NOREPLACE	0x100000	/* MAP_FIXED which doesn't unmap underlying mapping */
//#define MAP_UNINITIALIZED   0x4000000	/* For anonymous mmap, memory could be
//					                             * uninitialized */

void* b_syscall_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);

//11
int b_syscall_munmap(void* addr, size_t length);

//60
_Noreturn void b_syscall_exit(int status);

#endif //BENCTH_SYSCALL_H
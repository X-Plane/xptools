/* 
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */
#ifndef HL_TYPES_H
#define HL_TYPES_H

#include <time.h>

#define strDIM 600

typedef unsigned char xbyt;
typedef char xchr;
typedef int xint;
typedef float xflt;

#if SIM || WRL
#error DO NOT INCLUDE THIS FILE FROM X-PLANE!
#endif

#if APL && !defined(__MACH__)
	#include <stdint.h>
	#include <CGBase.h>
	#define PROT_READ 0x01
	#define O_RDONLY  0x0000
	#define MAP_FILE  0x0000
	typedef int64_t	   off_t;
	typedef u_int16_t mode_t;
	typedef	u_int32_t	ino_t;		/* inode number */
	typedef	u_int16_t	nlink_t;	/* link count */
	typedef	int32_t		dev_t;		/* device number */
	typedef	u_int32_t	uid_t;		/* user id */
	typedef	u_int32_t	gid_t;		/* group id */
	struct timespec {
		time_t	tv_sec;		/* seconds */
		int32_t	tv_nsec;	/* and nanoseconds */
	};

	struct stat {
		dev_t	  st_dev;		/* inode's device */
		ino_t	  st_ino;		/* inode's number */
		mode_t	  st_mode;		/* inode protection mode */
		nlink_t	  st_nlink;		/* number of hard links */
		uid_t	  st_uid;		/* user ID of the file's owner */
		gid_t	  st_gid;		/* group ID of the file's group */
		dev_t	  st_rdev;		/* device type */
	#ifndef _POSIX_SOURCE
		struct	timespec st_atimespec;	/* time of last access */
		struct	timespec st_mtimespec;	/* time of last data modification */
		struct	timespec st_ctimespec;	/* time of last file status change */
	#else
		time_t	  st_atime;		/* time of last access */
		long	  st_atimensec;		/* nsec of last access */
		time_t	  st_mtime;		/* time of last data modification */
		long	  st_mtimensec;		/* nsec of last data modification */
		time_t	  st_ctime;		/* time of last file status change */
		long	  st_ctimensec;		/* nsec of last file status change */
	#endif
		off_t	  st_size;		/* file size, in bytes */
		int64_t	  st_blocks;		/* blocks allocated for file */
		u_int32_t st_blksize;		/* optimal blocksize for I/O */
		u_int32_t st_flags;		/* user defined flags for file */
		u_int32_t st_gen;		/* file generation number */
		int32_t	  st_lspare;
		int64_t	  st_qspare[2];
	};


	extern int	(* bsd_open	 )(const char *path,int flags,mode_t mode);
	extern int	(* bsd_close )(int d);
	extern off_t(* bsd_lseek )(int fildes,off_t offset,int whence);
	extern void*(* bsd_mmap	 )(void * start,size_t length,int prot ,int flags,int fd,off_t offset);
	extern int	(* bsd_munmap)(void * start,size_t length);
	extern int  (* bsd_fstat )(int , struct stat *);


#endif

enum {
	t_exit = 0
};

#define MACIBM_alert(win,c1,c2,c3,c4,type) __MACIBM_alert((win),(c1),(c2),(c3),(c4),(type),__FILE__,__LINE__)
void  __MACIBM_alert(xint close_to,	string C_string1,			// these guys are of unknown length because
									string C_string2,			// we are always sending in string literals
									string C_string3,			// of completely varying lengths!
									string C_string4,xint type,	// therefore we can NOT dimension the pointers here!
									const xchr * file,xint line);


extern void * (* xmalloc)(size_t);
extern void (* xfree)(void *);

#endif /* HL_TYPES_H */


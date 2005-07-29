/*
 * Warning, this file was automatically created by the TIFF configure script
 * VERSION:	 v3.6.1
 * RELEASE:   
 * DATE:	 Fri Jun 25 00:08:01 PDT 2004
 * TARGET:	 powerpc-apple-darwin7.4.0
 * CCOMPILER:	 /usr/bin/gcc-3.3.20030304 (Apple Computer, Inc. build 1495)
 */
#ifndef _PORT_
#define _PORT_ 1
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
#define HOST_FILLORDER FILLORDER_MSB2LSB
#define HOST_BIGENDIAN	1
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
typedef double dblparam_t;
#ifdef __STRICT_ANSI__
#define	INLINE	__inline__
#else
#define	INLINE	inline
#endif
#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME
#ifdef __cplusplus
}
#endif
#endif

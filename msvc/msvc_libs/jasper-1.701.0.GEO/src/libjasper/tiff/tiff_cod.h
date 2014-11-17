/*
 * Copyright (c) 2003-2006 Dmitry V. Fedorov <www.dimin.net>
 * DPI, INPE <www.dpi.inpe.br>
 * All rights reserved.
 */

/*
 * TIFF File Library
 *
 * $Id$
 */

#ifndef TIFF_COD_H
#define TIFF_COD_H


// define this if OS is windows
#if defined(WIN32)
  #ifndef DIM_OS_WIN
  #define DIM_OS_WIN
  #endif
#endif

#include <math.h>
#if defined(WIN32)
#include <limits.h>
#endif

#ifndef USHRT_MAX
  #define USHRT_MAX 0xffff
#endif


/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jasper/jas_types.h"

//#include "jasper/jas_stream.h"
//#include "jasper/jas_image.h"
//#include "jasper/jas_malloc.h"

#if defined (DIM_OS_WIN)
  #ifndef MAP_FAILED
  #define MAP_FAILED 1
  #endif
#else
  #include <sys/mman.h>
#endif

#include "../tiffgeo/tiffio.h"
#include "../tiffgeo/tiff.h"


/******************************************************************************\
* Constants and macros.
\******************************************************************************/

// The signature for a TIFF file
#define	TIFF_MAGIC1 "\x049\x049\x02A\x000"
#define	TIFF_MAGIC2 "\x04D\x04D\x000\x02A"

#define JAS_TIFF_MAX_COMPONENTS 100

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

tsize_t tiff_read(thandle_t handle,tdata_t data,tsize_t size);

tsize_t tiff_write(thandle_t handle,tdata_t data,tsize_t size);

toff_t tiff_seek(thandle_t handle, toff_t offset, int whence);

int tiff_close(thandle_t handle);

toff_t tiff_size(thandle_t handle);

int tiff_mmap(thandle_t handle,tdata_t* data,toff_t* size);

void tiff_unmap(thandle_t handle, tdata_t data, toff_t size);


#endif

/*
 * Copyright (c) 2003 Dmitry V. Fedorov <www.dimin.net>
 * DPI, INPE <www.dpi.inpe.br>
 * All rights reserved.
 */


/*
 * TIFF File Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_types.h"
#include "jasper/jas_image.h"
#include "jasper/jas_malloc.h"

#include "tiff_cod.h"

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

tsize_t tiff_read(thandle_t handle,tdata_t data,tsize_t size) 
{
  jas_stream_t *stream = (jas_stream_t *) handle;

  // Write characters from a buffer to a stream. 
  return jas_stream_read(stream, (char*) data, size);

}

tsize_t tiff_write(thandle_t handle,tdata_t data,tsize_t size) 
{
  jas_stream_t *stream = (jas_stream_t *) handle;

  // Write characters from a buffer to a stream. 
  return jas_stream_write(stream, (const char*) data, size);
}

// returns the current file position. libtiff wants that.
toff_t tiff_seek(thandle_t handle, toff_t offset, int whence) 
{
  jas_stream_t *stream = (jas_stream_t *) handle;

  // Is it possible to seek on this stream?
  //int jas_stream_isseekable(jas_stream_t *stream);

  // Set the current position within the stream.
  return jas_stream_seek(stream, offset, whence);
}

// This is a dummy, the IO device's owner will close it.
int tiff_close(thandle_t handle) 
{
  jas_stream_t *stream = (jas_stream_t *) handle;
  
  jas_stream_flush(stream);
  jas_stream_close(stream);

  return 0;
}

toff_t tiff_size(thandle_t handle) {
  jas_stream_t *stream = (jas_stream_t *) handle;
  return jas_stream_length(stream);
}

// warning always returns MAP_FAILED.
int tiff_mmap(thandle_t handle,tdata_t* data,toff_t* size) {
  handle=handle; data=data; size=size;
  return (int) MAP_FAILED;
}

// warning because you can't mmap, this is a dummy.
void tiff_unmap(thandle_t handle, tdata_t data, toff_t size) {
  handle=handle; data=data; size=size;
}






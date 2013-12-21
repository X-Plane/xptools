/******************************************************************************
 * Copyright (c) 2003-06 Dmitry V. Fedorov <www.dimin.net>
 * DPI, INPE <www.dpi.inpe.br>
 * All rights reserved.

  For this implementation to work following function in jas_stream.c was changed 
    jas_stream_t *jas_stream_fopen(const char *filename, const char *mode)
  Line 325
  from:
	  obj->pathname[0] = '\0';
  to:
    //strncpy(obj->pathname, filename, L_tmpnam);
    strcpy(obj->pathname, filename);

  Changes: jas_image_t 

  History:
    09/06/2003 17:22 - First creation
    11/07/2003 16:48 - GeoTiff support
    19/09/2003 18:07 - New TIFF wrapper, support for Tiled, Strip modes
                       Multiple (RGB in 3 separate images) TIFF support
    17/11/2003 14:53 - correct PHOTOMETRIC_MINISWHITE
                       add TFW support
    04/20/2005 17:19 - Print geotiff with "listgeo" argument
    03/19/2006 16:15 - load 16 bit tiffs preserving bit rate
    04/09/2006 13:57 - Create RGB image for 3 or more componets (preserve channels)
    04/18/2006 02:57 - use JAS_TIFF_MAX_COMPONENTS for static array of channels

  ver: 9
******************************************************************************/

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_malloc.h"

#include "tiff_cod.h"

//#include <tiffio.h>
//#include <tiff.h>
#include "../tiffgeo/tiffio.h"
#include "../tiffgeo/tiff.h"

// geotifflib
#include "../tiffgeo/xtiffio.h"
#include "../tiffgeo/geotiff.h"

#include "../tiffgeo/geo_tiffp.h"
#include "../tiffgeo/geo_keyp.h"

#include "tif_memio.h"
#include "geotiff_buffer.h"

//#define TIFFOpen XTIFFOpen
//#define TIFFClose XTIFFClose
#define TIFFClientOpen XTIFFClientOpen

#define MAX_LEN 1024 

//****************************************************************************
// UTILS
//****************************************************************************
void getTFWFileName(char *s, char *o)
{
  char *st;
  int p=0;
  st = strrchr( s, '.' );
  if (st != NULL) p = st - s; else p = strlen(s);
  
  strncpy( o, s, p );
  sprintf(o, "%s.tfw", o);
}

int isFileExists(char *fileName)
{
  int res=0;
  FILE *stream;
  
  // Open for read (will fail if file fileName does not exist)
  if ( (stream  = fopen( fileName, "r" )) != NULL )
  {
    res = 1;
    fclose( stream );
  }
  
  return res;
}

//****************************************************************************
// TILED TIFF
//****************************************************************************

static int read_tiled_RGBA_tiff(TIFF *tif, jas_matrix_t *cmpts[], jas_image_t *image)
{
  uint32 columns, rows;
  uint32 *tile_buf;
  uint32 x, y;
  uint32 height = 0; 
  uint32 width = 0;
  uint32 tileW, tileH;
  uint_fast16_t numcmpts = image->maxcmpts_;
	uint16 cmptno;

  if (tif == NULL) return -1;

  // if tiff is not tiled get out and never come back :-)
  if( !TIFFIsTiled(tif) ) return -1;

  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  tile_buf = (uint32*) _TIFFmalloc(columns*rows*sizeof(uint32));
  if (tile_buf == NULL) return -1;

  tileW = columns; 
  tileH = rows;

  for (y = 0; y < height; y += rows)
  {
    // the tile height may vary 
    if (height-y < rows) tileH = height-y;

    tileW = columns;
    for (x = 0; x < width; x += columns)
    {
      register uint32 yi, xi;
      uint32 tW;
      
      if (!TIFFReadRGBATile(tif, x, y, tile_buf)) break;

      // the tile size is now treated by libtiff guys the
      // way that the size stay on unchanged      
      if (width-x < columns) tW = width-x; else tW = tileW;

      // now put tile into the image
      for(yi = 0; yi < tileH; yi++) 
      {
        for(xi = 0; xi < tW; xi++)  
        {
          const uint32 pix = *(tile_buf + ((rows-1-yi)*tileW)+xi);
          jas_matrix_set(cmpts[0], yi, xi+x, (int)TIFFGetR(pix));
          jas_matrix_set(cmpts[1], yi, xi+x, (int)TIFFGetG(pix));
          jas_matrix_set(cmpts[2], yi, xi+x, (int)TIFFGetB(pix));
        } // for xi
      } // for yi
    } // for x

    for (cmptno = 0; cmptno < numcmpts; ++cmptno)
    {
      if (jas_image_writecmpt(image, cmptno, 0, y, width, tileH, cmpts[cmptno])) return -1;
    }

  } // for y


  _TIFFfree(tile_buf);

  return 0;
}

static int read_tiled_8BIT_tiff(TIFF *tif, jas_matrix_t *cmpts, jas_image_t *image,
                                int cmptno)
{
  uint32 columns, rows;
  uchar *tile_buf;
  uint32 x, y;
  uint32 height = 0; 
  uint32 width = 0;
  uint32 tileW, tileH;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;

  if (tif == NULL) return -1;

  // if tiff is not tiled get out and never come back :-)
  if( !TIFFIsTiled(tif) ) return -1;

  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

  tile_buf = (uchar*) _TIFFmalloc(TIFFTileSize(tif));
  if (tile_buf == NULL) return -1;

  tileW = columns; 
  tileH = rows;

  for (y = 0; y < height; y += rows)
  {
    //fprintf(stdout, "\n"); // if text out

    // the tile height may vary 
    if (height-y < rows) tileH = height-y;

    tileW = columns;
    for (x = 0; x < width; x += columns)
    {
      register uint32 yi, xi;
      uint32 tW;
      
      if (!TIFFReadTile(tif, tile_buf, x, y, 0, 0)) break;
      //fprintf(stdout, "."); // if text out

      // the tile size is now treated by libtiff guys the
      // way that the size stay on unchanged      
      if (width-x < columns) tW = width-x; else tW = tileW;

      if (photometric != PHOTOMETRIC_MINISWHITE) {
        // now put tile into the image
        for(yi = 0; yi < tileH; yi++) {
          for(xi = 0; xi < tW; xi++) {
            const uchar pix = *(tile_buf + (yi*tileW)+xi);
            jas_matrix_set(cmpts, yi, xi+x, (int) pix );
          } // for xi
        } // for yi
      }
      else { // if PHOTOMETRIC_MINISWHITE
        for(yi = 0; yi < tileH; yi++) {
          for(xi = 0; xi < tW; xi++) {
            const uchar pix = *(tile_buf + (yi*tileW)+xi);
            jas_matrix_set(cmpts, yi, xi+x, (int) 255-pix );
          } // for xi
        } // for yi
      } // if PHOTOMETRIC_MINISWHITE
    } // for x
    if (jas_image_writecmpt(image, cmptno, 0, y, width, tileH, cmpts)) return -1;
  } // for y

  _TIFFfree(tile_buf);

  return 0;
}

static int read_tiled_16BIT_tiff(TIFF *tif, jas_matrix_t *cmpts, jas_image_t *image, int cmptno)
{
  uint32 columns, rows;
  unsigned short *tile_buf;
  uint32 x, y;
  uint32 height = 0; 
  uint32 width = 0;
  uint32 tileW, tileH;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;

  if (tif == NULL) return -1;

  // if tiff is not tiled get out and never come back :-)
  if( !TIFFIsTiled(tif) ) return -1;

  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

  tile_buf = (unsigned short*) _TIFFmalloc(TIFFTileSize(tif));
  if (tile_buf == NULL) return -1;

  tileW = columns; 
  tileH = rows;

  for (y = 0; y < height; y += rows)
  {
    //fprintf(stdout, "\n"); // if text out

    // the tile height may vary 
    if (height-y < rows) tileH = height-y;

    tileW = columns;
    for (x = 0; x < width; x += columns)
    {
      register uint32 yi, xi;
      uint32 tW;
      
      if (!TIFFReadTile(tif, tile_buf, x, y, 0, 0)) break;


      // the tile size is now treated by libtiff guys the
      // way that the size stay on unchanged      
      if (width-x < columns) tW = width-x; else tW = tileW;

      if (photometric != PHOTOMETRIC_MINISWHITE) {
        // now put tile into the image
        for(yi = 0; yi < tileH; yi++) {
          for(xi = 0; xi < tW; xi++) {
            const unsigned short pix = *(tile_buf + (yi*tileW)+xi);
            jas_matrix_set(cmpts, yi, xi+x, (int) pix );
          } // for xi
        } // for yi
      }
      else { // if PHOTOMETRIC_MINISWHITE
        for(yi = 0; yi < tileH; yi++) {
          for(xi = 0; xi < tW; xi++) {
            const unsigned short pix = *(tile_buf + (yi*tileW)+xi);
            jas_matrix_set(cmpts, yi, xi+x, (int) USHRT_MAX-pix );
          } // for xi
        } // for yi
      } // if PHOTOMETRIC_MINISWHITE
    } // for x
    if (jas_image_writecmpt(image, cmptno, 0, y, width, tileH, cmpts)) return -1;
  } // for y

  _TIFFfree(tile_buf);

  return 0;
}


static void invert_buffer( void* buf, unsigned int size_in_pixels, unsigned int bitspersample)
{
  uint32 x; 

  if (bitspersample == 8) 
  {
    uchar *pix = (uchar*) buf;
    for(x=0; x<size_in_pixels; x++) pix[x] = 255-pix[x];              
  }

  if (bitspersample == 16) 
  {
    unsigned short *pix = (unsigned short *) buf;
    for(x=0; x<size_in_pixels; x++) pix[x] = USHRT_MAX-pix[x];              
  }
}

static int read_tiled_tiff(TIFF *tif, jas_image_t *image)
{
  uint32 columns, rows;
  unsigned short *tile_buf;
  uint32 x, y;
  uint32 height = 0; 
  uint32 width = 0;
  uint32 tileW, tileH;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 planarConfig;
  uint16 numcmpts=1;
	uint16 bitspersample = 1;
  uint32 cmptno;
  jas_matrix_t *bufs=NULL;

  if (tif == NULL) return -1;
  if( !TIFFIsTiled(tif) ) return -1;

  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &numcmpts);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

  tile_buf = (unsigned short*) _TIFFmalloc(TIFFTileSize(tif));
  if (tile_buf == NULL) return -1;

  bufs = jas_matrix_create(rows, columns);
  if (!bufs) {
    jas_matrix_destroy(bufs);
    return -1;
  }

  //---------------------------------------------------------------------
  // planar separate or only one sample
  //---------------------------------------------------------------------  
  if ( (planarConfig == PLANARCONFIG_SEPARATE) || (numcmpts == 1) )
  {
    for (cmptno=0; cmptno<numcmpts; ++cmptno)
    {
      tileH = rows;    
      for (y=0; y<height; y+=rows)
      {
        // the tile height may vary 
        if (height-y<rows) tileH = height-y;

        tileW = columns;
        for (x=0; x<width; x+=columns)
        {
          register uint32 yi, xi;

          if (!TIFFReadTile(tif, (tdata_t)tile_buf, x, y, 0, (tsample_t)cmptno)) break;  

          if (photometric == PHOTOMETRIC_MINISWHITE)
            invert_buffer( tile_buf, rows*columns, bitspersample);

          // the tile size is now treated by libtiff guys the
          // way that the size stay on unchanged      
          if (width-x<columns) tileW = width-x;

          // write pixels 8 bits
          if (bitspersample == 8)
          for(yi=0; yi<tileH; yi++) {
            uchar *pix = ((uchar*)tile_buf) + (yi*columns);
            for(xi=0; xi<tileW; xi++)
              jas_matrix_set(bufs, yi, xi, (int) pix[xi] );
          } // for yi

          // write pixels 16 bits
          if (bitspersample == 16)
          for(yi=0; yi<tileH; yi++) {
            unsigned short *pix = ((unsigned short*)tile_buf) + (yi*columns);
            for(xi=0; xi<tileW; xi++)
              jas_matrix_set(bufs, yi, xi, (int) pix[xi] );
          } // for yi

          // write the buffer
	        if (jas_image_writecmpt(image, cmptno, x, y, tileW, tileH, bufs)) return -1;

        } // for x
      } // for y
    } // for cmptno
  } // separate planes or one channel
  
  
  //---------------------------------------------------------------------
  // planar interleaved
  //---------------------------------------------------------------------   
  else
  {
    uint xp;
    tileH = rows;    
    for (y=0; y<height; y+=rows)
    {
      // the tile height may vary 
      if (height-y<rows) tileH = height-y;

      tileW = columns;
      for (x=0; x<width; x+=columns)
      {
        register uint32 yi, xi;

        if (!TIFFReadTile(tif, (tdata_t)tile_buf, x, y, 0, 0)) break;  

        if (photometric == PHOTOMETRIC_MINISWHITE)
          invert_buffer( tile_buf, rows*columns*numcmpts, bitspersample);

        // the tile size is now treated by libtiff guys the
        // way that the size stay on unchanged      
        if (width-x<columns) tileW = width-x;

        for (cmptno=0; cmptno<numcmpts; ++cmptno)
        {
          // write pixels 8 bits
          if (bitspersample == 8)
          for(yi=0; yi<tileH; yi++) {
            uchar *pix = ((uchar*)tile_buf) + (yi*columns) + cmptno;
            xp = 0;
            for(xi=0; xi<tileW; xi++)
            {
              jas_matrix_set(bufs, yi, xi, (int) pix[xp] );
              xp+=numcmpts;
            }
          } // for yi

          // write pixels 16 bits
          if (bitspersample == 16)
          for(yi=0; yi<tileH; yi++) {
            unsigned short *pix = ((unsigned short*)tile_buf) + (yi*columns) + cmptno;
            xp = 0;
            for(xi=0; xi<tileW; xi++)
            {
              jas_matrix_set(bufs, yi, xi, (int) pix[xp] );
              xp+=numcmpts;
            }
          } // for yi

          // write the buffer
	        if (jas_image_writecmpt(image, cmptno, x, y, tileW, tileH, bufs)) return -1;

        } // for numcmpts

      } // for x
    } // for y
  } // interleaved channels

  jas_matrix_destroy(bufs);
  _TIFFfree(tile_buf);

  return 0;
}



//****************************************************************************
// STRIPED TIFF
//****************************************************************************
static int read_striped_RGBA_tiff(TIFF *tif, jas_matrix_t *cmpts[], jas_image_t *image)
{
  uint32 height = 0; 
  uint32 width = 0; 
  uint_fast16_t numcmpts = image->maxcmpts_;
	uint32 x = 0;
	uint32 y = 0;
	uint16 cmptno;
  uint32 *buf=NULL;
  uint32 rows;
  uint32 tileH;

  if (tif == NULL) return -1;

  // check if tiff is striped
  if( !TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows) ) return -1;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  buf = (uint32*) _TIFFmalloc(width*rows*sizeof(uint32));
  if (buf == NULL) return -1;

  tileH = rows;

  for (y = 0; y < height; y += rows)
  {
    uint32 yi;
    uint32 *inp = buf;

    // the tile height may vary 
    if (height-y < rows) tileH = height-y; 

    if (!TIFFReadRGBAStrip(tif, y, buf)) break;

    for(yi = 0; yi < tileH; yi++) 
    {
      for(x = 0; x < width; x++)  
      {
        const uint32 col = *(inp++);
        jas_matrix_setv(cmpts[0], x, (int)TIFFGetR(col));
        jas_matrix_setv(cmpts[1], x, (int)TIFFGetG(col));
        jas_matrix_setv(cmpts[2], x, (int)TIFFGetB(col));
      } // for x

      for (cmptno = 0; cmptno < numcmpts; ++cmptno)
      {
  			if (jas_image_writecmpt(image, cmptno, 0, y + (tileH-1-yi), 
                                width, 1, cmpts[cmptno])) return -1;
      }
    } // for yi
  } // for y

  _TIFFfree(buf);

  return 0;
}

//****************************************************************************
// SCANLINE METHOD TIFF
//****************************************************************************

void write_line_segment_8(jas_matrix_t *cmpts[], void *bufo, int nsamples, unsigned int sample, unsigned long w, uint16 photometric)
{
  uchar *buf = (uchar *) bufo;  
  register unsigned int x, xi=0;

  if (photometric != PHOTOMETRIC_MINISWHITE)
  for (x=sample; x<w*nsamples; x+=nsamples) 
  {
    jas_matrix_setv(cmpts[sample], xi, (int) buf[x] );
    xi++;
  }

  if (photometric == PHOTOMETRIC_MINISWHITE)
  for (x=sample; x<w*nsamples; x+=nsamples) 
  {
    jas_matrix_setv(cmpts[sample], xi, (int) 255-buf[x] );
    xi++;
  }

}

void write_line_segment_16(jas_matrix_t *cmpts[], void *bufo, int nsamples, unsigned int sample, unsigned long w, uint16 photometric)
{
  unsigned short *buf = (unsigned short *) bufo;  
  register unsigned int x, xi=0;
  
  if (photometric != PHOTOMETRIC_MINISWHITE)
  for (x=sample; x<w*nsamples; x+=nsamples) 
  {
    jas_matrix_setv(cmpts[sample], xi, (int) buf[x] );
    xi++;
  }

  if (photometric == PHOTOMETRIC_MINISWHITE)
  for (x=sample; x<w*nsamples; x+=nsamples) 
  {
    jas_matrix_setv(cmpts[sample], xi, (int) USHRT_MAX-buf[x] );
    xi++;
  }

}

// the simplies default method, it takes too many memory to load
static int read_scanline_tiff(TIFF *tif, jas_matrix_t *cmpts[], jas_image_t *image)
{
  uchar *bits;
  uint32 height = 0; 
  uint32 width = 0;
	uint32 x = 0;
	uint32 y = 0;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  uint16 planarConfig;
	uint16 samplesperpixel = 1;
	uint16 bitspersample = 1;
  int cmptno;

  if (tif == NULL) return -1;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,   &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH,  &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC,  &photometric);
  TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);


  if ( (planarConfig == PLANARCONFIG_SEPARATE) || (samplesperpixel == 1) )
  {
    bits = (uchar*) _TIFFmalloc( TIFFScanlineSize(tif) );
    if (!bits) return -1;

    for (cmptno=0; cmptno<samplesperpixel; ++cmptno)
    for(y=0; y<height; ++y)
    {
      TIFFReadScanline(tif, bits, y, cmptno);

      if (bitspersample == 8)
      {
        if (photometric != PHOTOMETRIC_MINISWHITE) {
          for(x=0; x<width; x++) 
            jas_matrix_setv(cmpts[cmptno], x, (int) bits[x] );
        }
        else {
          for(x=0; x<width; x++) 
            jas_matrix_setv(cmpts[cmptno], x, (int) (255-bits[x]) );
        }
      }

      if (bitspersample == 16)
      {
        unsigned short *bits16 = (unsigned short *) bits;

        if (photometric != PHOTOMETRIC_MINISWHITE) {
          for(x=0; x<width; x++) 
            jas_matrix_setv(cmpts[cmptno], x, (int) bits16[x] );
        }
        else {
          for(x=0; x<width; x++) 
            jas_matrix_setv(cmpts[cmptno], x, (int) (USHRT_MAX-bits16[x]) );
        }
      }
          
      if (jas_image_writecmpt(image, cmptno, 0, y, width, 1, cmpts[cmptno])) return -1;
    } // for y

    _TIFFfree(bits);
  } // if planar
  else // if image contain several samples in one same plane ex: RGBRGBRGB...
  {
    uchar *bits = (uchar *) _TIFFmalloc( TIFFScanlineSize(tif) );
    if (!bits) return -1;

    for(y=0; y<height; ++y) 
    {
      TIFFReadScanline(tif, bits, y, 0);

      for (cmptno=0; cmptno<samplesperpixel; ++cmptno)
      {
        if (bitspersample == 8)
          write_line_segment_8(cmpts, bits, samplesperpixel, cmptno, width, photometric);

        if (bitspersample == 16)
          write_line_segment_16(cmpts, bits, samplesperpixel, cmptno, width, photometric);

        if (jas_image_writecmpt(image, cmptno, 0, y, width, 1, cmpts[cmptno])) return -1;
      }  // for sample

    } // for y

    _TIFFfree( bits );
  }

  return 0;
}

// the simplies default method, it takes too many memory to load
static int read_scanline_8BIT_tiff(TIFF *tif, jas_matrix_t *cmpts, jas_image_t *image, int cmptno)
{
  uchar *bits;
  uint32 height = 0; 
  uint32 width = 0;
	uint32 x = 0;
	uint32 y = 0;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;
  if (tif == NULL) return -1;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,   &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH,  &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC,  &photometric);

  bits = (uchar*) _TIFFmalloc(width * sizeof (uchar));
  if (!bits) return -1;

  for(y=0; y<height; y++)
  {
    TIFFReadScanline(tif, bits, y, 0);

    if (photometric != PHOTOMETRIC_MINISWHITE) {
      for(x=0; x<width; x++) 
        jas_matrix_setv(cmpts, x, (int) bits[x] );
    }
    else {
      for(x=0; x<width; x++) 
        jas_matrix_setv(cmpts, x, (int) (255-bits[x]) );
    }
  
    if (jas_image_writecmpt(image, cmptno, 0, y, width, 1, cmpts)) return -1;
	}
  _TIFFfree(bits);

  return 0;
}

static int read_scanline_16BIT_tiff(TIFF *tif, jas_matrix_t *cmpts, jas_image_t *image,
                                   int cmptno)
{
  unsigned short *bits;
  uint32 height = 0; 
  uint32 width = 0;
	uint32 x = 0;
	uint32 y = 0;
  uint16 photometric = PHOTOMETRIC_MINISWHITE;

  if (tif == NULL) return -1;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

  // as defined above palete will be all GRAY: i.e. NO PALETTE!!!
  bits = (unsigned short*) _TIFFmalloc(width * sizeof (unsigned short));
  if (!bits) return -1;

  for(y=0; y<height; y++)
  {
    TIFFReadScanline(tif, bits, y, 0);

    if (photometric != PHOTOMETRIC_MINISWHITE) {
      for(x=0; x<width; x++) 
        jas_matrix_setv(cmpts, x, (int) bits[x] );
    }
    else {
      for(x=0; x<width; x++) 
        jas_matrix_setv(cmpts, x, (int) (USHRT_MAX-bits[x]) );
    }
    
    if (jas_image_writecmpt(image, cmptno, 0, y, width, 1, cmpts)) return -1;
	}
  _TIFFfree(bits);
  return 0;
}


//****************************************************************************
// DEFAULT METHOD TIFF
//****************************************************************************

// the simplies default method, it takes too many memory to load
static int read_generic_RGBA_tiff(TIFF *tif, jas_matrix_t *cmpts[], jas_image_t *image)
{
  uint32 height = 0; 
  uint32 width = 0; 
  uint_fast16_t numcmpts = image->maxcmpts_;
  uint32 *bits; // buffer to hold the image
	uint32 x = 0;
	uint32 y = 0;
	uint16 cmptno;

  if (tif == NULL) return -1;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  bits=(uint32*) _TIFFmalloc(width * height * sizeof (uint32));
    
  if (!bits) return -1;

  if (TIFFReadRGBAImage(tif, width, height, bits, 0)) 
  {
    uint32 *inp;
    inp = bits;
    for(y=0; y<height; y++) 
    {

      for(x=0; x<width; x++) 
      {
        const uint32 col=*(inp++);

        jas_matrix_setv(cmpts[0], x, (int)TIFFGetR(col));
        jas_matrix_setv(cmpts[1], x, (int)TIFFGetG(col));
        jas_matrix_setv(cmpts[2], x, (int)TIFFGetB(col));
      }
		      
      for (cmptno = 0; cmptno < numcmpts; ++cmptno)
      {
			  uint32 yi = height-1-y;
  			if (jas_image_writecmpt(image, cmptno, 0, yi, width, 1, cmpts[cmptno])) return -1;
      }
    }
  }
  _TIFFfree(bits);

  return 0;
}


/******************************************************************************\
* Interface functions.
\******************************************************************************/

jas_image_t *tiff_decode(jas_stream_t *in, char *optstr)
{
	// jasper stuff
  jas_image_t *image;
	uint_fast16_t numcmpts;
	jas_image_cmptparm_t cmptparms[JAS_TIFF_MAX_COMPONENTS];
	jas_image_cmptparm_t *cmptparm;
	uint16 cmptno;

  // tiff info
  TIFF *tif, *tif2, *tif3;
  unsigned size = 0;
  uint32 height = 0; 
  uint32 width = 0; 
	uint16 bitspersample = 1;
	uint16 samplesperpixel = 1;
	uint32 rowsperstrip;  
	uint16 photometric = PHOTOMETRIC_MINISWHITE;
	uint16 compression = COMPRESSION_NONE;
	uint32 x = 0;
	uint32 y = 0;
  int bpsres = 8;
  
  char fn2[MAX_LEN] = "\0", fn3[MAX_LEN] = "\0";
  bool separateTiff = FALSE;
  char tfwName[MAX_LEN] = "\0";

  // temp params
  jas_matrix_t *cmpts[JAS_TIFF_MAX_COMPONENTS];
	int i;

  // input params
  jas_stream_fileobj_t *fo = in->obj_;

	if (optstr) {
    sscanf(optstr, "%s\n%s", fn2, fn3);
    fprintf(stdout, "Using multiple TIFF mode\n");
    fprintf(stdout, "  R: [%s]\n", fo->pathname);
    fprintf(stdout, "  G: [%s]\n", fn2);
    fprintf(stdout, "  B: [%s]\n", fn3);
    separateTiff = TRUE;
	}
  
  // open without memory mapping.
  tif = XTIFFClientOpen(fo->pathname, "rm",
                           (thandle_t) (in),
                           tiff_read,
                           tiff_write,
                           tiff_seek,
                           tiff_close,
                           tiff_size,
                           tiff_mmap,
                           tiff_unmap );
  if (!tif) return 0;


  if ( (optstr) && ( strstr(optstr, "listgeo") != NULL ) )
  {
    GTIFPrintFull( tif );

    XTIFFClose( tif );
    //return null; 
    exit(0);
  }

  if (separateTiff)
  {
    tif2 = TIFFOpen(fn2, "rm");
    if (!tif2) return 0;
    tif3 = TIFFOpen(fn3, "rm");
    if (!tif3) return 0;
  }  

  // read TIFF parameters
	TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);   
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  size=width*height;

  
  // Get the number of components.
	numcmpts = samplesperpixel;
  if (separateTiff) numcmpts = 3;

  // Get the resultant bits per samples 
  if (bitspersample == 16) bpsres = 16; else bpsres = 8;

	for (cmptno = 0, cmptparm = cmptparms; cmptno < numcmpts; ++cmptno, ++cmptparm) 
  {
		cmptparm->tlx = 0;
		cmptparm->tly = 0;
		cmptparm->hstep = 1;
		cmptparm->vstep = 1;
		cmptparm->width = width;
		cmptparm->height = height;
		cmptparm->prec = bpsres;
		cmptparm->sgnd = false;
	}

	// Create image object
	if (!(image = jas_image_create(numcmpts, cmptparms, JAS_CLRSPC_UNKNOWN))) {
		return 0;
	}

  //-----------------------------------------------------------------------
  // retreive GeoTiff info if some
  //-----------------------------------------------------------------------
  image->aux_buf.size = 0;
  image->aux_buf.buf = NULL;
  MemBufFromGTIF ( tif, &image->aux_buf.size, &image->aux_buf.buf );
  if (isGeoTiff(tif) == TRUE)
    fprintf(stdout, "GeoTiff found, size: %d\n", image->aux_buf.size);

  // try to read TFW file if exists - this will override GeoTiff information
  getTFWFileName(fo->pathname, tfwName);
  //fprintf(stdout, "file: [%s] twf: [%s]\n", fo->pathname, tfwName);

  if (isFileExists(tfwName) == 1) {
    fprintf(stdout, "World File found [%s], TFW used instead of GeoTiff!\n", tfwName);
    if (readWorldFile(tfwName, &image->aux_buf.size, &image->aux_buf.buf) == 2)
      fprintf(stdout, "Warning: Rotation is defined in TFW - discarded!\n");
  }
  //-----------------------------------------------------------------------

  for (i = 0; i < numcmpts; ++i)
	  jas_image_setcmpttype(image, i, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));

	if (numcmpts >= 3) 
  {
		jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
		jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
		jas_image_setcmpttype(image, 1, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
		jas_image_setcmpttype(image, 2, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
	} 
  else
  if (numcmpts == 1) 
  {
		jas_image_setclrspc(image, JAS_CLRSPC_SGRAY);
		jas_image_setcmpttype(image, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
  }  

  if ( !TIFFIsTiled(tif) )
  { 
    //-----------------------------------------------------------------------------
    // not tiled tiff
    //-----------------------------------------------------------------------------

	  // Create temporary matrices to hold component data.
	  for (i = 0; i < numcmpts; ++i) cmpts[i] = NULL;
	  for (i = 0; i < numcmpts; ++i)
		  if (!(cmpts[i] = jas_matrix_create(1, width))) {
	      jas_image_destroy(image);
	      return 0;
      }

    if (separateTiff)
    {
      // make a combination of three separate files

      if ( (bitspersample==8) && (samplesperpixel==1) )
      { 
        if ( (read_scanline_8BIT_tiff(tif , cmpts[0], image, 0) == -1) ||
             (read_scanline_8BIT_tiff(tif2, cmpts[1], image, 1) == -1) ||
             (read_scanline_8BIT_tiff(tif3, cmpts[2], image, 2) == -1) )
        { jas_image_destroy(image); return 0; }
      }

      if ( (bitspersample==16) && (samplesperpixel==1) )
      { 
        if ( (read_scanline_16BIT_tiff(tif , cmpts[0], image, 0) == -1) ||
             (read_scanline_16BIT_tiff(tif2, cmpts[1], image, 1) == -1) ||
             (read_scanline_16BIT_tiff(tif3, cmpts[2], image, 2) == -1) )
        { jas_image_destroy(image); return 0; }
      }
    }
    else    
    if ( (bitspersample==8) || (bitspersample == 16) )
    { 
      if (read_scanline_tiff(tif, cmpts, image) != 0) 
      { jas_image_destroy(image); return 0; }        
    }
    else // any other config
    {
      if (read_striped_RGBA_tiff(tif, cmpts, image) == -1)
      if (read_generic_RGBA_tiff(tif, cmpts, image) == -1)
      { jas_image_destroy(image); return 0; }
    }

  	// Destroy the temporary matrices
  	for (i = 0; i < numcmpts; ++i) if (cmpts[i]) jas_matrix_destroy(cmpts[i]);
  } // end not tiled tiff
  else
  {
    //-----------------------------------------------------------------------------    
    // tiled tiff
    //-----------------------------------------------------------------------------
    if ( (!separateTiff) && ( (bitspersample == 8) || (bitspersample == 16) ) )
    {
      read_tiled_tiff(tif, image);
    }
    else
    {
      uint32 rows;
      TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
	    
      // Create temporary matrices to hold component data.
	    for (i = 0; i < numcmpts; ++i) cmpts[i] = NULL;
	    for (i = 0; i < numcmpts; ++i)
		    if (!(cmpts[i] = jas_matrix_create(rows, width))) {
	        jas_image_destroy(image);
	        return 0;
        }

      if (separateTiff)
      {
        // if get channels from separate tiffs
        if ( (bitspersample == 8) && (samplesperpixel == 1) )
        { 
          if ( (read_tiled_8BIT_tiff(tif , cmpts[0], image, 0) == -1) ||
               (read_tiled_8BIT_tiff(tif2, cmpts[1], image, 1) == -1) ||
               (read_tiled_8BIT_tiff(tif3, cmpts[2], image, 2) == -1) )
          { jas_image_destroy(image); return 0; }
        }

        if ( (bitspersample == 16) && (samplesperpixel == 1) )
        { 
          if ( (read_tiled_16BIT_tiff(tif , cmpts[0], image, 0) == -1) ||
               (read_tiled_16BIT_tiff(tif2, cmpts[1], image, 1) == -1) ||
               (read_tiled_16BIT_tiff(tif3, cmpts[2], image, 2) == -1) )
          { jas_image_destroy(image); return 0; }
        }

      } // sep tiffs
      else // any other config
      {
        if (read_tiled_RGBA_tiff(tif, cmpts, image) == -1)
        if (read_generic_RGBA_tiff(tif, cmpts, image) == -1)
        { jas_image_destroy(image); return 0; }
      }

  	  // Destroy the temporary matrices
  	  for (i = 0; i < numcmpts; ++i) if (cmpts[i]) jas_matrix_destroy(cmpts[i]); 
    }
  }

  //TIFFClose(tif); // do not close file, it was open before...
  // clean up the mess we made
  if (separateTiff)
  {
    TIFFClose(tif2);
    TIFFClose(tif3);
  }

  fprintf(stdout, "TIFF file decoded!\n");

  return image;
}

int tiff_validate(jas_stream_t *in)
{
	int n;
	int i;
	uchar buf[4];

	assert(JAS_STREAM_MAXPUTBACK >= 2);

	// Read the first two characters that constitute the signature.
	if ((n = jas_stream_read(in, (char *) buf, 4)) < 0) {
		return -1;
	}
	// Put the characters read back onto the stream.
	for (i = n - 1; i >= 0; --i) {
		if (jas_stream_ungetc(in, buf[i]) == EOF) {
			return -1;
		}
	}
	// Did we read enough characters?
	if (n < 4) {
		return -1;
	}

  if (memcmp(buf, TIFF_MAGIC1, 4) == 0) return 0;
  if (memcmp(buf, TIFF_MAGIC2, 4) == 0) return 0;

	return -1;
}




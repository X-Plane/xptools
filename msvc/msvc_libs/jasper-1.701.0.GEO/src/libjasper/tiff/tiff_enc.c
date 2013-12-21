/******************************************************************************
 * Copyright (c) 2003-06 Dmitry V. Fedorov <www.dimin.net>
 * Copyright (c) 2003, DPI, INPE <www.dpi.inpe.br>
 * All rights reserved.

  History:
    09/06/2003 17:22 - First creation
    03/19/2006 16:15 - write 16 bit tiffs preserving original bit rate
    04/09/2006 13:57 - Preserve channels in images, create tiled images
    04/18/2006 02:57 - fixed bug with limit of 4 channels in the tiff_enc_t
    09/24/2006 18:40 - support for special bit rates, 12bpp...

  ver: 6
******************************************************************************/

#include <assert.h>
#include <math.h>

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_debug.h"

#include "tiff_cod.h"

#include "../tiffgeo/tiffio.h"
#include "../tiffgeo/tiff.h"

// geotifflib
#include "../tiffgeo/xtiffio.h"
#include "../tiffgeo/geotiff.h"

#include "../tiffgeo/geo_tiffp.h"
#include "../tiffgeo/geo_keyp.h"

#include "tif_memio.h"
#include "geotiff_buffer.h"

#define TIFFOpen XTIFFOpen
#define TIFFClose XTIFFClose
#define TIFFClientOpen XTIFFClientOpen 

typedef struct {
	int numcmpts;
	int cmpts[JAS_TIFF_MAX_COMPONENTS];
} tiff_enc_t;

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static int write_tiled_tiff(TIFF *tif, jas_image_t *image)
{
  uint32 columns, rows;
  uint32 x, y;
  uint32 height = 0; 
  uint32 width = 0;
  uint32 tileW, tileH;
  uint16 numcmpts=1;
	uint16 bitspersample = 1;
  uint32 cmptno;
  uchar *tile_buf;
  jas_matrix_t *bufs=NULL;

  if (tif == NULL) return -1;

  if( !TIFFIsTiled(tif) ) return -1;

  TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &columns);
  TIFFGetField(tif, TIFFTAG_TILELENGTH, &rows);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &numcmpts);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

  tile_buf = (uchar*) _TIFFmalloc(TIFFTileSize(tif));
  if (tile_buf == NULL) return -1;

  bufs = jas_matrix_create(rows, columns);
  if (!bufs) {
    jas_matrix_destroy(bufs);
    return -1;
  }

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
      
        // the tile size is now treated by libtiff guys the
        // way that the size stay on unchanged      
        if (width-x<columns) tileW = width-x;

        // first read the buffer
	      jas_image_readcmpt(image, cmptno, x, y, tileW, tileH, bufs);

        // read pixels 8 bits
        if (bitspersample == 8)
        for(yi=0; yi<tileH; yi++) {
          uchar *pix = ((uchar*)tile_buf) + (yi*columns);
          for(xi=0; xi<tileW; xi++)
            pix[xi] = (uchar) jas_matrix_get(bufs, yi, xi);
        } // for yi

        // read pixels 16 bits
        if (bitspersample == 16)
        for(yi=0; yi<tileH; yi++) {
          unsigned short *pix = ((unsigned short*)tile_buf) + (yi*columns);
          for(xi=0; xi<tileW; xi++)
            pix[xi] = (unsigned short) jas_matrix_get(bufs, yi, xi);
        } // for yi

        // read pixels 32 bits
        if (bitspersample > 16)
        for(yi=0; yi<tileH; yi++) {
          int *pix = ((int*)tile_buf) + (yi*columns);
          for(xi=0; xi<tileW; xi++)
            pix[xi] = (int) jas_matrix_get(bufs, yi, xi);
        } // for yi

        if (!TIFFWriteTile(tif, (tdata_t)tile_buf, x, y, 0, (tsample_t)cmptno)) break;
      } // for x
    } // for y
  } // for cmptno


  jas_matrix_destroy(bufs);
  _TIFFfree(tile_buf);
  return 0;
}

/******************************************************************************\
* Interface functions.
\******************************************************************************/

int tiff_encode(jas_image_t *image, jas_stream_t *out, char *optstr)
{
	int depth;
	uint cmptno;
	jas_clrspc_t clrspc;

  TIFF *tiff;
	uint32 height;
	uint32 width;
	uint32 rowsperstrip = (uint32) -1;
	uint16 bitspersample;
  uint16 bytespersample;
	uint16 photometric = PHOTOMETRIC_RGB;
	uint16 compression;
	uint32 x, y, c;
  uint32 tileW=0, tileH=0;
  bool isTiled=false;
  char *tstr = NULL;

  tiff_enc_t encbuf;
  tiff_enc_t *enc = &encbuf;
	uint numcmpts;

  // input params
  jas_stream_fileobj_t *fo = out->obj_;

	//if (optstr) { fprintf(stderr, "warning: ignoring TIFF encoder options\n"); }
  // accept option -O tiles
  if ( (optstr) && ( strstr(optstr, "tiles") != NULL ) ) isTiled=true;
  

  // accept option -O tilewidth=n
  if ( (optstr) && ( (tstr=strstr(optstr, "tilewidth")) != NULL ) ) {
    int size=0;
    sscanf(tstr, "tilewidth=%d", &size);
    tileW = size;
    isTiled=true;
  }

  // accept option -O tileheight=n
  if ( (optstr) && ( (tstr=strstr(optstr, "tileheight")) != NULL ) ) {
    int size=0;
    sscanf(tstr, "tileheight=%d", &size);
    tileH = size;
    isTiled=true;
  }


  //--------------------------------------------------------------------
  // Check n set params
  //--------------------------------------------------------------------

  clrspc = jas_image_clrspc(image);
	switch (jas_clrspc_fam(clrspc)) {
	case JAS_CLRSPC_FAM_RGB:
		if (clrspc != JAS_CLRSPC_SRGB)
			jas_eprintf("warning: inaccurate color\n");
		break;
	case JAS_CLRSPC_FAM_GRAY:
		if (clrspc != JAS_CLRSPC_SGRAY)
			jas_eprintf("warning: inaccurate color\n");
		break;
	default:
		jas_eprintf("error: TIFF format does not support color space\n");
		return -1;
		break;
	}


  // generic case
  enc->numcmpts = jas_image_numcmpts(image);
  for (cmptno=0;cmptno<enc->numcmpts;++cmptno) {
		enc->cmpts[cmptno] = cmptno;
  }

	switch (jas_clrspc_fam(clrspc)) {
	case JAS_CLRSPC_FAM_RGB:
    if (enc->numcmpts >= 3)
		if ((enc->cmpts[0] = jas_image_getcmptbytype(image,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R))) < 0 ||
		  (enc->cmpts[1] = jas_image_getcmptbytype(image,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G))) < 0 ||
		  (enc->cmpts[2] = jas_image_getcmptbytype(image,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B))) < 0) {
			jas_eprintf("error: missing color component\n");
			return -1;
		}
		break;
	case JAS_CLRSPC_FAM_GRAY:
		if ((enc->cmpts[0] = jas_image_getcmptbytype(image,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y))) < 0) {
			jas_eprintf("error: missing color component\n");
			return -1;
		}
		break;
	default:
		abort();
		break;
	}

  
  width  = jas_image_cmptwidth (image, enc->cmpts[0]);
	height = jas_image_cmptheight(image, enc->cmpts[0]);
	depth  = jas_image_cmptprec  (image, enc->cmpts[0]);

	// Check to ensure that the image to be saved can actually be represented
	//  using the TIFF format.
	for (cmptno = 0; cmptno < (uint) enc->numcmpts; ++cmptno) {
		if ((uint32)jas_image_cmptwidth(image, enc->cmpts[cmptno]) != width ||
		  (uint32)jas_image_cmptheight(image, enc->cmpts[cmptno]) != height ||
		  jas_image_cmptprec(image, enc->cmpts[cmptno]) != depth ||
		  jas_image_cmptsgnd(image, enc->cmpts[cmptno]) != false ||
		  jas_image_cmpttlx(image, enc->cmpts[cmptno]) != 0 ||
		  jas_image_cmpttly(image, enc->cmpts[cmptno]) != 0) {
			fprintf(stderr, "The TIFF format cannot be used to represent an image with this geometry.\n");
			return -1;
		}
	}


  //--------------------------------------------------------------------
  // Now create TIFF
  //--------------------------------------------------------------------

  bytespersample = (unsigned short) ceil((double)depth / 8.0);
  if (bytespersample>2) bytespersample=4;
 	numcmpts = enc->numcmpts;
	
  // check for bit_depth in jpeg image
  if ( (depth>=1) && (depth<=32) ) {
    bitspersample = bytespersample * 8;
  }
  else  {
    jas_eprintf("error: Cannon handle input pixel depth of %d bits\n", bitspersample);
    return -1;
  }

  // open without memory mapping.
  tiff = TIFFClientOpen(fo->pathname, "w",
                           (thandle_t) (out),
                           tiff_read,
                           tiff_write,
                           tiff_seek,
                           tiff_close,
                           tiff_size,
                           tiff_mmap,
                           tiff_unmap );


  if (!tiff) {
    jas_eprintf("error: cannot create TIFF image\n");
    return -1;
  }

  if (numcmpts>1) photometric = PHOTOMETRIC_RGB;
  else photometric = PHOTOMETRIC_MINISBLACK;

  // handle standard width/height/bpp stuff
	TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, numcmpts);
	TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bitspersample);
	TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric);
	TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tiff, TIFFTAG_SOFTWARE, "GeoJasPer <www.dimin.net>");

  if ( (numcmpts == 1) || (!isTiled) )
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  else
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);


  if (!isTiled)
  	TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, rowsperstrip));
  else
  {
    if ( (tileW==0) || (tileH==0) ) TIFFDefaultTileSize(tiff, &tileW, &tileH);    
    TIFFSetField(tiff, TIFFTAG_TILEWIDTH, tileW);
  	TIFFSetField(tiff, TIFFTAG_TILELENGTH, tileH);
  	//TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1);
  }

  // handle metrics
  //TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	//TIFFSetField(tiff, TIFFTAG_XRESOLUTION, 0.0254*30.0);
	//TIFFSetField(tiff, TIFFTAG_YRESOLUTION, 0.0254*30.0);

	// only one page
  TIFFSetField(tiff, TIFFTAG_SUBFILETYPE, 0);

  TIFFFlushData(tiff);

	// palettes (image colormaps are automatically scaled to 16-bits)
  /*
	if (photometric == PHOTOMETRIC_PALETTE) {
		uint16 *r, *g, *b;
		uint16 nColors = img.numColors();

		r = (uint16 *) _TIFFmalloc(sizeof(uint16) * 3 * nColors);
		g = r + nColors;
		b = g + nColors;

		for (int i = nColors - 1; i >= 0; i--) 
    {
			r[i] = (uint16) qRed(img.color(i));
			g[i] = (uint16) qGreen(img.color(i));
			b[i] = (uint16) qBlue(img.color(i));
		}

		TIFFSetField(tiff, TIFFTAG_COLORMAP, r, g, b);

		_TIFFfree(r);
	}
  
  TIFFFlushData(tiff);
  */

  // ----------------------------------------------------------------------
  // Now copy Geo TIFF information if any
  // ----------------------------------------------------------------------
  if ( (image->aux_buf.size > 0) && (image->aux_buf.buf != NULL) )
  {
    GTIFFromMemBuf( image->aux_buf.buf, image->aux_buf.size, tiff );
  }

  TIFFFlushData(tiff);
  // ----------------------------------------------------------------------

  // compression
  compression = COMPRESSION_NONE;
	TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
  TIFFFlushData(tiff);

  
  //--------------------------------------------------------------------
  // pass through data
  //-------------------------------------------------------------------- 
  if (isTiled)
    if (write_tiled_tiff(tiff, image) != 0) return -1;

  if (!isTiled)
  {
    jas_matrix_t *bufs[JAS_TIFF_MAX_COMPONENTS];
    uchar *buffer;
    uint i;

    for (i = 0; i < numcmpts; ++i) bufs[i] = 0;

	  // Create temporary matrices to hold component data
	  for (i = 0; i < numcmpts; ++i) {
	  	if (!(bufs[i] = jas_matrix_create(1, width))) { // Destroy the temporary matrices
	      for (i = 0; i < numcmpts; ++i) if (bufs[i]) jas_matrix_destroy(bufs[i]);
		  	return -1;
      }
    }
    
 
    buffer = (uchar *) _TIFFmalloc(width*numcmpts*bytespersample);

    if (!buffer) { // Destroy the temporary matrices
	    for (i=0; i<numcmpts; ++i) if (bufs[i]) jas_matrix_destroy(bufs[i]);
		  return -1;
    }

  	// Put the image data
    //
    for (y = 0; y < height; y++) 
    {
      // read image line
		  for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
			  if ( jas_image_readcmpt(image, cmptno, 0, y, width, 1, bufs[enc->cmpts[cmptno]]) ) {
	        for (i = 0; i < numcmpts; ++i) if (bufs[i]) jas_matrix_destroy(bufs[i]);
		  	  return -1;
			  }
		  }

      
      if (bitspersample == 8) {
        // let this be optimized by compiler
        for (x=0; x<width; ++x) 
          for (c=0; c<numcmpts; ++c) 
            buffer[x*numcmpts+c] = (uchar) jas_matrix_getv(bufs[c], x);

        // write the scanline to disc
				TIFFWriteScanline(tiff, buffer, y, 0);
        TIFFFlushData(tiff);
      }

      if (bitspersample == 16) {
        unsigned short *buf16 = (unsigned short *) buffer;
        
        // let this be optimized by compiler
        for (x=0; x<width; ++x) 
          for (c=0; c<numcmpts; ++c) 
            buf16[x*numcmpts+c] = (unsigned short) jas_matrix_getv(bufs[c], x);

        // write the scanline to disc
				TIFFWriteScanline(tiff, buf16, y, 0);
        TIFFFlushData(tiff);
      }

      if (bitspersample > 16) {
        int *buf32 = (int *) buffer;
        
        // let this be optimized by compiler
        for (x=0; x<width; ++x) 
          for (c=0; c<numcmpts; ++c) 
            buf32[x*numcmpts+c] = (int) jas_matrix_getv(bufs[c], x);

        // write the scanline to disc
				TIFFWriteScanline(tiff, buf32, y, 0);
        TIFFFlushData(tiff);
      }
 
    }  // for y
		
    _TIFFfree(buffer);
  } // pass trough data block

  
  fprintf(stdout, "TIFF file encoded!\n");


  TIFFFlushData(tiff);
  TIFFFlush(tiff);

  //TIFFClose(tiff);
  // do not close it overwise jasper will hang...

	return 0;
}


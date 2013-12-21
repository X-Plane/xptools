/*****************************************************************************
 GeoTIFF memory buffer
 Copyright (c) 2001-2002 by Dmitry V. Fedorov <www.dimin.net>

 DEFS

 Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 History:
   11/07/2003 15:26 - First creation
   04/20/2005 17:19 - Print geotiff

 Ver : 3
*****************************************************************************/


#ifndef GEOTIFF_BUFFER_H
#define GEOTIFF_BUFFER_H

#include "../tiffgeo/xtiffio.h" 

#define DIM_TFW_XVX 0
#define DIM_TFW_XVY 1
#define DIM_TFW_YVX 2
#define DIM_TFW_YVY 3
#define DIM_TFW_REFX 4
#define DIM_TFW_REFY 5 

int isGeoTiff (TIFF *tif);

int MemBufFromGTIF ( TIFF *in, long *pSize, unsigned char **pBuffer );

int GTIFFromMemBuf( unsigned char *pBuffer, long pSize, TIFF *out );

// im_h, im_w are image real width and height needed for full print since the 
// degenerated tiff doesn't contain any image
void GTIFPrintFullA(TIFF *tif, long im_w, long im_h);
void GTIFPrintFull(TIFF *tif);
int printGTIFFromMemBufA( unsigned char *pBuffer, long pSize, long im_w, long im_h );
int printGTIFFromMemBuf( unsigned char *pBuffer, long pSize );

//----------------------------------------------------------------------------
// World File (TFW) workaround
//----------------------------------------------------------------------------
int readWorldFile(const char *fileName, long *pSize, unsigned char **pBuffer);
int writeWorldFile(unsigned char *pBuffer, long pSize, const char *fileName);
//----------------------------------------------------------------------------

#endif

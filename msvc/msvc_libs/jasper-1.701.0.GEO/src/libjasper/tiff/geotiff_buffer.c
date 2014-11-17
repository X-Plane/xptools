/*****************************************************************************
 GeoTIFF memory buffer
 Copyright (c) 2001-2002 by Dmitry V. Fedorov <www.dimin.net>

 IMPLEMENTATION

 Programmer: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

 History:
   11/07/2003 15:26 - First creation
   04/20/2005 17:19 - Print geotiff keys and tags

 Ver : 5
*****************************************************************************/

// geotifflib
#include "../tiffgeo/geo_tiffp.h"
#include "../tiffgeo/geo_keyp.h"
#include "../tiffgeo/xtiffio.h"
#include "../tiffgeo/geotiff.h"
#include "../tiffgeo/geo_normalize.h"
#include "../tiffgeo/geovalues.h"

#include "tif_memio.h"
#include "geotiff_buffer.h"

static void GTIFPrintCorners( GTIF *, GTIFDefn *, FILE *, int, int, int, int ); 
static void CopyGeoTIFF2(TIFF *in, TIFF *out); 


int isGeoTiff (TIFF *tif)
{
  double *d_list = NULL;
  int16   d_list_count;

  if (!TIFFGetField(tif, GTIFF_TIEPOINTS, &d_list_count, &d_list) &&
      !TIFFGetField(tif, GTIFF_PIXELSCALE, &d_list_count, &d_list) &&
      !TIFFGetField(tif, GTIFF_TRANSMATRIX, &d_list_count, &d_list) )
  return FALSE;
  else 
  return TRUE;
}

//************************************************************************
//*                         MemBufFromGTIF                               *
//************************************************************************

int GTIFFromMemBuf( unsigned char *pBuffer, long pSize, TIFF *out )
{
  MemIOBuf sIOBuf;
  TIFF *hTIFF;

  // --------------------------------------------------------------------
  // Initialize access to the memory geotiff structure.               
  // --------------------------------------------------------------------
  MemIO_InitBuf( &sIOBuf, pSize, pBuffer );
    
  hTIFF = XTIFFClientOpen( "membuf", "r", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return -1;
  
  CopyGeoTIFF2(hTIFF, out);

  // -------------------------------------------------------------------- 
  // Cleanup.                                                        
  // -------------------------------------------------------------------- 
  XTIFFClose( hTIFF );

  MemIO_DeinitBuf( &sIOBuf );

  return 0;
}

//************************************************************************
//*                         MemBufFromGTIF                               *
//************************************************************************
int MemBufFromGTIF ( TIFF *in, long *pSize, unsigned char **pBuffer )
{
  MemIOBuf sIOBuf;
  TIFF *hTIFF;
  unsigned char bySmallImage = 0;

  // --------------------------------------------------------------------
  //      Initialize access to the memory geotiff structure.             
  // --------------------------------------------------------------------
  MemIO_InitBuf( &sIOBuf, 0, NULL );
    
  hTIFF = XTIFFClientOpen( "membuf", "w", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return -1;


  // --------------------------------------------------------------------
  // Write some minimal set of image parameters.                    
  // --------------------------------------------------------------------
  TIFFSetField( hTIFF, TIFFTAG_IMAGEWIDTH, 1 );
  TIFFSetField( hTIFF, TIFFTAG_IMAGELENGTH, 1 );
  TIFFSetField( hTIFF, TIFFTAG_BITSPERSAMPLE, 8 );
  TIFFSetField( hTIFF, TIFFTAG_SAMPLESPERPIXEL, 1 );
  TIFFSetField( hTIFF, TIFFTAG_ROWSPERSTRIP, 1 );
  TIFFSetField( hTIFF, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  TIFFSetField( hTIFF, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
    
  // -------------------------------------------------------------------- 
  // Copy GEO params
  // -------------------------------------------------------------------- 
  if (isGeoTiff(in) == TRUE)
  {
    CopyGeoTIFF2(in, hTIFF);

    // --------------------------------------------------------------------
    // Cleanup and return the created memory buffer.                  
    // --------------------------------------------------------------------
    TIFFWriteEncodedStrip( hTIFF, 0, (char *) &bySmallImage, 1 );
    TIFFWriteCheck( hTIFF, TIFFIsTiled(hTIFF), "MemBufFromGTIF");
    TIFFWriteDirectory( hTIFF );

    XTIFFClose( hTIFF );

    *pSize = sIOBuf.size;
    //*pBuffer = (unsigned char *) CPLMalloc(*pSize);
    if (*pBuffer != NULL) free(*pBuffer);
    *pBuffer = (unsigned char *) malloc(*pSize);
    memcpy( *pBuffer, sIOBuf.data, *pSize );
  }
  else
  {
    XTIFFClose( hTIFF );
    *pBuffer = NULL;
    *pSize = 0;
  }
  
  MemIO_DeinitBuf( &sIOBuf );
  
  return 0;
}

//************************************************************************
//*                      printGTIFFromMemBuf                             *
//************************************************************************

void GTIFPrintFull(TIFF *tif)
{
  int xsize, ysize;
  TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &xsize );
  TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &ysize );
  GTIFPrintFullA(tif, xsize, ysize);
}

void GTIFPrintFullA(TIFF *tif, long im_w, long im_h)
{
  GTIF *gtif;
  GTIFDefn defn;
  int inv_flag = 0, dec_flag = 0;

  gtif = (GTIF*) 0; // GeoKey-level descriptor 
  gtif = GTIFNew(tif);
  if (gtif)
  {
    GTIFPrint( gtif, 0, 0 );

    if( GTIFGetDefn( gtif, &defn ) )
    {
      int xsize, ysize;

      printf( "\n" );
      GTIFPrintDefn( &defn, stdout );

      //TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &xsize );
      //TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &ysize );
      xsize = im_w;
      ysize = im_h;
      GTIFPrintCorners( gtif, &defn, stdout, xsize, ysize, inv_flag, dec_flag );
    } 
    
    GTIFFree(gtif);
  }
}

int printGTIFFromMemBufA( unsigned char *pBuffer, long pSize, long im_w, long im_h )
{
  MemIOBuf sIOBuf;
  TIFF *hTIFF;

  // --------------------------------------------------------------------
  // Initialize access to the memory geotiff structure.               
  // --------------------------------------------------------------------
  MemIO_InitBuf( &sIOBuf, pSize, pBuffer );
    
  hTIFF = XTIFFClientOpen( "membuf", "r", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return -1;

  GTIFPrintFullA( hTIFF, im_w, im_h );

  // -------------------------------------------------------------------- 
  // Cleanup.                                                        
  // -------------------------------------------------------------------- 
  XTIFFClose( hTIFF );

  MemIO_DeinitBuf( &sIOBuf );

  return 0;
}

int printGTIFFromMemBuf( unsigned char *pBuffer, long pSize )
{
  MemIOBuf sIOBuf;
  TIFF *hTIFF;

  // --------------------------------------------------------------------
  // Initialize access to the memory geotiff structure.               
  // --------------------------------------------------------------------
  MemIO_InitBuf( &sIOBuf, pSize, pBuffer );
    
  hTIFF = XTIFFClientOpen( "membuf", "r", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return -1;

  GTIFPrintFull( hTIFF );

  // -------------------------------------------------------------------- 
  // Cleanup.                                                        
  // -------------------------------------------------------------------- 
  XTIFFClose( hTIFF );

  MemIO_DeinitBuf( &sIOBuf );

  return 0;
}


//****************************************************************************
// WorldFile
//****************************************************************************

int readWorldFile(const char *fileName, long *pSize, unsigned char **pBuffer)
{
  float tfwVars[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
  FILE *stream;
  int i=0;

  MemIOBuf sIOBuf;
  TIFF *hTIFF;
  unsigned char bySmallImage = 0;
  
  stream = fopen(fileName, "rt");
  while( !feof( stream ) )
  {
    if (i > 5) break;
    fscanf(stream, "%f\n", &tfwVars[i]);
    //tfwVars[i] = fv;
    i++;
  }
  fclose(stream);

  // if pixel scale parameters are undefined, something's wront
  if ( (tfwVars[0] == 0.0) || (tfwVars[3] == 0.0) ) return 1;
  // if rotation parameters are defined, we don't read this yet
  //if ( (tfwVars[1] != 0.0) || (tfwVars[2] != 0.0) ) return 2;

  //------------------------------------------------------------------------
  // now create geotiff buffer
  //------------------------------------------------------------------------
  MemIO_InitBuf( &sIOBuf, 0, NULL );
    
  hTIFF = XTIFFClientOpen( "membuf", "w", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return FALSE;


  // --------------------------------------------------------------------
  // Write some minimal set of image parameters.                    
  // --------------------------------------------------------------------
  TIFFSetField( hTIFF, TIFFTAG_IMAGEWIDTH, 1 );
  TIFFSetField( hTIFF, TIFFTAG_IMAGELENGTH, 1 );
  TIFFSetField( hTIFF, TIFFTAG_BITSPERSAMPLE, 8 );
  TIFFSetField( hTIFF, TIFFTAG_SAMPLESPERPIXEL, 1 );
  TIFFSetField( hTIFF, TIFFTAG_ROWSPERSTRIP, 1 );
  TIFFSetField( hTIFF, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  TIFFSetField( hTIFF, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
    
  // -------------------------------------------------------------------- 
  // Copy GEO params
  // -------------------------------------------------------------------- 
  {
    double tie_points[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double pix_res[3] = {0.0, 0.0, 0.0};

    // set pixel resolution
    pix_res[0] = tfwVars[DIM_TFW_XVX];
    pix_res[1] = -1.0 * tfwVars[DIM_TFW_YVY];
    TIFFSetField(hTIFF, GTIFF_PIXELSCALE, 3, pix_res);

    // correct for center of pixel vs. top left of pixel
    tie_points[3] = tfwVars[DIM_TFW_REFX] - (pix_res[0] / 2.0);
    tie_points[4] = tfwVars[DIM_TFW_REFY] + (pix_res[1] / 2.0);
    TIFFSetField(hTIFF, GTIFF_TIEPOINTS, 6, tie_points);
  }
  // --------------------------------------------------------------------
  // Cleanup and return the created memory buffer.                  
  // --------------------------------------------------------------------
  TIFFWriteEncodedStrip( hTIFF, 0, (char *) &bySmallImage, 1 );
  TIFFWriteCheck( hTIFF, TIFFIsTiled(hTIFF), "MemBufFromGTIF");
  TIFFWriteDirectory( hTIFF );

  XTIFFClose( hTIFF );

  if (*pBuffer != NULL) free(*pBuffer);
  *pSize = sIOBuf.size;
  *pBuffer = (unsigned char *) malloc(*pSize);
  memcpy( *pBuffer, sIOBuf.data, *pSize );

  MemIO_DeinitBuf( &sIOBuf );

  if ( (tfwVars[1] != 0.0) || (tfwVars[2] != 0.0) ) return 2;
  return 0;
}

int writeWorldFile(unsigned char *pBuffer, long pSize, const char *fileName)
{
  float tfwVars[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
  FILE *stream;
  int i=0; 
  
  MemIOBuf sIOBuf;
  TIFF *hTIFF;

  double *d_list = NULL;
  int16   d_list_count;

  if (pBuffer == NULL) return 1;
  if (pSize == 0) return 1;

  MemIO_InitBuf( &sIOBuf, pSize, pBuffer );
    
  hTIFF = XTIFFClientOpen( "membuf", "r", (thandle_t) &sIOBuf, 
                           MemIO_ReadProc, MemIO_WriteProc, MemIO_SeekProc, 
                           MemIO_CloseProc, MemIO_SizeProc, 
                           MemIO_MapProc, MemIO_UnmapProc );

  if( hTIFF == NULL ) return 1;
  
  if (TIFFGetField(hTIFF, GTIFF_PIXELSCALE, &d_list_count, &d_list))
  {
    if (d_list_count >= 1) tfwVars[DIM_TFW_XVX] = (float) d_list[0];
    if (d_list_count >= 2) tfwVars[DIM_TFW_YVY] = (float) (-1.0 * d_list[1]);
  }
  
  if (TIFFGetField(hTIFF, GTIFF_TIEPOINTS, &d_list_count, &d_list))
  {
    // here we should add the value of half resolution
    if (d_list_count >= 1) tfwVars[DIM_TFW_REFX] = (float) (d_list[3] + (tfwVars[DIM_TFW_XVX] / 2.0));
    // here we should subtract the value of half resolution
    if (d_list_count >= 2) tfwVars[DIM_TFW_REFY] = (float) (d_list[4] + (tfwVars[DIM_TFW_YVY] / 2.0));
  }

  XTIFFClose( hTIFF );
  MemIO_DeinitBuf( &sIOBuf );
 
  // --------------------------------------------------------------------
  // Now write the world file                  
  // --------------------------------------------------------------------  
  stream = fopen(fileName, "wt");
  for (i=0; i<6; i++)
  {
    fprintf(stream, "%f\n", tfwVars[i]);
  }
  fclose(stream);
  
  return 0;
}

//****************************************************************************
// Misc funcs from libgeotiff
//****************************************************************************

static void CopyGeoTIFF2(TIFF *in, TIFF *out)
{
    GTIF *gtif=(GTIF*)0; // GeoKey-level descriptor 
    double *d_list = NULL;
    int16   d_list_count;

    // read definition from source file
    gtif = GTIFNew(in);
    if (!gtif)
        return;

    // TAG 33922
    if (TIFFGetField(in, GTIFF_TIEPOINTS, &d_list_count, &d_list))
        TIFFSetField(out, GTIFF_TIEPOINTS, d_list_count, d_list);
    
    // TAG 33550
    if (TIFFGetField(in, GTIFF_PIXELSCALE, &d_list_count, &d_list))
        TIFFSetField(out, GTIFF_PIXELSCALE, d_list_count, d_list);
    
    // 34264
    if (TIFFGetField(in, GTIFF_TRANSMATRIX, &d_list_count, &d_list))
        TIFFSetField(out, GTIFF_TRANSMATRIX, d_list_count, d_list);
            
    // Here we violate the GTIF abstraction to retarget on another file.
    //   We should just have a function for copying tags from one GTIF object
    //   to another
    gtif->gt_tif = out;
    gtif->gt_flags |= FLAG_FILE_MODIFIED;

    // Install keys and tags
    GTIFWriteKeys(gtif);
    GTIFFree(gtif);
    return;
}; 

const char *GTIFDecToDDec( double dfAngle, const char * pszAxis, int nPrecision )
{
    char        szFormat[30];
    static char szBuffer[50];
    const char  *pszHemisphere = NULL;

    if( EQUAL(pszAxis,"Long") && dfAngle < 0.0 )
        pszHemisphere = "W";
    else if( EQUAL(pszAxis,"Long") )
        pszHemisphere = "E";
    else if( dfAngle < 0.0 )
        pszHemisphere = "S";
    else
        pszHemisphere = "N";

    sprintf( szFormat, "%%%d.%df%s",
             nPrecision+5, nPrecision, pszHemisphere );
    sprintf( szBuffer, szFormat, dfAngle );

    return( szBuffer );
} 


// Report the file(s) corner coordinates in projected coordinates, and
// if possible lat/long.
static int GTIFReportACorner( GTIF *gtif, GTIFDefn *defn, FILE * fp_out,
                              const char * corner_name,
                              double x, double y, int inv_flag, int dec_flag )
{
    double	x_saved, y_saved;

    /* Try to transform the coordinate into PCS space */
    if( !GTIFImageToPCS( gtif, &x, &y ) )
        return FALSE;
    
    x_saved = x;
    y_saved = y;

    fprintf( fp_out, "%-13s ", corner_name );

    if( defn->Model == ModelTypeGeographic )
    {
	if (dec_flag) 
	{
	    fprintf( fp_out, "(%s,", GTIFDecToDDec( x, "Long", 7 ) );
	    fprintf( fp_out, "%s)\n", GTIFDecToDDec( y, "Lat", 7 ) );
	} 
	else 
	{
	    fprintf( fp_out, "(%s,", GTIFDecToDMS( x, "Long", 2 ) );
	    fprintf( fp_out, "%s)\n", GTIFDecToDMS( y, "Lat", 2 ) );
	}
    }
    else
    {
        fprintf( fp_out, "(%12.3f,%12.3f)", x, y );

        if( GTIFProj4ToLatLong( defn, 1, &x, &y ) )
        {
	    if (dec_flag) 
	    {
		fprintf( fp_out, "  (%s,", GTIFDecToDDec( x, "Long", 7 ) );
		fprintf( fp_out, "%s)", GTIFDecToDDec( y, "Lat", 7 ) );
	    } 
	    else 
	    {
		fprintf( fp_out, "  (%s,", GTIFDecToDMS( x, "Long", 2 ) );
		fprintf( fp_out, "%s)", GTIFDecToDMS( y, "Lat", 2 ) );
	    }
        }

        fprintf( fp_out, "\n" );
    }

    if( inv_flag && GTIFPCSToImage( gtif, &x_saved, &y_saved ) )
    {
        fprintf( fp_out, "      inverse (%11.3f,%11.3f)\n", x_saved, y_saved );
    }
    
    return TRUE;
} 

static void GTIFPrintCorners( GTIF *gtif, GTIFDefn *defn, FILE * fp_out,
                              int xsize, int ysize, int inv_flag, int dec_flag )
                             
{
    printf( "\nCorner Coordinates:\n" );
    if( !GTIFReportACorner( gtif, defn, fp_out,
                            "Upper Left", 0.0, 0.0, inv_flag, dec_flag ) )
    {
        printf( " ... unable to transform points between pixel/line and PCS space\n" );
        return;
    }
  
    GTIFReportACorner( gtif, defn, fp_out, "Lower Left", 0.0, ysize, 
                       inv_flag, dec_flag );
    GTIFReportACorner( gtif, defn, fp_out, "Upper Right", xsize, 0.0,
                       inv_flag, dec_flag );
    GTIFReportACorner( gtif, defn, fp_out, "Lower Right", xsize, ysize,
                       inv_flag, dec_flag );
    GTIFReportACorner( gtif, defn, fp_out, "Center", xsize/2.0, ysize/2.0,
                       inv_flag, dec_flag );
}







/*****************************************************************************
 GeoJP2 support for Jasper 

 Extracted form jpeg2000dataset.cpp Revision 1.18
 GDAL 1.1.9
 Jasper 1.700.2

*****************************************************************************/


#ifndef JASPER_DEFS_H
#define JASPER_DEFS_H

#include <jasper/jasper.h>

/*
#ifdef HAVE_JASPER_UUID
CPLErr CPL_DLL GTIFMemBufFromWkt( const char *pszWKT, 
                                  const double *padfGeoTransform,
                                  int nGCPCount, const GDAL_GCP *pasGCPList,
                                  int *pnSize, unsigned char **ppabyBuffer );
CPLErr CPL_DLL GTIFWktFromMemBuf( int nSize, unsigned char *pabyBuffer, 
                          char **ppszWKT, double *padfGeoTransform,
                          int *pnGCPCount, GDAL_GCP **ppasGCPList );
#endif
*/


// XXX: Part of code below extracted from the JasPer internal headers and
// must be in sync with JasPer version (this one works with JasPer 1.700.2)
#define JP2_FTYP_MAXCOMPATCODES 32
#define JP2_BOX_IHDR 0x69686472 // Image Header
#define JP2_BOX_BPCC 0x62706363 // Bits Per Component
#define	JP2_BOX_PCLR 0x70636c72 // Palette
#define JP2_BOX_UUID 0x75756964 // UUID

//extern "C" {
typedef struct {
        uint_fast32_t magic;
} jp2_jp_t;

typedef struct {
        uint_fast32_t majver;
        uint_fast32_t minver;
        uint_fast32_t numcompatcodes;
        uint_fast32_t compatcodes[JP2_FTYP_MAXCOMPATCODES];
} jp2_ftyp_t;
typedef struct {
        uint_fast32_t width;
        uint_fast32_t height;
        uint_fast16_t numcmpts;
        uint_fast8_t bpc;
        uint_fast8_t comptype;
        uint_fast8_t csunk;
        uint_fast8_t ipr;
} jp2_ihdr_t;
typedef struct {
        uint_fast16_t numcmpts;
        uint_fast8_t *bpcs;
} jp2_bpcc_t;
typedef struct {
        uint_fast8_t method;
        uint_fast8_t pri;
        uint_fast8_t approx;
        uint_fast32_t csid;
        uint_fast8_t *iccp;
        int iccplen;
} jp2_colr_t;
typedef struct {
        uint_fast16_t numlutents;
        uint_fast8_t numchans;
        int_fast32_t *lutdata;
        uint_fast8_t *bpc;
} jp2_pclr_t;
typedef struct {
        uint_fast16_t channo;
        uint_fast16_t type;
        uint_fast16_t assoc;
} jp2_cdefchan_t;
typedef struct {
        uint_fast16_t numchans;
        jp2_cdefchan_t *ents;
} jp2_cdef_t;
typedef struct {
        uint_fast16_t cmptno;
        uint_fast8_t map;
        uint_fast8_t pcol;
} jp2_cmapent_t;

typedef struct {
        uint_fast16_t numchans;
        jp2_cmapent_t *ents;
} jp2_cmap_t;

#ifdef HAVE_JASPER_UUID
typedef struct {
        uint_fast32_t data_len;
        uint_fast8_t uuid[16];
        uint_fast8_t *data;
} jp2_uuid_t;
#endif

struct jp2_boxops_s;
typedef struct {

        struct jp2_boxops_s *ops;
        struct jp2_boxinfo_s *info;

        uint_fast32_t type;
        uint_fast32_t len;
#ifdef HAVE_JASPER_UUID
        uint_fast32_t data_len;
#endif

        union {
                jp2_jp_t jp;
                jp2_ftyp_t ftyp;
                jp2_ihdr_t ihdr;
                jp2_bpcc_t bpcc;
                jp2_colr_t colr;
                jp2_pclr_t pclr;
                jp2_cdef_t cdef;
                jp2_cmap_t cmap;
#ifdef HAVE_JASPER_UUID
                jp2_uuid_t uuid;
#endif
        } data;

} jp2_box_t;
typedef struct jp2_boxops_s {
        void (*init)(jp2_box_t *box);
        void (*destroy)(jp2_box_t *box);
        int (*getdata)(jp2_box_t *box, jas_stream_t *in);
        int (*putdata)(jp2_box_t *box, jas_stream_t *out);
        void (*dumpdata)(jp2_box_t *box, FILE *out);
} jp2_boxops_t;

extern jp2_box_t *jp2_box_create(int type);
extern void jp2_box_destroy(jp2_box_t *box);
extern jp2_box_t *jp2_box_get(jas_stream_t *in);
extern int jp2_box_put(jp2_box_t *box, jas_stream_t *out);
#ifdef HAVE_JASPER_UUID
int jp2_encode_uuid(jas_image_t *image, jas_stream_t *out,
                    char *optstr, jp2_box_t *uuid);
#endif
//}
// XXX: End of JasPer header.

#ifdef HAVE_JASPER_UUID

// Magick sequence for GeoJP2 box
static unsigned char msi_uuid2[16] =
        {0xb1,0x4b,0xf8,0xbd,0x08,0x3d,0x4b,0x43,
         0xa5,0xae,0x8c,0xd7,0xd5,0xa6,0xce,0x03}; 
#endif

#endif
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
#include "DEMIO.h"
#include "EndianUtils.h"
#include "SimpleIO.h"
#include "MemFileUtils.h"
#include "ConfigSystem.h"
#include "GISUtils.h"
#include "EnumSystem.h"
#include "PlatformUtils.h"
#include "DEMTables.h"

#if IBM
#define AVOID_WIN32_FILEIO
#endif

#include <tiffio.h>
#include <xtiffio.h>
const double one_256 = 1.0 / 256.0;

static	double	ReadReal48(const unsigned char * p)
{
	// Read a Turbopascal REAL48 - always little endian since that's what TP ran on.
	// Format is: 8 bit exponent (+129), 39 bits of mantissa, MSB is sign.

	// Special case - 0 exponent means 0.	
	if (p[0] == 0) return 0.0;
	
	bool sign_negative = p[5] & 0x80;
	int	expv = (int) p[0] - 0x81;
	double	exponent = 1.0;
	if (expv > 0) exponent = 1 << expv;
	else if (expv < 0) exponent = 1.0 / double ( 1 << (-expv));
	
	double m = 1.0 + 2.0 * ((((p[1]*one_256+p[2])*one_256+p[3])*one_256+p[4])*one_256+(p[5] & 0x7f))*one_256;
	return sign_negative ? ( -m * exponent) : (m * exponent);
}
	
void	WriteDEM(DEMGeo& inMap, IOWriter * inWriter)
{
	inWriter->WriteInt(inMap.mWidth);
	inWriter->WriteInt(inMap.mHeight);
	inWriter->WriteDouble(inMap.mWest);
	inWriter->WriteDouble(inMap.mSouth);
	inWriter->WriteDouble(inMap.mEast);
	inWriter->WriteDouble(inMap.mNorth);
	
	EndianSwapArray(platform_Native, platform_LittleEndian, inMap.mWidth *inMap.mHeight, sizeof(float), inMap.mData);
	inWriter->WriteBulk((const char *) inMap.mData, inMap.mWidth * inMap.mHeight * sizeof(float), false);
	EndianSwapArray(platform_LittleEndian, platform_Native, inMap.mWidth *inMap.mHeight, sizeof(float), inMap.mData);
}

void	ReadDEM (		DEMGeo& inMap, IOReader * inReader)
{
	int	hpix, vpix;
	inReader->ReadInt(hpix);
	inReader->ReadInt(vpix);
	
	inMap.resize(hpix, vpix);

	inReader->ReadDouble(inMap.mWest);
	inReader->ReadDouble(inMap.mSouth);
	inReader->ReadDouble(inMap.mEast);
	inReader->ReadDouble(inMap.mNorth);
	
	if (inMap.mData)
	{
		inReader->ReadBulk((char *) inMap.mData, inMap.mWidth * inMap.mHeight * sizeof(float), false);
		EndianSwapArray(platform_LittleEndian, platform_Native, inMap.mWidth * inMap.mHeight, sizeof(float), inMap.mData);
	}	
}

void	RemapEnumDEM(	DEMGeo& ioMap, const TokenConversionMap& inMap)
{
	for (int x = 0; x < ioMap.mWidth; ++x)
	for (int y = 0 ; y < ioMap.mHeight; ++y)
	{
		int v = ioMap(x,y);
		if (v >= 0 && v < inMap.size())
			ioMap(x,y) = inMap[v];
	}
}

#pragma mark -


// RAW HEIGHT FILE: N34W072.HGT
// These files contian big-endian shorts with -32768 as NO_DATA
bool	ReadRawHGT(DEMGeo& inMap, const char * inFileName)
{
	int	lat, lon;
	char ns, ew;
	string	fname(inFileName);
	string::size_type p = fname.find_last_of(":\\/");
	if (p != fname.npos) fname = fname.substr(p+1);
	if (sscanf(fname.c_str(), "%c%d%c%d", &ns, &lat, &ew, &lon) == 4)
	{
		if (ns == 'S' || ns == 's' || ns == '-') lat = -lat;
		if (ew == 'W' || ew == 'w' || ew == '-') lon = -lon;
		inMap.mWest = lon;
		inMap.mEast = lon + 1;
		inMap.mSouth = lat;
		inMap.mNorth = lat + 1;
	}
	
	MFMemFile *	fi = MemFile_Open(inFileName);
	if (!fi) return false;
	
	MemFileReader	reader(MemFile_GetBegin(fi), MemFile_GetEnd(fi), platform_BigEndian);
	
	int len = MemFile_GetEnd(fi) - MemFile_GetBegin(fi);
	long words = len / sizeof(short);
	long dim = sqrt(words);
	
	inMap.resize(dim, dim);
	if (inMap.mData)
	{
		for (int y = dim-1; y >= 0; --y)
		for (int x = 0; x < dim; ++x)
		{
			short	v;
			reader.ReadShort(v);
			inMap.mData[x + y * dim] = v;
		}
	}
	
	MemFile_Close(fi);	
	return true;
}

bool	WriteRawHGT(const DEMGeo& dem, const char * inFileName)
{
	string	sname(inFileName);
	string::size_type p = sname.rfind(DIR_CHAR);
	if (p != sname.npos)
		sname.erase(0, p+1);
	sname.erase(sname.length()-4);
	ZipFileWriter	writer2(inFileName, sname.c_str(), platform_BigEndian);

	WriterBuffer	writer(&writer2, platform_BigEndian);

	for (int y = dem.mHeight-1; y >= 0; --y)
	for (int x = 0; x < dem.mWidth; ++x)
	{
		short	v = dem.mData[x + y * dem.mWidth];
		writer.WriteShort(v);
	}
	
	return true;		
}



// Austin's DEM: +40-018.DEM
// These files contain a 5 byte header (who knew) and then
// 32-bit big endian floats.  Also IMPORTANT: they are stored
// in COLUMN consecutive form, not ROW consecutive form!!
bool	ReadFloatHGT(DEMGeo& inMap, const char * inFileName)
{
	int	lat, lon;
	char ns, ew;
	string	fname(inFileName);
	string::size_type p = fname.find_last_of(":\\/");
	if (p != fname.npos) fname = fname.substr(p+1);
	if (sscanf(fname.c_str(), "%c%d%c%d", &ns, &lat, &ew, &lon) == 4)
	{
		if (ns == '-') lat = -lat;
		if (ew == '-') lon = -lon;
		inMap.mWest = lon;
		inMap.mEast = lon + 1;
		inMap.mSouth = lat;
		inMap.mNorth = lat + 1;
	}

	MFMemFile *	fi = MemFile_Open(inFileName);
	if (!fi) return false;
	
	MemFileReader	reader(MemFile_GetBegin(fi), MemFile_GetEnd(fi), platform_BigEndian);
	
	int len = MemFile_GetEnd(fi) - MemFile_GetBegin(fi);
	long words = len / sizeof(float);
	long dim = sqrt(words);
	
	inMap.resize(dim, dim);
	int dummy1;
	char dummy2;
	reader.ReadInt(dummy1);
	reader.ReadBulk(&dummy2,1,false);
	{
		if (inMap.mData)
		{
			for (int x = 0; x < dim; ++x)
			for (int y = 0; y < dim; ++y)
			{
				float	v;
				reader.ReadFloat(v);
				inMap.mData[x + y * dim] = v;
			}
		}
	}
		
	MemFile_Close(fi);	
	return true;
}

bool	ReadShortOz(DEMGeo& inMap, const char * inFileName)
{
	int	lat, lon;
	char ns, ew;
	string	fname(inFileName);
	string::size_type p = fname.find_last_of(":\\/");
	if (p != fname.npos) fname = fname.substr(p+1);
	if (sscanf(fname.c_str(), "%c%d%c%d", &ns, &lat, &ew, &lon) == 4)
	{
		if (ns == '-') lat = -lat;
		if (ew == '-') lon = -lon;
		inMap.mWest = lon;
		inMap.mEast = lon + 1;
		inMap.mSouth = lat;
		inMap.mNorth = lat + 1;
	}

	MFMemFile *	fi = MemFile_Open(inFileName);
	if (!fi) return false;
	
	MemFileReader	reader(MemFile_GetBegin(fi), MemFile_GetEnd(fi), platform_BigEndian);
	
	int len = MemFile_GetEnd(fi) - MemFile_GetBegin(fi);
	long words = len / sizeof(short);
	long dim = sqrt(words);
	
	inMap.resize(dim, dim);
	{
		if (inMap.mData)
		{
			for (int y = 0; y < dim; ++y)
			for (int x = 0; x < dim; ++x)
			{
				short	s;
				reader.ReadShort(s);
				inMap.mData[x + y * dim] = s;
			}
		}
	}
		
	MemFile_Close(fi);	
	return true;
}

bool	WriteFloatHGT(const DEMGeo& inMap, const char * inFileName)
{
	FILE * fi = fopen(inFileName, "wb");
	if (!fi) return false;
	FileWriter	writer(fi, platform_BigEndian);
	char header[5] = { 'a', 0, 0, 0, 1 };
	writer.WriteBulk(header, sizeof(header), false);
	for (int x = 0; x < inMap.mWidth; ++x)
	for (int y = 0; y < inMap.mHeight; ++y)
	{
		float v = inMap.mData[x + y * inMap.mWidth];
		writer.WriteFloat(v);
	}
	fclose(fi);
	return true; 
}


#define IMG_X_RES 120
#define IMG_Y_RES 120
#define IMG_X_SIZE (IMG_X_RES * 360)

#if BASTODO
WARNING: we do not handle the right or top edges properly!
#endif

// Given a 120x120 points per deg full world image with the IDL on the left and north pole up
// top, import one degree in a 121x121 DEM.
bool 	ExtractRawIMGFile(DEMGeo& inMap, const char * inFileName, int inWest, int inSouth, int inEast, int inNorth)
{
	// In this case we are only using a FRACTION of the actual needed data.
	// So use stdio.

	// X-Off is the location of the leftmost pixel in our tile	
	int x_off = IMG_X_RES * (inWest + 180);	// X offset puts international dateline on the left edge.
	// Y-off is the location of the topmost pixel in our tile
	int y_off = IMG_Y_RES * (90 - inSouth - 1);	// Y offset puts antarctica on the bottom.
	
	int	imp_y_res = IMG_Y_RES * (inNorth-inSouth);
	int	imp_x_res = IMG_X_RES * (inEast-inWest);
	
	FILE * fi = fopen(inFileName, "rb");
	if (!fi) return false;

	unsigned char *	membuf = new unsigned char[imp_x_res+1];

	inMap.resize(imp_y_res+1,imp_x_res+1);
	inMap.mSouth = inSouth;
	inMap.mNorth = inSouth+1;
	inMap.mWest = inWest;
	inMap.mEast = inWest+1;
	
	float	mmax = -300, mmin = 300;
	
	for (int y = 0; y <= imp_y_res; ++y)
	{
		fseek(fi, (y_off + IMG_Y_RES - y) * IMG_X_SIZE + (x_off), SEEK_SET);
		fread(membuf, imp_x_res+1, 1, fi);
		for (int x = 0; x <= imp_x_res; ++x)
		{
			float e = inMap(x,y) = membuf[x];
			if (e > mmax) mmax = e;
			if (e < mmin) mmin = e;
		}
	}
	fclose(fi);
	delete [] membuf;
	printf("Land uses from %f to %f\n", mmin, mmax);
	return true;
}


bool	ExtractIDAFile(DEMGeo& inMap, const char * inFileName)
{
	MFMemFile * fi = MemFile_Open(inFileName);
	if (fi == NULL) return false;
	bool	 ok = false;
	const unsigned char * bp = (const unsigned char *) MemFile_GetBegin(fi);
	const unsigned char * ep = (const unsigned char *) MemFile_GetEnd(fi);
	if ((ep - bp) < 512) goto bail;
	// More info is available in said file:
	// http://www.fao.org/giews/english/windisp/manuals/WD35EN25.htm
	// 30-32	height					integer (2 bytes)
	// 32-34	width					integer (2 bytes)	
	// 170		missing value			character
	// 171-177	slope (m)				real 6 bits
	// 177-183	intercept (b)			real 6 bits		
	unsigned short height = bp[30] + ((unsigned short) bp[31] << 8);
	unsigned short width  = bp[32] + ((unsigned short) bp[33] << 8);
	unsigned char  missing = bp[170];
	double		   m = ReadReal48(bp+171);
	double		   b = ReadReal48(bp+177);
	
	if ((ep-bp) < (512 + width * height)) goto bail;
	
	printf("File %s: %dx%d, slope=%lf,intercept=%lf, null val = %02x\n",
		inFileName, width, height, m, b, missing);
		
	inMap.resize(width, height);
	for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
		unsigned char v = bp[512+width * (height-y-1) + x];
		if (v == missing)
			inMap(x,y) = NO_DATA;
		else
			inMap(x,y) = m * (double) v + b;
	}
	ok = true;
bail:	
	MemFile_Close(fi);
	return ok;
}

static	void	trim_down(const char ** sp, const char ** ep)
{
	const char * s = *sp;
	const char * e = *ep;
	while (s < e && *s == ' ') ++s;
	while (s < e && *(e-1) == ' ') --e;
	*sp = s;
	*ep = e;
}

int parse_field_int(const char ** s, const char * e)
{
	const char * p = *s;
	int result = 0;
	while (p < e && *p == ' ') ++p;
	int sign = 1;
	if (p < e && *p == '-') sign=-1,++p;
	if (p < e && *p == '+') sign= 1,++p;
	while (p < e && *p >= '0' && *p <= '9')
	{
		result *= 10;
		result += (*p - '0');
		++p;
	}
	*s = p;
	return result * sign;
}

double	parse_field_float(const char ** s, const char * e)
{
	const char * p = *s;
	double	mantissa = 0.0;
	int exponent = 0;
	int digits = 0;
	while (p < e && *p == ' ') ++p;
	double sign = 1;
	if (p < e && *p == '-') sign=-1,++p;
	if (p < e && *p == '+') sign= 1,++p;
	if (p >= e || *p != '0') goto bail;	++p;
	if (p >= e || *p != '.') goto bail;	++p;
	while (p < e && *p >= '0' && *p <= '9')
	{
		mantissa *= 10.0;
		mantissa += (*p - '0');
		++p;
		++digits;
	}
	if (p >= e || (*p != 'D' && *p != 'd' && *p != 'e' && *p != 'E')) goto bail;	++p;
	exponent = parse_field_int(&p, e);
	*s = p;
	int	rshift = digits - exponent;
	if (rshift > 0)
		return sign * (mantissa / pow(10, rshift));
	else
		return sign * (mantissa * pow(10, rshift));
bail:	
	*s = p;
	return 0;
	
}

// This routine parses a USGS "natural format" DEM, e.g. a DEM as a series of elevations in ASCII.
// For the specs on this see:
//
// http://rockyweb.cr.usgs.gov/nmpstds/demstds.html
//
// Limitations: we only handle: geographic projection, meters and degrees.
// Limitations: we do not handle any rotation of the grid data, datum offsets,
// 					or anything funky in the column/row system.
//
// Note: the spec says that there will always be one  or more B record per column and one
// column per profile, but the spec has fields to allow otherwise.  So we blow chunks if we
// see multidimensional profiles.
//
bool	ExtractUSGSNaturalFile(DEMGeo& inMap, const char * inFileName)
{
	int	n;
	
	MFMemFile * fi = MemFile_Open(inFileName);
	if (fi == NULL) return false;
	
	const char * b = MemFile_GetBegin(fi);
	const char * s, * e;
	s=b    ;    e=b+ 40;	trim_down(&s, &e);	string	fname(s,e);
	s=b+156;	e=b+162;	int geo = parse_field_int(&s, e);
	s=b+528;	e=b+534;	int hunits = parse_field_int(&s, e);
	s=b+534;	e=b+540;	int vunits = parse_field_int(&s, e);
	s=b+546;	e=b+738;	double	west = parse_field_float(&s, e) / 3600.0; double	south = parse_field_float(&s, e)  / 3600.0;
							parse_field_float(&s, e)  / 3600.0;	parse_field_float(&s, e)  / 3600.0;
							double	east = parse_field_float(&s, e) / 3600.0; double	north = parse_field_float(&s, e)  / 3600.0;
	s=b+852;	e=b+864;	int k = parse_field_int(&s, e); int profiles = parse_field_int(&s, e);
		
	if (geo != 0)		{printf("ERROR: %s not geo projected.\n", inFileName);	goto bail;}
	if (hunits != 3)	{printf("ERROR: %s not in arc seconds.\n", inFileName);	goto bail;}
	if (vunits != 2)	{printf("ERROR: %s not in meters.\n", inFileName);		goto bail;}
	
	printf("File name: '%s'\n", fname.c_str());
	printf("Geocoding: %d\n", geo);
	printf("Ground Units: %d\n", hunits);
	printf("Elevation Units: %d\n", vunits);
	printf("Profiles: %d\n", profiles);
	printf("Bounds: %lf %lf -> %lf %lf\n", west, south, east, north);
	
	if (k != 1) { printf("ERROR: expect 1 count of profiles.\n");	goto bail; }

	inMap.mWest = west;
	inMap.mEast = east;
	inMap.mNorth = north;
	inMap.mSouth = south;

	const char * p = b + 1024;
	n = 0;
	int total_profiles = profiles;
	while (profiles > 0)
	{
		int ox, oy, count;
		if (p >= MemFile_GetEnd(fi)) { printf("ERROR: out of files bounds.\n"); goto bail; }
		s=p    ;	e=p+ 12;	oy = parse_field_int(&s,e)-1; ox = parse_field_int(&s,e)-1;
		s=p +12;	e=p+ 24;	count = parse_field_int(&s,e); k = parse_field_int(&s,e);
		s=p +72;	e=p+ 96;	trim_down(&s,&e);		string datum(s,e);

		if (k != 1) { printf("ERROR, expect 1 count inside profiles.\n"); goto bail; }

//		printf("   Will read %d scans at %d %d\n", count, ox, oy);
		if (inMap.mWidth != total_profiles || inMap.mHeight != count)
		{
			printf("Setting DEM size to %d, %d\n", total_profiles, count);
			inMap.resize(total_profiles, count);
		}
		bool	is_first_record = true;
		while (count > 0)
		{
			const char * o = is_first_record ? (p + 144) : p;
			const char * e = MemFile_GetEnd(fi);
			int	max_per_record = is_first_record ? 146 : 170;
			int num_read = (max_per_record < count) ? max_per_record : count;
//			printf("       Will read %d starting at %d, first rec = %s\n", num_read, o-p, is_first_record ? "true" : "false");
			for (int n = 0; n < num_read; ++n)
			{
				int elev = parse_field_int(&o, e);
				if (o >= e) {printf("ERROR: overrun, n = %d\n", n); goto bail; }
				inMap(ox, oy) = elev;
				++oy;
			}
			count -= num_read;
			if (count > 0)
			{
				p += 1024;
				++n;
				is_first_record = false;
			}
		}
		p += 1024;
		--profiles;
		++n;
	}
	printf("Read %d records.\n", n);
	MemFile_Close(fi);
	return true;
bail:
	MemFile_Close(fi);
	return false;
}

static	void	IgnoreTiffWarnings(const char *, const char* fmt, va_list args)
{
	vprintf(fmt, args);
}

static	void	IgnoreTiffErrs(const char *, const char* fmt, va_list args)
{
	vprintf(fmt, args);	
}

struct	StTiffMemFile {
	StTiffMemFile(const char * fname) { file = MemFile_Open(fname); offset = 0; }
	~StTiffMemFile() { if (file) MemFile_Close(file); }

	MFMemFile *		file;
	int				offset;
};	

static tsize_t	MemTIFFReadWriteProc(thandle_t handle, tdata_t data, tsize_t len)
{
	StTiffMemFile *	f = (StTiffMemFile *) handle;
	int remain = MemFile_GetEnd(f->file) - MemFile_GetBegin(f->file) - f->offset;
	if (len > remain) len = remain;
	if (len < 0) len = 0;
	if (len > 0)
	memcpy(data, MemFile_GetBegin(f->file) + f->offset, len);
	f->offset += len;
	return len;
}

static toff_t 	MemTIFFSeekProc(thandle_t handle, toff_t pos, int mode)
{
	StTiffMemFile *	f = (StTiffMemFile *) handle;	
	switch(mode) {
	case SEEK_SET:
	default:
		f->offset = pos;
		return f->offset;
	case SEEK_CUR:
		f->offset += pos;
		return f->offset;
	case SEEK_END:
		f->offset = MemFile_GetEnd(f->file) - MemFile_GetBegin(f->file) - pos;
		return f->offset;
	}
}

static int 		MemTIFFCloseProc(thandle_t)
{
	return 0;
}

static toff_t 	MemTIFFSizeProc(thandle_t handle)
{
	StTiffMemFile *	f = (StTiffMemFile *) handle;
	return MemFile_GetEnd(f->file) - MemFile_GetBegin(f->file);
}

static int 		MemTIFFMapFileProc(thandle_t handle, tdata_t* dp, toff_t* len)
{
	StTiffMemFile *	f = (StTiffMemFile *) handle;
	*dp = (tdata_t) MemFile_GetBegin(f->file);
	*len = MemFile_GetEnd(f->file) - MemFile_GetBegin(f->file);
	return 1;
}

static void 	MemTIFFUnmapFileProc(thandle_t, tdata_t, toff_t)
{
}


/*
	GeoTiff notes - 
	First of all, Geotiff - unlike our DEMs, the first scanline is the "top" of the image, meaning north-most scanline.
	Left edge is west like we expect.
	
	Pixels can be 'area' or 'point', but the distinction is moot...the center of the pixel, not the lower left corner,
	is what corresponds to the geo-coding references.
	
	SRTM Problems:
	The original SRTMs featured 1201 samples covering a single degree with point samples, covering both edges.  This is 
	just like our internal system works.
	
	The GeoTiff SRTM recuts contain 1200 samples per tile.  For a given degree tile, the 1200 samples include the west'
	and south but not north and east edge of the original SRTM file.
	
	BUT here's where things get weird: the samples are listed as area, with the tie points 1.5 arc-seconds to the east and 
	north?  Why?  Well, it looks to me like someone misinterpretted the original SRTMs as being area points, with 1200 
	points covering the DEM.  (With area pixels, we'd expect the left edge of pixel 0 to touch the left boundary and the
	right edge of pixel 1199 to cover the right boundary.  With single sample pixels, pixel 0 and 1200 are directly on
	the border.)
	
	In other words, the CGIAR SRTM files have essentially been shifted to the northeast by 1.5 arc-seconds.

*/
bool	ExtractGeoTiff(DEMGeo& inMap, const char * inFileName)
{
	int result = -1;
	double	corners[8];
	TIFFErrorHandler	warnH = TIFFSetWarningHandler(IgnoreTiffWarnings);
	TIFFErrorHandler	errH = TIFFSetErrorHandler(IgnoreTiffErrs);
	StTiffMemFile	tiffMem(inFileName);
	if (tiffMem.file == NULL) goto bail;
	printf("Trying file: %s\n", inFileName);
	TIFF * tif = XTIFFClientOpen(inFileName, "r", &tiffMem,
	    MemTIFFReadWriteProc, MemTIFFReadWriteProc,
	    MemTIFFSeekProc, MemTIFFCloseProc,
	    MemTIFFSizeProc,
	    MemTIFFMapFileProc, MemTIFFUnmapFileProc);
	printf("Opened TIF file.\n");
//    TIFF* tif = TIFFOpen(inFileName, "r");
    if (tif == NULL) goto bail;

	if (!FetchTIFFCornersWithTIFF(tif, corners))
	{
		printf("Could not read GeoTiff projection data.\n");
		return false;
	}

	inMap.mWest = round(corners[0]);
	inMap.mSouth = round(corners[1]);
	inMap.mEast = round(corners[6]);
	inMap.mNorth = round(corners[7]);
	
	printf("Corners: %lf,%lf   %lf,%lf   %lf,%lf   %lf,%lf\n",
		corners[0], corners[1], corners[2], corners[3], corners[4], corners[5], corners[6], corners[7]);

	uint32 w, h;
	uint16 cc;
	uint16 d;
//	size_t npixels;
//	uint32* raster;
		
		
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &cc);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &d);
	printf("Image is: %dx%d, samples: %d, depth: %d\n", w, h, cc, d);

	inMap.resize(w+1,h+1);

	tsize_t line_size = TIFFScanlineSize(tif);
	tdata_t aline = _TIFFmalloc(line_size);
	
	int cs = TIFFCurrentStrip(tif);
//	TIFFSetupStrips
	int nos = TIFFNumberOfStrips(tif);
	int cr = TIFFCurrentRow  (tif);
//	printf("cs = %d, nos = %d, cr = %d, linesize=%d\n", cs, nos, cr,line_size);
	
	for (int y = 0; y < h; ++y)
	{
//		printf("Reading line %d\n", y);
		result = TIFFReadScanline(tif, aline, y, 0);
		if (result == -1) { printf("Tiff error in read.\n"); break; }
		
		if (d == 16)
		{
			short * v = (short *) aline;
			for (int x = 0; x < w; ++x, ++v)
			{
				float e = *v;
				if (*v == 0xFFFF) e = NO_DATA;
				if (e > 30000) e = NO_DATA;		// SRTM HACK!!!
				if (e < -30000) e = NO_DATA;		// SRTM HACK!!!
				inMap(x,h-y-1) = e;
				if (y == 0)
					inMap(x,h) = e;
			}
		} else {
			char * v = (char *) aline;
			for (int x = 0; x < w; ++x, ++v)
			{
				float e = *v;
				if (*v == 0xFF) e = NO_DATA;
				if (e > 127) e = NO_DATA;		// SRTM HACK!!!
				inMap(x,h-y-1) = e;
				if (y == 0)
					inMap(x,h) = e;
			}
		}		
		inMap(w,y) = inMap(w-1,y);
	}
//	printf("Extra byte.\n");
	inMap(w,h) = inMap(w-1,h-1);
//	printf("Done reading.\n");
	_TIFFfree(aline);

	TIFFClose(tif);    
//	printf("closed file.\n");
	TIFFSetWarningHandler(warnH);
	TIFFSetErrorHandler(errH);
    return result != -1;
bail:
	TIFFSetWarningHandler(warnH);
	TIFFSetErrorHandler(errH);
	return false;

}

// DTED - military elevation DEMs.  I found a spec here:
// http://www.nga.mil/ast/fm/acq/89020B.pdf
// DTED comes with lots of packaging but we just open the .dt0, .dt1 or .dt2 file.
// We read a few headers, jump ahead, and splat down the fixed-length records.
//
// UHL (80 bytes) - user header
// DSI (648 bytes) - data info
// ACC (2700 bytes) - accuracy info
// Then follows the real data records.  For each scanline we have:
// 	252 3-byte block count, 2-byte lon, 2-byte lat then
//	magnitude-signed big-endian shorts for elevatoin
//  4-byte checksum

struct DTED_UHL_t {
	char			cookie[3];					// Must be 'UHL'
	char			version;					// Must be	'1'
	char			longitude[8];				// West edge In form of 1180000W = -118.0
	char			latitude[8];				// South edge In form of 0340000N = 34.0
	char			lon_interval[4];			// Approx lon spacing in tenth-arc-seconds, e.g.  0300 = 30 arc-second
	char			lat_interval[4];			// Approx lat spacing in tenth-arc-seconds, e.g.  0300 = 30 arc-second
	char			vert_accuracy[4];			// Absolute vertical accuray, 90th percentile in meters, e.g. 0026 = 26m might be "NA"
	char			security_code[3];			// Securtity code, might be 'U  ' one of S=Secret,C=Confidential,U=Unclassified,R=Restricted
	char			unique_ref_num[12];			// May be blank, some kind of producer-defined blob
	char			num_lines_lon[4];			// Num scanlines H, e.g. 0121 = 121 pixels wide
	char			num_lines_lat[4];			// Num scanlines V, e.g. 3601 = 3601 pixels wide
	char			multiple_accuracy_flag;		// '0' or '1' 
	char			reserved[24];				// Usually lblank
};	

bool	ExtractDTED(DEMGeo& inMap, const char * inFileName)
{	
	MFMemFile *			fi = NULL;
	const DTED_UHL_t *	uhl;
	const char *		base_ptr;
	int					x, y;
	const char *		end_ptr;

	fi = MemFile_Open(inFileName);
	if (fi == NULL) goto bail;
	if (MemFile_GetEnd(fi) - MemFile_GetBegin(fi) < (80 + 648 + 2700)) goto bail;
	
	uhl = (const DTED_UHL_t *) MemFile_GetBegin(fi);
	
	if (uhl->cookie[0] != 'U' || uhl->cookie[1] != 'H' || uhl->cookie[2] != 'L') goto bail;
	if (uhl->version != '1') goto bail;
	
	inMap.mSouth = 
		(uhl->latitude[0]-'0') * 1000000 +
		(uhl->latitude[1]-'0') * 100000 +
		(uhl->latitude[2]-'0') * 10000 +
		(uhl->latitude[3]-'0') * 1000+
		(uhl->latitude[4]-'0') * 100 +
		(uhl->latitude[5]-'0') * 10 +
		(uhl->latitude[6]-'0') * 1;
	if (uhl->latitude[7] == 'S' || uhl->latitude[7] == 's')
		inMap.mSouth = -inMap.mSouth;
	inMap.mSouth /= 10000.0;

	inMap.mWest = 
		(uhl->longitude[0]-'0') * 1000000 +
		(uhl->longitude[1]-'0') * 100000 +
		(uhl->longitude[2]-'0') * 10000 +
		(uhl->longitude[3]-'0') * 1000+
		(uhl->longitude[4]-'0') * 100 +
		(uhl->longitude[5]-'0') * 10 +
		(uhl->longitude[6]-'0') * 1;
	if (uhl->longitude[7] == 'W' || uhl->longitude[7] == 'w')
		inMap.mWest = -inMap.mWest;
	inMap.mWest /= 10000.0;
	
	inMap.mNorth = inMap.mSouth + 1.0;
	inMap.mEast = inMap.mWest + 1.0;
	
	int x_size = (uhl->num_lines_lon[0]-'0') * 1000 + 
				 (uhl->num_lines_lon[1]-'0') * 100 + 
				 (uhl->num_lines_lon[2]-'0') * 10 + 
				 (uhl->num_lines_lon[3]-'0') * 1;
	int y_size = (uhl->num_lines_lat[0]-'0') * 1000 + 
				 (uhl->num_lines_lat[1]-'0') * 100 + 
				 (uhl->num_lines_lat[2]-'0') * 10 + 
				 (uhl->num_lines_lat[3]-'0') * 1;

	if (x_size < 1 || y_size < 1 || x_size > 10000 || y_size > 10000) goto bail;
	if (inMap.mWest < -180.0 || inMap.mEast > 180.0 || inMap.mSouth < -90.0 || inMap.mNorth > 90.0) goto bail;
	
	inMap.resize(x_size, y_size);

	base_ptr = MemFile_GetBegin(fi) + 80 + 648 + 2700;
	end_ptr = MemFile_GetEnd(fi);
	for (x = 0; x < x_size; ++x)
	{
		base_ptr += 8;
		for (y = 0; y < y_size; ++y)
		{
			if (base_ptr >= end_ptr) goto bail;
			unsigned char c1 = (unsigned char) *base_ptr++;
			if (base_ptr >= end_ptr) goto bail;
			unsigned char c2 = (unsigned char) *base_ptr++;
			
			float height = c2 + ((c1 & 0x7F) << 8);
			if (c1 & 0x80)	height = -height;
			if (height == -32767.0)	height = NO_DATA;
			inMap(x,y) = height;			
		}
		base_ptr += 4;
	}
	
	MemFile_Close(fi);
	return true;
bail:
	if (fi) MemFile_Close(fi);
	return false;
}

#pragma mark -

static vector<int> *		sTranslateMap = NULL;

static 	bool	DEMLineImporter(const vector<string>& inTokenLine, void * inRef)
{
	if (inTokenLine.size() != 3)
	{
		printf("Bad DEM import line.\n");
		return false;
	}
	int key = TokenizeInt(inTokenLine[1]);
	int value = LookupTokenCreate(inTokenLine[2].c_str());
	if (value == -1)
	{
		printf("Unknown token %s\n", inTokenLine[2].c_str());
		return false;
	}
	if (sTranslateMap == NULL) 
	{
		printf("LU_IMPORT line hit unexpecetedly.\n");
		return false;
	}
	if (key >= sTranslateMap->size())
	{
		int start_fill = sTranslateMap->size();
		sTranslateMap->resize(key+1);
		while (start_fill < sTranslateMap->size())
		{
			(*sTranslateMap)[start_fill++] = NO_VALUE;
		}
	}
	(*sTranslateMap)[key] = value;
	return true;
}

bool	LoadTranslationFile(const char * 		inFileName, 
						vector<int>& 			outForwardMap, 
						hash_map<int, int> * 	outReverseMap,
						vector<char> *			outCLUT)
{
	static bool	first_time = true;
	if (first_time)
	{
		RegisterLineHandler("LU_IMPORT", DEMLineImporter, NULL);
		first_time = false;
	}
	outForwardMap.clear();
	sTranslateMap = &outForwardMap;
	if (!LoadConfigFile(inFileName))
	{
		printf("Could not load config file %s\n", inFileName);
		return false;
	}
	sTranslateMap = NULL;
	
	if (outReverseMap)
	{
		outReverseMap->clear();
		for (int n = 0; n < outForwardMap.size(); ++n)
			(*outReverseMap)[outForwardMap[n]] = n;
	}
	if (outCLUT)
	{
		outCLUT->clear();
		outCLUT->resize(outForwardMap.size() * 3);
		for (int n = 0; n < outForwardMap.size(); ++n)
		{
			if (gEnumColors.count(outForwardMap[n]) > 0)
			{
				RGBColor_t& c = gEnumColors[outForwardMap[n]];
				(*outCLUT)[n*3  ] = c.rgb[0] * 255.0;
				(*outCLUT)[n*3+1] = c.rgb[1] * 255.0;
				(*outCLUT)[n*3+2] = c.rgb[2] * 255.0;				
			} else {
				(*outCLUT)[n*3  ] = 0;
				(*outCLUT)[n*3+1] = 0;
				(*outCLUT)[n*3+2] = 0;
			}
		}
	}
	return true;
}

bool	TranslateDEMForward(DEMGeo& ioDem, const vector<int>& inForwardMap)
{
	bool	ret = true;
	for (int x = 0; x < ioDem.mWidth; ++x)
	for (int y = 0; y < ioDem.mHeight;++y)
	{
		int v = ioDem(x,y);
		
			 if (v < 0					  ){ioDem(x,y)=NO_DATA;					ret = false;	}
		else if (v >= inForwardMap.size()) {ioDem(x,y)=NO_DATA;					ret = false;	}
		else 							   {ioDem(x,y)=inForwardMap[v];							}
	}
	return ret;
}

bool	TranslateDEMReverse(DEMGeo& ioDem, const hash_map<int, int>& inReverseMap)
{
	bool	ret = true;
	for (int x = 0; x < ioDem.mWidth; ++x)
	for (int y = 0; y < ioDem.mHeight;++y)
	{
		int v = ioDem(x,y);
		hash_map<int,int>::const_iterator i = inReverseMap.find(v);
		if (i == inReverseMap.end())
		{
			ioDem(x,y)=NO_DATA;
			ret = false;
		} else 
			ioDem(x,y) = i->second;
	}
	return ret;

}

bool	TranslateDEM(DEMGeo& ioDEM, const char * inFileName)
{
	vector<int>	mapping;
	if (!LoadTranslationFile(inFileName, mapping, NULL, NULL))	return false;
	TranslateDEMForward(ioDEM, mapping);
	return true;
}

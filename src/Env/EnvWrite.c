/*
 * Copyright (c) 2007, Laminar Research.
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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "PlatformUtils.h"
#include "EndianUtils.h"
#include <math.h>
#include "EnvWrite.h"
#include "EnvDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
const char *	StripFileName(const char * inFilePath, char inSeparator);
#ifdef __cplusplus
}
#endif

static	long	ENCODE_LAT_LON(double lat, double lon, double latRef, double lonRef);

const	char	kEndianSwapVertex610BEWrite [] = { -4, 2, 0 };	/* Swap native to BE for elevation */

int	EnvWrite(const char * inFileName)
{
		FILE *		fi = NULL;
		int			err = 0;
		Vertex610	vertex;
		long		latRef, lonRef;

	sscanf(StripFileName(inFileName, DIR_CHAR), "%ld%ld", &latRef, &lonRef);

	fi = fopen(inFileName, "wb");
	if (fi == NULL)
	{
		err = -1;
		goto bail;
	}

	err = WRITE_ERR;

	/* HEADER
	 *
	 * The header is a one-byte endian code, followed by a four-byte version.
	 *
	 */

	{
			char		format;
			long		version;

		format = ((DIR_CHAR == ':') || (DIR_CHAR == '/')) ? kFormatMac : kFormatIntel;
		version = kVersion650;

		if (fwrite(&format, sizeof(format), 1, fi) != 1)
			goto bail;

		if (fwrite(&version, sizeof(version), 1, fi) != 1)
			goto bail;
	}

	/* VERTICES
	 *
	 * There are 201 vertical by 151 horizontal vertices, containing 200x150 quads.  They are
	 * written from left to right, then bottom to top.  The bottom left point is always at the lat/lon
	 * coordinates of the .env file's name.
	 *
	 * The vertexes are written as 10-byte chunks; the first 4 are the fractional lat/lon deltas of the vertex
	 * in whole number parts of 9999 with the latitude multiplied by 10000.
	 *
	 * Then comes a two-byte altitude in MSL meters plus 10000.  It is always big endian!
	 *
	 * Finally a texture code.  The low 3 decimal digits are the texture or land use, then a 1 or 0 for custom or
	 * default texture, then an empty place, then 1-4 for rotation (4=straight up, subtract one for each 90 degree
	 * CCW rotation), then the scale, y offset, and xoffset of the texture.  0 means no scaling, 1 means use half the
	 * texture, etc.
	 *
	 */

	{
			int		h, v, custom;
			double 	lat, lon;
			float	elev;
			short	use, rotation, scale, xoff, yoff, bodyID;

		for (v = 0; v < kEnvHeight; ++v)
			for (h = 0; h < kEnvWidth; ++h)
			{
				GetNthVertex(h, v, &lat, &lon, &elev, &use, &custom, &rotation, &scale, &xoff, &yoff, &bodyID);

				vertex.latLonCode = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
				vertex.altitude = elev + 10000.0;
				vertex.texture = use % 1000;
				if (custom) vertex.texture += 1000;
				vertex.texture += ((360 - rotation) / 90) * 100000;

				if (!custom)
					vertex.texture += 1000000 * bodyID;
				else {
					vertex.texture += 1000000 * scale;
					vertex.texture += 10000000 * yoff;
					vertex.texture += 100000000 * xoff;
				}

				EndianSwapBuffer(platform_Native, platform_BigEndian, kEndianSwapVertex610BEWrite, &vertex);

				if (fwrite(&vertex, sizeof(vertex), 1, fi) != 1)
					goto bail;
			}
	}

	/* OBSTACLES
	 *
	 * Each obstacle has a 4-byte kind integer code, 4 byte floating lat
	 * and lon.  For built-in obstacles, the next 4 bytes are a floating point altitude AGL meters.
	 * For custom objects, a 4-byte floating point heading in degrees followed by a 150-byte 0-padded
	 * custom object name.   A kind of 99 indicates the end of data (with no data following it for objects).
	 *
	 */

	{
			long		index = 0, stopCode = kObstacleTypeStop;
			Obstacle	obs;
			char		name[ENV_STR_SIZE+1];

		memset(name, 0, ENV_STR_SIZE+1);
		while (GetNthObject(index++, &obs.type, &obs.lat, &obs.lon, &obs.heading, name))
		{
			if (fwrite(&obs, sizeof(obs), 1, fi) != 1)
				goto bail;
			if (obs.type == kObstacleTypeCustom)
				if (fwrite(name, ENV_STR_SIZE, 1, fi) != 1)
					goto bail;
			memset(name, 0, ENV_STR_SIZE+1);
		}
		if (fwrite(&stopCode, sizeof(stopCode), 1, fi) !=1)
			goto bail;
	}

	/* PATHS
	 *
	 * There are four kinds of paths in order in the file: roads, trails, train tracks, and power lines.
	 * Each has the same format: a list of 4-byte integer codes, with 99 to stop.  The codes are fractional
	 * lat/lon offsets from the lower left of the .env, each as whole parts of 9999, the lat multiplied by
	 * 10000, then added together.  If the point is the last in a chain, this code is then made negative.
	 *
	 */

	{
			long			index;
			long			code;
			char			taxiCode;
			double			lat, lon;
			float			flat, flon;
			int				last;

		/* Roads */
		index = 0;
		while (GetNthRoadSegment(index++, &lat, &lon, &last))
		{
			code = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
			if (last)
				code = -code;
			if (code == kRoadStop)
				--code;
			if (fwrite(&code, sizeof(code), 1, fi) != 1)
				goto bail;
		}
		code = kRoadStop;
		if (fwrite(&code, sizeof(code), 1, fi) != 1)
			goto bail;

		/* Trails */
		index = 0;
		while (GetNthTrailSegment(index++, &lat, &lon, &last))
		{
			code = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
			if (last)
				code = -code;
			if (code == kRoadStop)
				--code;
			if (fwrite(&code, sizeof(code), 1, fi) != 1)
				goto bail;
		}
		code = kRoadStop;
		if (fwrite(&code, sizeof(code), 1, fi) != 1)
			goto bail;

		/* Railroad Tracks */
		index = 0;
		while (GetNthTrainSegment(index++, &lat, &lon, &last))
		{
			code = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
			if (last)
				code = -code;
			if (code == kRoadStop)
				--code;
			if (fwrite(&code, sizeof(code), 1, fi) != 1)
				goto bail;
		}
		code = kRoadStop;
		if (fwrite(&code, sizeof(code), 1, fi) != 1)
			goto bail;

		/* Power Lines */
		index = 0;
		while (GetNthPowerSegment(index++, &lat, &lon, &last))
		{
			code = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
			if (last)
				code = -code;
			if (code == kRoadStop)
				--code;
			if (fwrite(&code, sizeof(code), 1, fi) != 1)
				goto bail;
		}
		code = kRoadStop;
		if (fwrite(&code, sizeof(code), 1, fi) != 1)
			goto bail;

		/* Taxiwys */
		index = 0;
		while (GetNthTaxiwaySegment(index++, &lat, &lon, &last))
		{
			flat = lat;
			flon = lon;
			taxiCode = (last ? kTaxiGroupBreak : kTaxiGroupContinue);
			if (fwrite(&flat, sizeof(flat), 1, fi) != 1)
				goto bail;
			if (fwrite(&flon, sizeof(flon), 1, fi) != 1)
				goto bail;
			if (fwrite(&taxiCode, sizeof(taxiCode), 1, fi) != 1)
				goto bail;
		}
		taxiCode = kTaxiGroupEnd;
		flat = kRoadStop;
		flon = kRoadStop;
		if (fwrite(&flat, sizeof(flat), 1, fi) != 1)
			goto bail;
		if (fwrite(&flon, sizeof(flon), 1, fi) != 1)
			goto bail;
		if (fwrite(&taxiCode, sizeof(taxiCode), 1, fi) != 1)
			goto bail;


		/* Rivers */
		index = 0;
		while (GetNthRiverSegment(index++, &lat, &lon, &last))
		{
			code = ENCODE_LAT_LON(lat, lon, latRef, lonRef);
			if (last)
				code = -code;
			if (code == kRoadStop)
				--code;
			if (fwrite(&code, sizeof(code), 1, fi) != 1)
				goto bail;
		}
		code = kRoadStop;
		if (fwrite(&code, sizeof(code), 1, fi) != 1)
			goto bail;


	}

	/* CUSTOM TEXTURES
	 *
	 * Each custom texture is given a 150-byte slot in a table, counted from 0 for
	 * purposes above (I think).  Each entry is a 0-padded C string, or "Untitled"
	 * (zero padded) for unused entries.  The table stops when there are no more
	 * valid entries in the file.
	 *
	 */

	{
			char	name[ENV_STR_SIZE+1];
			long	index = 0;
		memset(name, 0, ENV_STR_SIZE+1);
		strcpy(name, "Untitled");

		while (GetNthTexture(index++, name))
		{
			if (fwrite(name, ENV_STR_SIZE, 1, fi) != 1)
				goto bail;
			memset(name, 0, ENV_STR_SIZE+1);
			strcpy(name, "Untitled");
		}
	}

	err = 0;
bail:
	fclose(fi);
	return err;

}

/*
 * ENCODE_LAT_LON
 *
 * This routine encodes the latitude and longitude by:
 *
 * 1. Taking a difference of the point to encode from our "reference" (the lower left
 *	  corner of the .env, then
 * 2. Multiplying by 9999 and rounding to get parts of 9999, then
 * 3. Shifting latitude by 10000 and then
 * 4. Adding them together.
 *
 */


long	ENCODE_LAT_LON(double lat, double lon, double latRef, double lonRef)
{
	if ((lat - latRef) < 0.0)
		printf("WARNING: lat too low.\t\t%lf,%lf,%lf,%lf.\n", latRef, lonRef, lat, lon);
	if ((lon - lonRef) < 0.0)
		printf("WARNING: lon too left.\t\t%lf,%lf,%lf,%lf.\n", latRef, lonRef, lat, lon);
	if ((lat - latRef) > 1.0)
		printf("WARNING: lat too high.\t\t%lf,%lf,%lf,%lf.\n", latRef, lonRef, lat, lon);
	if ((lon - lonRef) > 1.0)
		printf("WARNING: lon too right.\t\t%lf,%lf,%lf,%lf.\n", latRef, lonRef, lat, lon);
	return 10000 * (long) round((lat - (double) latRef) * 9999.0) + (long) round((lon - (double) lonRef) * 9999.0);
}
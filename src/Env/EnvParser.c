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

#include "EnvParser.h"
#include <stdio.h>
#include <string.h>
#include "PlatformUtils.h"
#include "EnvDefs.h"
#include "EndianUtils.h"
#include "errno.h"

/* Endian swapping arrays.  See EndianUtils.h; basically they describe the data size of structures as we read them in. */
const	char	kEndianSwapVersion [] = { 4, 0 };
const	char	kEndianSwapVertex606 [] = { 4, 4, 4, 4, 0 };
const	char	kEndianSwapVertex610 [] = { 4, -2, 4, 0 };	/* Swap file to native */
const	char	kEndianSwapVertex610BE [] = { -4, 2, 0 };	/* Swap BE to native */
const	char	kEndianSwapObstacleCode [] = { 4, 0 };
const	char	kEndianSwapObstacleRemain [] = { 4, 4, 4, 0 };
const	char	kEndianSwapLatLonCode [] = { 4, 0 };
const	char	kEndianSwapFloat [] = { 4, 0 };

static 	int	ReadTerrain606(FILE * inFile, PlatformType inFileEndian);
static	int	ReadTerrain610(FILE * inFile, PlatformType inFileEndian, long inLat, long inLon);
static	int ReadObstacles(FILE * inFile, PlatformType inFileEndian);
static	int ReadTextures(FILE * inFile);
static	int ReadPaths(FILE * inFile, PlatformType inEndian, long inLat, long inLon, int inHasTaxiways, int inHasRivers);
static	int	ReadSpecificPath(FILE * inFile, PlatformType inEndian, long inLat, long inLon, void (* acceptFunc)(double, double, int));
static	int	ReadPreciseSpecificPath(FILE * inFile, PlatformType inEndian, long inLat, long inLon, void (* acceptFunc)(double, double, int));

int	ReadEnvFile(const char * envFileName)
{
//	printf("parsing.\n");

		FILE *			fi = 0;
		char			fileFormat;
		PlatformType	fileEndian;
		long			fileVersion;
		int				err=0;
		long			lat, lon;

	/* Attempt to read the file format for a lat and lon, e.g. +42-71.  We need this
	 * because 6.10 .env files only store lat/lon deltas!
	 */
	sscanf(StripFileName(envFileName, DIR_CHAR), "%ld%ld", &lat, &lon);

	fi = fopen(envFileName, "rb");
	if (fi == NULL)
	{
//		printf("no open: %s.\n", envFileName);
		err = CANNOT_OPEN_ERR;
		goto bail;
	}

	/* First comes a 1-char file format identifier. */

	if (fread(&fileFormat, sizeof(fileFormat), 1, fi) != 1)
	{
//		printf("no read endian.\n");
		err = BAD_FORMAT_ERR;
		goto bail;
	}

	switch(fileFormat) {
	case kFormatMac:
		fileEndian = platform_BigEndian;
		break;
	case kFormatIntel:
		fileEndian = platform_LittleEndian;
		break;
	default:
//		printf("no platform: %c\n", fileFormat);
		err = BAD_FORMAT_ERR;
		goto bail;
	}

	/* Next comes the version.  Four-bye version number. */

	if (fread(&fileVersion, sizeof(fileVersion), 1, fi) != 1)
	{
//		printf("vers read err.\n");

		err = BAD_FORMAT_ERR;
		goto bail;
	}
	EndianSwapBuffer(fileEndian, platform_Native, kEndianSwapVersion, &fileVersion);
//	printf("vers: %d.\n", fileVersion);
	switch(fileVersion) {
	case kVersion606:
		/* The 6.06 file order is terrain, obstacles, textures. */
		err = ReadTerrain606(fi, fileEndian);
		if (err != 0)
			goto bail;
		err = ReadObstacles(fi, fileEndian);
		if (err != 0)
			goto bail;
		err = ReadTextures(fi);
		if (err != 0)
			goto bail;
		break;
	case kVersion610:
	case kVersion631:
	case kVersion650:
		/* The 6.10 file order is compressed terrain, obstacles, roads, textures. */
		err = ReadTerrain610(fi, fileEndian, lat, lon);
		if (err != 0)
		{
//			printf("Bad vers: %d.\n", fileVersion);
			goto bail;
		}
		err = ReadObstacles(fi, fileEndian);
		if (err != 0)
		{
//			printf("Bad obs: %d.\n", fileVersion);
			goto bail;
		}
		err = ReadPaths(fi, fileEndian, lat, lon,
			(fileVersion == kVersion631) || (fileVersion == kVersion650),	// Taxiways in 631 >
			(fileVersion == kVersion650));									// Rivers in 650 >
		if (err != 0)
		{
//			printf("Bad path: %d.\n", fileVersion);
			goto bail;
		}
		err = ReadTextures(fi);
		if (err != 0)
		{
//			printf("Bad tex read: %d.\n", fileVersion);
			goto bail;
		}
		break;
	default:
//		printf("Bad vers: %d.\n", fileVersion);
		err = BAD_FORMAT_ERR;
		goto bail;
	}

bail:
	fclose(fi);
	return err;

}

/*
 * ReadTerrian606
 *
 * This routine reads the vertices in a 6.06 file.  The only interesting thing here is the decoding
 * of the texture field into its component parts.
 *
 */
int	ReadTerrain606(FILE * inFile, PlatformType inFileEndian)
{
		int	h,	v;
		Vertex606	vertex;

	AcceptDimensions(kEnvWidth, kEnvHeight);

	for (v = 0; v < kEnvHeight; ++v)
		for (h = 0; h < kEnvWidth; ++h)
		{
			int custom;
			if (fread(&vertex, sizeof(vertex), 1, inFile) != 1)
				return BAD_FORMAT_ERR;

			EndianSwapBuffer(inFileEndian, platform_Native, kEndianSwapVertex606, &vertex);

			custom = ((vertex.texture / 1000) % 10) > 0;

			AcceptVertex(h, v, vertex.latitude, vertex.longitude, vertex.altitude,
					vertex.texture % 1000,
					custom,
					360 - 90 * ((vertex.texture / 100000) % 10),
					custom ? ((vertex.texture / 1000000) % 10) : 0,
					custom ? ((vertex.texture / 100000000) % 10) : 0,
					custom ? ((vertex.texture / 10000000) % 10) : 0,
					0);

		}
	return 0;

}

/*
 * ReadTerrain610
 *
 * This routine reads the 6.10 vertex data.  Note that part of the vertex info is
 * big endian, while the rest is native endian.  Also, we have to reassemble the
 * elevation and lat/lon which have been compressed.  Texture handling is the same as above.
 *
 */
int	ReadTerrain610(FILE * inFile, PlatformType inFileEndian, long inLat, long inLon)
{
		int	h,	v, custom;
		Vertex610	vertex;
		float		altitude;
		double		lat, lon;
	AcceptDimensions(kEnvWidth, kEnvHeight);

	for (v = 0; v < kEnvHeight; ++v)
		for (h = 0; h < kEnvWidth; ++h)
		{
			if (fread(&vertex, sizeof(vertex), 1, inFile) != 1)
				return BAD_FORMAT_ERR;

			EndianSwapBuffer(inFileEndian, platform_Native, kEndianSwapVertex610, &vertex);
			EndianSwapBuffer(platform_BigEndian, platform_Native, kEndianSwapVertex610BE, &vertex);

			altitude = (float) vertex.altitude - 10000.0;
			lat = (double) (vertex.latLonCode / 10000) / 9999.0;
			lon = (double) (vertex.latLonCode % 10000) / 9999.0;
			lat += (double) inLat;
			lon += (double) inLon;

			custom = ((vertex.texture / 1000) % 10) > 0;

			AcceptVertex(h, v,lat, lon, altitude,
					vertex.texture % 1000,
					custom,
					(360 - 90 * ((vertex.texture / 100000) % 10)) % 360,
					custom ? ((vertex.texture / 1000000) % 10) : 0,
					custom ? ((vertex.texture / 100000000) % 10) : 0,
					custom ? ((vertex.texture / 10000000) % 10) : 0,
					custom ? 0 : ((vertex.texture / 1000000) % 1000));

		}
	return 0;
}

/*
 * ReadObstacles
 *
 * This file reads the variable-length obstacle-data section.
 *
 */
int ReadObstacles(FILE * inFile, PlatformType inFileEndian)
{
		Obstacle	obstacle;
		char		tex[ENV_STR_SIZE + 1];
	while (1)
	{

		if (fread(&obstacle.type, sizeof(obstacle.type), 1, inFile) != 1)
			return BAD_FORMAT_ERR;
		EndianSwapBuffer(inFileEndian, platform_Native, kEndianSwapObstacleCode, &obstacle.type);

		/* If we hit a stop-code we're done. */
		if (obstacle.type == kObstacleTypeStop)
			return 0;

		/* Otherwise read the remainder of the obstacle. */
		if (fread(&obstacle.lat, sizeof(obstacle) - sizeof(obstacle.type), 1, inFile) != 1)
			return BAD_FORMAT_ERR;
		EndianSwapBuffer(inFileEndian, platform_Native, kEndianSwapObstacleRemain, &obstacle.lat);

		if (obstacle.type == kObstacleTypeCustom)
		{
			/* If we're a custom obstacle, a fixed-length object name string follows. */
			if (fread(tex, ENV_STR_SIZE, 1, inFile) != 1)
				return BAD_FORMAT_ERR;

			tex[ENV_STR_SIZE] = 0;
		} else
			tex[0] = 0;
		AcceptObject(obstacle.type, obstacle.lat, obstacle.lon, obstacle.heading, tex);

	}
	return 0;
}

/*
 * ReadTextures
 *
 * This routine reads the textures.  Textures can end in 6.10 when no new ones are left, so
 * if we get an EOF error on an even boundary, we call it a day and return.
 *
 */
int ReadTextures(FILE * inFile)
{
 	char	tex[ENV_STR_SIZE + 1];

 	short	index = 0;
 	while (1)
 	{
	 	long	read = fread(tex, 1, ENV_STR_SIZE, inFile);
		if (read == 0)		/* No textures left. */
			return 0;
		if (read != ENV_STR_SIZE)	/* Weird half-formed texture? */
			return BAD_FORMAT_ERR;

		tex[ENV_STR_SIZE] = 0;

//		if (strcmp(tex, "Untitled"))
			AcceptCustomTexture(index++, tex);
	}
	return 0;
}

/*
 * ReadPaths
 *
 * This routine reads the path-based data (roads, trains, etc.).
 *
 */

int ReadPaths(FILE * inFile, PlatformType inEndian, long inLat, long inLon, int inHasTaxiways, int inHasRivers)
{
//	printf("Getting paths, taxiways? %s\n", inHasTaxiways ? "yes" : "no");
	/* All path data is stored the same way in the following order in the file:
	 * roads, trails, train tracks, electric power lines. */

	/* In each case, we just call a subroutine with a different "acceptor" func. */
	int	err = 0;
	err = ReadSpecificPath(inFile, inEndian, inLat, inLon, AcceptRoadSegment);
//	printf("Read roads, err = %d.\n", err);
	if (err == 0)
		err = ReadSpecificPath(inFile, inEndian, inLat, inLon, AcceptTrailSegment);
//	printf("Read trails, err = %d.\n", err);
	if (err == 0)
		err = ReadSpecificPath(inFile, inEndian, inLat, inLon, AcceptTrainSegment);
//	printf("Read trains, err = %d.\n", err);
	if (err == 0)
		err = ReadSpecificPath(inFile, inEndian, inLat, inLon, AcceptElectricSegment);
//	printf("Read electric, err = %d.\n", err);
	if ((err == 0) && inHasTaxiways)
		err = ReadPreciseSpecificPath(inFile, inEndian, inLat, inLon, AcceptTaxiwaySegment);
	if ((err == 0) && inHasRivers)
		err = ReadSpecificPath(inFile, inEndian, inLat, inLon, AcceptRiverSegment);
//	printf("Read taxiways, err = %d.\n", err);
	return err;
}

/*
 * ReadSpecificPath
 *
 * This routine reads one set of path data.
 *
 * Path data is a set of integer lat/lon codes.  99 indicates the end of that kind of path data.  A negative integer
 * represents the last point in a path.
 *
 * Lat/Lon codes are encoded the same as in 6.10 vertex data above.
 *
 */
int	ReadSpecificPath(FILE * inFile, PlatformType inEndian, long inLat, long inLon, void (* acceptFunc)(double, double, int))
{

		long last_code = 0;
		long	code;
		float	lat;
		float	lon;
		int		endOfSeg = 0;
	while (1)
	{
		if (fread(&code, sizeof(code), 1, inFile) != 1)
		{
//			printf("pathsEnd of file.\n");
			return BAD_FORMAT_ERR;
		}
		if (last_code == code)
			printf("Duplicate point!\n");
		last_code = code;
		EndianSwapBuffer(inEndian, platform_Native, kEndianSwapLatLonCode, &code);

		if (code == kRoadStop)
			return 0;

		if (code < 0)
		{
			endOfSeg = 1;
			code = -code;
		} else
			endOfSeg = 0;

		lat = (double) (code / 10000) / 9999.0;
		lon = (double) (code % 10000) / 9999.0;

//		printf("Read: %f,%f.\n", lat, lon);

		lat += (double) inLat;
		lon += (double) inLon;

		acceptFunc(lat, lon, endOfSeg);

	}
	return 0;
}


/*
 * ReadPreciseSpecificPath
 *
 * This routine reads one set of path data.
 *
 * Each vertex is encoded as a lat and lon (4 byte native endian floats each) followed by a termination character.
 * c = continue group, e = end group, g = end the whole thing.  The lat and lon for the final terminator are not used.
 *
 *
 *
 */
int	ReadPreciseSpecificPath(FILE * inFile, PlatformType inEndian, long inLat, long inLon, void (* acceptFunc)(double, double, int))
{
	char	code;
	float	lat;
	float	lon;
	int		endOfSeg;

	while (1)
	{
		if (fread(&lat, sizeof(lat), 1, inFile) != 1)
		{
//			printf("pathsEnd of file.\n");
			return BAD_FORMAT_ERR;
		}
		if (fread(&lon, sizeof(lon), 1, inFile) != 1)
		{
//			printf("pathsEnd of file.\n");
			return BAD_FORMAT_ERR;
		}
		if (fread(&code, sizeof(code), 1, inFile) != 1)
		{
//			printf("pathsEnd of file.\n");
			return BAD_FORMAT_ERR;
		}
		EndianSwapBuffer(inEndian, platform_Native, kEndianSwapFloat, &lat);
		EndianSwapBuffer(inEndian, platform_Native, kEndianSwapFloat, &lon);
		if (code == kTaxiGroupEnd)
		{
			return 0;
		}
		endOfSeg = (code == kTaxiGroupBreak);

//		printf("Read: %f,%f.  Code = %c\n", lat, lon, code);

		acceptFunc(lat, lon, endOfSeg);

	}
	return 0;
}



/*
 * StripFileName
 *
 * This routine weeds the path data for an env, returning a char * of the file name itself.
 *
 */
const char *	StripFileName(const char * inFilePath, char inSeparator)
{
	long	strLen = strlen(inFilePath);
	const char * iter = inFilePath + strLen - 1;
	while ((iter > inFilePath) && (*iter != inSeparator))
		--iter;
	if ((*iter) == inSeparator)
		++iter;
	return iter;
}


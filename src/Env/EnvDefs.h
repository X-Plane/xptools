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
#ifndef _EnvDefs_h_
#define _EnvDefs_h_

/*
 * Versioning constants.  These identify the format of the .env file.
 *
 */

#define	kVersion606		0x06		/* 006 decimal */
#define	kVersion610		0x3D		/* 061 decimal */
#define kVersion631		0x277		/* 631 decimal */
#define kVersion650		0x28A		/* 650 decimal */

/* This code appears to indicate "no more obstacles." */
#define	kObstacleTypeStop	99

/* This is the type code for custom obstacles. */
#define	kObstacleTypeCustom	8

/* This is the type code for no more roads or paths */
#define kRoadStop	99

/* These characters indicate the endian-ness of the file. */
#define	kFormatMac		'a'
#define	kFormatIntel	'i'

/* The size of fixed-length strings in the .env files. */
#define	ENV_STR_SIZE	150

/* The number of vertices horizontally and vertically in the .env file.  There is one more vertex in each
 * env file than quad. */
#define	kEnvWidth 151
#define	kEnvHeight 201

#define	kTaxiGroupBreak 'e'
#define kTaxiGroupEnd 'g'
#define kTaxiGroupContinue 'c'

/* We have to align to 2-byte boundaries to read the file; I'll fix this to work on the PC eventually. :-( */

#if APL
#pragma pack(2)
#endif
#if IBM
#pragma pack(push, 2)
#endif

/* Old style vertex; each vertex is a 4-byte group of lat, lon, the altitude MSL in meters, and a texture code.
 * The texture code is broken into decimal digits as follows:
 *	XYSURCTTT
 *	TTT = a texture number or land use number, 0-999.  XP 6.06 has 16 built-in land uses; 6.10 has custom defined
 *			land uses.  Custom textures are numbered from 0 and correspond to the items in the custom texture list
 *			at the end of the file.
 *	C = 0 for a default texture/land use, non-zero for a custom texture/land use.
 *	R = CCW rotation of the texture, e.g. 0 = 0, 1 = 270, 2 = 180, 3 = 90, etc.
 *	U = land use code (6.06 only) 1 = ocean/water, 2 = land w/ dynamic scenery, 0 = land w/out dynamic scenery
 *	S = scaling of bitmap 0 = no scale, 1 = scale over 2 tiles, 2 = scale over three, ttc.
 *	X, Y = offset into the bitmap for scaling, 0, 0 means use botleft of bitmap, +x=right, +y=up
 *
 */

typedef	struct {
	float	latitude;
	float	longitude;
	float	altitude;
	long	texture;
} Vertex606;

/* New style vertex; each vertex is a 10 byte group.
 * latLonCode is the delta of this vertices lat/lon from the lat/lon of the lower left of the file (e.g. the lat/lon
 * the .env file is named after), multiplied by 9999.0, with the lat * 10000, then added together.
 *
 * Altitude is the altitude in meters + 10000, as a BIG ENDIAN short.
 * Texture encoding is the same as 6.06 above.
 */

typedef	struct {
	long	latLonCode;
	short	altitude;
	long	texture;
} Vertex610;

/*
 * A custom object.
 *
 * Type is one of:
 *   1 = control tower, 2 = skyscraper, 3 = radio tower, 4 = power tower, 5 = cooling tower,
 *	6 = smokestacks, 8 = custom, 99 = no more objects
 *
 * the heading is the height in meters of the object for standard objects, or the rotation of the object in degrees
 * for custom ones.
 *
 */

typedef	struct {
	long	type;
	float	lat;
	float	lon;
	float	heading;
} Obstacle;

#if APL
#pragma options align=reset
#endif
#if IBM
#pragma pack(pop)
#endif

#endif

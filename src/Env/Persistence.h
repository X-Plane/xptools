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
#ifndef _Persistence_h_
#define _Persistence_h_

#include <vector>
#include <string>

#ifndef PERSIST_VERTICES
#define PERSIST_VERTICES 1
#endif

#ifndef PERSIST_OBJECTS
#define PERSIST_OBJECTS 1
#endif

#ifndef PERSIST_PATHS
#define PERSIST_PATHS 1
#endif

#ifndef PERSIST_TEXTURES
#define PERSIST_TEXTURES 1
#endif

/*

	Persistence
	
	This header and .c implement the file parsing hooks by reading data into and 
	writing it from a series of STL containers.  This makes it easy to read a .env
	file, do some processing, and then write it back out again.
	
*/

#if PERSIST_VERTICES

struct VertexInfo {
	double		latitude;
	double		longitude;
	double		elevation;
	short		landUse;
	int			custom;
	short		rotation;
	short		scale;
	short		xOff;
	short		yOff;
	short		bodyID;
};

extern	std::vector<VertexInfo>			gVertices;

#endif

#if PERSIST_OBJECTS

struct ObjectInfo {
	long	kind;
	double	latitude;
	double	longitude;
	double	elevation;
	std::string	name;
};

extern	std::vector<ObjectInfo>			gObjects;

#endif

#if PERSIST_PATHS

struct PathInfo {
	double	latitude;
	double	longitude;
	int		term;
};

extern	std::vector<PathInfo>			gRoads;
extern	std::vector<PathInfo>			gTrails;
extern	std::vector<PathInfo>			gTrains;
extern	std::vector<PathInfo>			gLines;
extern	std::vector<PathInfo>			gTaxiways;
extern	std::vector<PathInfo>			gRiverVectors;

#endif

#if PERSIST_TEXTURES
extern	std::vector<std::string>		gTextures;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void	ClearEnvData(void);

#ifdef __cplusplus
}
#endif

#endif
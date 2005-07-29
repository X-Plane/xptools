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
#include "Persistence.h"
#include "EnvParser.h"
#include "EnvWrite.h"
#include <string.h>

static	int		sWidth = 0;

#if PERSIST_VERTICES

std::vector<VertexInfo>			gVertices;

#endif

#if PERSIST_OBJECTS

std::vector<ObjectInfo>			gObjects;

#endif

#if PERSIST_PATHS

std::vector<PathInfo>			gRoads;
std::vector<PathInfo>			gTrails;
std::vector<PathInfo>			gTrains;
std::vector<PathInfo>			gLines;
std::vector<PathInfo>			gTaxiways;
std::vector<PathInfo>			gRiverVectors;

#endif

#if PERSIST_TEXTURES

std::vector<std::string>		gTextures;

#endif

#if PERSIST_VERTICES


void	AcceptDimensions(int inH, int inV)
{
	sWidth = inH;
	gVertices.resize(inH * inV);
}

void	AcceptVertex(
					int		inH,
					int		inV,
					double	inLatitude,
					double	inLongitude,
					float	inElevation,
					short	inLandUse,
					int		inCustom,
					short	inRotation,
					short	inTextureScale,
					short	inTextureXOffset,
					short	inTextureYOffset,
					short	inBodyID)
{
	VertexInfo	i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.elevation = inElevation;
	i.landUse = inLandUse;
	i.custom = inCustom;
	i.rotation = inRotation;
	i.scale = inTextureScale;
	i.xOff = inTextureXOffset;
	i.yOff = inTextureYOffset;
	i.bodyID = inBodyID;
	gVertices[inH + sWidth * inV] = i;
}					

#endif

#if PERSIST_OBJECTS
					
void	AcceptObject(
					long	inKind,
					float	inLatitude,
					float	inLongitude,
					float	inElevationHeading,
					char *	inName)
{
	ObjectInfo	i;
	i.kind = inKind;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.elevation = inElevationHeading;
	if (inKind == 8)
		i.name = std::string(inName);
	gObjects.push_back(i);
}					

#endif

#if PERSIST_PATHS					
void	AcceptRoadSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo 	i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gRoads.push_back(i);
}					

void	AcceptTrailSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo 	i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gTrails.push_back(i);
}					

void	AcceptTrainSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo 	i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gTrains.push_back(i);
}					

void	AcceptElectricSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo 	i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gLines.push_back(i);
}		

void	AcceptTaxiwaySegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo		i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gTaxiways.push_back(i);
}
		
void	AcceptRiverSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast)
{
	PathInfo		i;
	i.latitude = inLatitude;
	i.longitude = inLongitude;
	i.term = isLast;
	gRiverVectors.push_back(i);
}
				
					
#endif

#if PERSIST_TEXTURES					
void	AcceptCustomTexture(
					short	inIndex,
					char *	inName)
{
	if (gTextures.size() <= inIndex)
		gTextures.resize(inIndex + 1);
	gTextures[inIndex] = std::string(inName);
}					

#endif

#pragma mark -

#if PERSIST_VERTICES

void	GetNthVertex(
					long	inH,
					long	inV,
					double *	outLatitude,
					double *	outLongitude,
					float *	outElevation,
					short *	outLandUse,
					int *	outCustom,
					short *	outRotation,
					short *	outTextureScale,
					short *	outTextureXOffset,
					short *	outTextureYOffset,
					short *	outBodyID)
{
	long	index = inH + sWidth * inV;
	*outLatitude = gVertices[index].latitude;
	*outLongitude = gVertices[index].longitude;
	*outElevation = gVertices[index].elevation;
	*outLandUse = gVertices[index].landUse;
	*outCustom = gVertices[index].custom;
	*outRotation = gVertices[index].rotation;
	*outTextureScale = gVertices[index].scale;
	*outTextureXOffset = gVertices[index].xOff;
	*outTextureYOffset = gVertices[index].yOff;
	*outBodyID = gVertices[index].bodyID;
}					

#endif

#if PERSIST_OBJECTS
					
int		GetNthObject(
					long	inNth,
					long *	outKind,
					float *	outLatitude,
					float *	outLongitude,
					float *	outElevationHeading,
					char *	outName)
{
	if (inNth >= gObjects.size())	
		return 0;
	*outKind = gObjects[inNth].kind;
	*outLatitude = gObjects[inNth].latitude;
	*outLongitude = gObjects[inNth].longitude;
	*outElevationHeading = gObjects[inNth].elevation;
	if (gObjects[inNth].kind == 8)
		strcpy(outName, gObjects[inNth].name.c_str());
	return 1;
}			

#endif

#if PERSIST_PATHS		

int		GetNthRoadSegment(
					long	inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *	outLast)
{
	if (inNth >= gRoads.size())
		return 0;

	*outLatitude = gRoads[inNth].latitude;
	*outLongitude = gRoads[inNth].longitude;
	*outLast = gRoads[inNth].term;
	return 1;
}					

int		GetNthTrailSegment(
					long	inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *	outLast)
{
	if (inNth >= gTrails.size())
		return 0;

	*outLatitude = gTrails[inNth].latitude;
	*outLongitude = gTrails[inNth].longitude;
	*outLast = gTrails[inNth].term;
	return 1;
}					

int		GetNthTrainSegment(
					long	inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *	outLast)
{
	if (inNth >= gTrains.size())
		return 0;

	*outLatitude = gTrains[inNth].latitude;
	*outLongitude = gTrains[inNth].longitude;
	*outLast = gTrains[inNth].term;
	return 1;
}					

int		GetNthPowerSegment(
					long	inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *	outLast)
{
	if (inNth >= gLines.size())
		return 0;

	*outLatitude = gLines[inNth].latitude;
	*outLongitude = gLines[inNth].longitude;
	*outLast = gLines[inNth].term;
	return 1;
}					

int		GetNthTaxiwaySegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast)
{
	if (inNth >= gTaxiways.size())
		return 0;
		
	*outLatitude = gTaxiways[inNth].latitude;
	*outLongitude = gTaxiways[inNth].longitude;
	*outLast = gTaxiways[inNth].term;
	return 1;
}

int		GetNthRiverSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast)
{
	if (inNth >= gRiverVectors.size())
		return 0;
		
	*outLatitude = gRiverVectors[inNth].latitude;
	*outLongitude = gRiverVectors[inNth].longitude;
	*outLast = gRiverVectors[inNth].term;
	return 1;
}



#endif

#if PERSIST_TEXTURES

int		GetNthTexture(
					long	inNth,
					char *	outName)
{
	if (inNth >= gTextures.size())
		return 0;
		
	if (!gTextures[inNth].empty())
		strcpy(outName, gTextures[inNth].c_str());
	return 1;
}

#endif

void	ClearEnvData(void)
{
#if PERSIST_VERTICES
	gVertices.clear();
#endif	
#if PERSIST_OBJECTS
	gObjects.clear();
#endif
#if PERSIST_PATHS
	gRoads.clear();
	gTrails.clear();
	gTrains.clear();
	gLines.clear();
	gTaxiways.clear();
	gRiverVectors.clear();
#endif
#if PERSIST_TEXTURES
	gTextures.clear();
#endif
}

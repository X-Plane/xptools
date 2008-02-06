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

#ifndef DSF_EXPORT_H
#define DSF_EXPORT_H

#include "TclStubs.h"
#include <ac_plugin.h>


bool AC3D_NextPass(int finished_pass_index, void * inRef);
void AC3D_AcceptTerrainDef(const char * inPartialPath, void * inRef);
void AC3D_AcceptObjectDef(const char * inPartialPath, void * inRef);
void AC3D_AcceptPolygonDef(const char * inPartialPath, void * inRef);
void AC3D_AcceptNetworkDef(const char * inPartialPath, void * inRef);
void AC3D_AcceptProperty(const char * inProp, const char * inValue, void * inRef);
void AC3D_BeginPatch(
				unsigned int	inTerrainType,
				double 			inNearLOD, 
				double 			inFarLOD,
				unsigned char	inFlags,
				int				inCoordDepth,
				void *			inRef);
void AC3D_BeginPrimitive(
				int				inType,
				void *			inRef);
void AC3D_AddPatchVertex(
				double			inCoordinates[],
				void *			inRef);					
void AC3D_EndPrimitive(
				void *			inRef);					
void AC3D_EndPatch(
				void *			inRef);
void AC3D_AddObject(
				unsigned int	inObjectType,
				double			inCoordinates[2],
				double			inRotation,
				void *			inRef);
void AC3D_BeginSegment(
				unsigned int	inNetworkType,
				unsigned int	inNetworkSubtype,
				unsigned int	inStartNodeID,
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef);					
void AC3D_AddSegmentShapePoint(
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef);
void AC3D_EndSegment(
				unsigned int	inEndNodeID,
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef);					
void AC3D_BeginPolygon(
				unsigned int	inPolygonType,
				unsigned short	inParam,
				int				inCoordDepth,
				void *			inRef);					
void AC3D_BeginPolygonWinding(
				void *			inRef);					
void AC3D_AddPolygonPoint(
				double *		inCoordinates,
				void *			inRef);					
void AC3D_EndPolygonWinding(
				void *			inRef);					
void AC3D_EndPolygon(
				void *			inRef);








int 		do_dsf_save(char * fname, ACObject * obj);
ACObject *	do_dsf_load(char *filename);

#endif

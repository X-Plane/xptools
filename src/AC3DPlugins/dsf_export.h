#ifndef DSF_EXPORT_H
#define DSF_EXPORT_H

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

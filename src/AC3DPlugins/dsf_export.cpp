#include "DSFLib.h"
#include <string>

#ifdef Boolean
#undef Boolean
#endif
#include <ac_plugin.h>
#include "dsf_export.h"

#include "ac_utils.h"

using std::string;

static	vector<ACObject *>	terrain_layers;
static	ACObject *			terrain_cur_layer;
static	int					terrain_cur_primitive;
static	vector<Vertex *>	terrain_cur_coords;

static	double				lat_ref;
static	double				lon_ref;
static	double				cos_scale;
static	int					tri_count;

bool AC3D_NextPass(int finished_pass_index, void * inRef)
{
	return true;
}

void AC3D_AcceptTerrainDef(const char * inPartialPath, void * inRef)
{
	ACObject * new_obj = new_object(OBJECT_NORMAL);
	object_set_name(new_obj, (char *) inPartialPath);
	terrain_layers.push_back(new_obj);		
}

void AC3D_AcceptObjectDef(const char * inPartialPath, void * inRef)
{
}

void AC3D_AcceptPolygonDef(const char * inPartialPath, void * inRef)
{
}

void AC3D_AcceptNetworkDef(const char * inPartialPath, void * inRef)
{
}

void AC3D_AcceptProperty(const char * inProp, const char * inValue, void * inRef)
{
	if (!strcmp(inProp, "sim/west" )){		lon_ref = atof(inValue);										  }
	if (!strcmp(inProp, "sim/south")){		lat_ref = atof(inValue); cos_scale = cos(lat_ref * M_PI / 180.0); }
}

void AC3D_BeginPatch(
				unsigned int	inTerrainType,
				double 			inNearLOD, 
				double 			inFarLOD,
				unsigned char	inFlags,
				int				inCoordDepth,
				void *			inRef)
{
	terrain_cur_layer = terrain_layers[inTerrainType];
}

void AC3D_BeginPrimitive(
				int				inType,
				void *			inRef)
{
	terrain_cur_primitive = inType;
	terrain_cur_coords.clear();
}

void AC3D_AddPatchVertex(
				double			inCoordinates[],
				void *			inRef)
{
	double xyz[3];
	latlonel2xyz(inCoordinates, lat_ref, lon_ref, cos_scale, xyz);
	Point3	ac3d_p = { xyz[0], xyz[1], xyz[2] };
	// object_add_new_vertex_head would be a lot faster but - then the mesh loses connectivity and shading goes to hell. :-(  actually it's pretty borked anyway.
	terrain_cur_coords.push_back(object_add_new_vertex_reuse(terrain_cur_layer, &ac3d_p));
}

void AC3D_EndPrimitive(
				void *			inRef)
{
	int n;
	switch(terrain_cur_primitive) {
	case dsf_Tri:
		for (n = 2; n < terrain_cur_coords.size(); n+=3)
		{
			tri_count++;
			add_tri_to_obj(terrain_cur_layer, terrain_cur_coords[n-2], terrain_cur_coords[n-1], terrain_cur_coords[n]);
		}
		break;
	case dsf_TriStrip:
		for (n = 2; n < terrain_cur_coords.size(); ++n)
		{
			tri_count++;
			if (n%2)
				add_tri_to_obj(terrain_cur_layer, terrain_cur_coords[n-2], terrain_cur_coords[n  ], terrain_cur_coords[n-1]);
			else
				add_tri_to_obj(terrain_cur_layer, terrain_cur_coords[n-2], terrain_cur_coords[n-1], terrain_cur_coords[n  ]);
		}
		break;
	case dsf_TriFan:
		for (n = 2; n < terrain_cur_coords.size(); ++n)
		{
			tri_count++;
			add_tri_to_obj(terrain_cur_layer, terrain_cur_coords[0], terrain_cur_coords[n-1], terrain_cur_coords[n]);
		}
		break;
	}

	terrain_cur_coords.clear();
}

void AC3D_EndPatch(
				void *			inRef)
{
	terrain_cur_layer = NULL;
}

void AC3D_AddObject(
				unsigned int	inObjectType,
				double			inCoordinates[2],
				double			inRotation,
				void *			inRef)
{
}

void AC3D_BeginSegment(
				unsigned int	inNetworkType,
				unsigned int	inNetworkSubtype,
				unsigned int	inStartNodeID,
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef)
{
}

void AC3D_AddSegmentShapePoint(
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef)
{
}

void AC3D_EndSegment(
				unsigned int	inEndNodeID,
				double			inCoordinates[],
				bool			inCurved,
				void *			inRef)
{
}

void AC3D_BeginPolygon(
				unsigned int	inPolygonType,
				unsigned short	inParam,
				int				inCoordDepth,
				void *			inRef)
{
}

void AC3D_BeginPolygonWinding(
				void *			inRef)
{
}

void AC3D_AddPolygonPoint(
				double *		inCoordinates,
				void *			inRef)
{
}

void AC3D_EndPolygonWinding(
				void *			inRef)
{
}

void AC3D_EndPolygon(
				void *			inRef)
{
}


static DSFCallbacks_t ac3d_callbacks = {
 AC3D_NextPass, 
 AC3D_AcceptTerrainDef, 
 AC3D_AcceptObjectDef,
 AC3D_AcceptPolygonDef, 
 AC3D_AcceptNetworkDef, 
 AC3D_AcceptProperty, 
 AC3D_BeginPatch, 
 AC3D_BeginPrimitive, 
 AC3D_AddPatchVertex, 
 AC3D_EndPrimitive, 
 AC3D_EndPatch, 
 AC3D_AddObject, 
 AC3D_BeginSegment, 
 AC3D_AddSegmentShapePoint, 
 AC3D_EndSegment, 
 AC3D_BeginPolygon, 
 AC3D_BeginPolygonWinding, 
 AC3D_AddPolygonPoint, 
 AC3D_EndPolygonWinding, 
 AC3D_EndPolygon 
};

int 		do_dsf_save(char * fname, ACObject * obj)
{
	return -1;
}

ACObject *	do_dsf_load(char *filename)
{
	const int passes[] = { dsf_CmdProps + dsf_CmdDefs, dsf_CmdAll - dsf_CmdProps - dsf_CmdDefs, 0 };
	
	int n;
	int err;
	
    string	fname(filename);
    string::size_type p = fname.find_last_of("\\/");	
    string justName = (p == fname.npos) ? fname : fname.substr(p+1);
    string justPath = fname.substr(0,p+1);

    ACObject * group = new_object(OBJECT_GROUP);	
    object_set_name(group, (char*) justName.c_str());
	terrain_layers.clear();
	tri_count = 0;

	err = DSFReadFile(filename, &ac3d_callbacks, passes, NULL);
	if (err != 0) printf("Got error: %d\n", err);
	
	for (n = 0; n < terrain_layers.size(); ++n)
		object_add_child(group, terrain_layers[n]);

	object_calc_normals_force(group);

	printf("Imported %d tris.\n", tri_count);	

	terrain_layers.clear();
	tri_count = 0;
	return group;
}

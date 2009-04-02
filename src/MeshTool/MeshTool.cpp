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

#include "MapDefsCGAL.h"
#include "XESInit.h"
#include "GISTool_Globals.h"
#include "XESIO.h"
#include "DEMIO.h"
#include "DEMAlgs.h"
#include "DEMTables.h"
#include "MapAlgs.h"
#include "MeshAlgs.h"
#include "DSFBuilder.h"
#include "Zoning.h"
#include "MemFileUtils.h"
#include "MeshDefs.h"
#include "NetPlacement.h"
#include "ObjTables.h"
#include "ObjPlacement.h"
#include <CGAL/assertions.h>
#if LIN
#include <execinfo.h>
#endif

CGAL_BEGIN_NAMESPACE
/*!
 * \class
 * An overlay-traits class for computing the overlay of two arrangement whose
 * face records are extended with auxiliary data fields, of type Data1 and
 * Data2, respectively. The resulting arrangement is also assumed to be
 * templated with the face-extended DCEL, where each face stores an auxiliart
 * Res_data field.
 * The resulting data object that corresponds to the overlay of two data
 * object of type Data1 and Data2 is computed using the functor
 * Overlay_face_data.
 * Additionally, we also handle edge data using the functor Overlay_edge_data
 * Note that we do not support interaction between face and edge data
 */
template <class ArrangementA, class ArrangementB, class ArrangementR,
class OverlayEdgeData_,
class OverlayFaceData_>
class Arr_face_edge_overlay_traits :
public _Arr_default_overlay_traits<ArrangementA, ArrangementB, ArrangementR> 
{
public:
	
	typedef typename ArrangementA::Face_const_handle    Face_handle_A;
	typedef typename ArrangementB::Face_const_handle    Face_handle_B;
	typedef typename ArrangementR::Face_handle          Face_handle_R;

	typedef typename ArrangementA::Halfedge_const_handle  Halfedge_handle_A;
	typedef typename ArrangementB::Halfedge_const_handle  Halfedge_handle_B;
	typedef typename ArrangementR::Halfedge_handle        Halfedge_handle_R;	
	
	typedef OverlayEdgeData_                            Overlay_edge_data;
	typedef OverlayFaceData_                            Overlay_face_data;
	
private:
	
	Overlay_edge_data         overlay_edge_data;
	Overlay_face_data         overlay_face_data;
	
public:
	
	/*!
	 * Create an edge e that matches the overlap between e1 and e2.
	 */
	virtual void create_edge (Halfedge_handle_A e1,
							  Halfedge_handle_B e2,
							  Halfedge_handle_R e) const
	{
		e->set_data (overlay_edge_data (e1->data(), e2->data()));
		return;
	}
	
	/*!
	 * Create an edge e that matches the edge e1, contained in the face f2.
	 */
	virtual void create_edge (Halfedge_handle_A e1,
							  Face_handle_B f2,
							  Halfedge_handle_R e) const
	{
		e->set_data (e1->data());
		return;
	}
	
	/*!
	 * Create an edge e that matches the edge e2, contained in the face f1.
	 */
	virtual void create_edge (Face_handle_A f1,
							  Halfedge_handle_B e2,
							  Halfedge_handle_R e) const
	{
		e->set_data (e2->data());
		return;
	}
	
	
	/*!
	 * Create a face f that matches the overlapping region between f1 and f2.
	 */
	virtual void create_face (Face_handle_A f1,
							  Face_handle_B f2,
							  Face_handle_R f) const
	{
		// Overlay the data objects associated with f1 and f2 and store the result
		// with f.
		f->set_data (overlay_face_data (f1->data(), f2->data()));
		return;
	}
	
};
CGAL_END_NAMESPACE

struct Overlay_terrain
{
	GIS_face_data operator() (GIS_face_data a, GIS_face_data b) const
	{
		GIS_face_data r;
		//fprintf(stderr, "%d-%d ", a.mTerrainType, b.mTerrainType);
		// Our overlay comes from the RHS, but it might be a hole (in which case mTerrainType will be 0)
		if (b.mTerrainType != 0 ) {
			r.mTerrainType = b.mTerrainType;
			return r;
		}
		if (a.mTerrainType != 0 ) {
			r.mTerrainType = a.mTerrainType;
			return r;
		}
		return r;
	}
};

struct Overlay_network
{
	GIS_halfedge_data operator() (GIS_halfedge_data a, GIS_halfedge_data b) const
	{
		GIS_halfedge_data r;
		GISNetworkSegmentVector::iterator i;
		if (b.mSegments.empty())
			for (i = a.mSegments.begin(); i != a.mSegments.end(); ++i)
				r.mSegments.push_back(*i);
		for (i = b.mSegments.begin(); i != b.mSegments.end(); ++i)
			r.mSegments.push_back(*i);
		return r;
	}
};

typedef CGAL::Arr_face_edge_overlay_traits<Pmwx, Pmwx, Pmwx, Overlay_network, Overlay_terrain>        Overlay_traits;



void	CGALFailure(
        const char* what, const char* expr, const char* file, int line, const char* msg)
{
#if LIN
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	for (i = 0; i < frames; ++i) {
		fprintf(stdout,"%s\n", strs[i]);
	}
	free(strs);
#else
	fprintf(stderr,"callstack for mingw builds not yet implemented\n");
#endif	
	fprintf(stdout,"Terminating due to a CGAL exception.\n");
	fprintf(stdout,"****************************************************************************\n");
	fprintf(stdout,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	fprintf(stdout,"****************************************************************************\n");

	fprintf(stderr,"ERROR  (%d,%d) %s: %s (%s:%d).%s\n", gMapWest, gMapSouth, what, expr, file, line, msg ? msg : "");
	exit(1);
}




int	main(int argc, char * argv[])
{
	Polygon_set_2 *the_map = new Polygon_set_2();
	
	if(argc == 2 && !strcmp(argv[1],"--version"))
	{
		fprintf(stdout, "MeshTool version 1.0b1, compiled " __DATE__ ".\n");
		exit(0);
	}

	if(argc == 2 && !strcmp(argv[1],"--auto_config"))
	{
		exit(0);
	}

	try {

		// Set CGAL to throw an exception rather than just
		// call exit!
		CGAL::set_error_handler(CGALFailure);

		XESInit();

		if(argc != 6)
		{
			fprintf(stderr, "USAGE: MeshTool <script.txt> <file.xes> <file.hgt> <dir_base> <file.dsf>\n");
			exit(1);
		}
		
		fprintf(stderr,"Open XES\n");
		MFMemFile *	xes = MemFile_Open(argv[2]);
		if(xes == NULL)
		{
			fprintf(stderr,"ERROR: could not read:%s\n", argv[2]);
			exit(1);
		}

		{
			Pmwx		dummy_vec;
			CDT			dummy_mesh;
			AptVector	dummy_apt;
			fprintf(stderr,"Read XES: %p %p %p %p %p\n", xes, &dummy_vec, &dummy_mesh, &gDem, &dummy_apt);

			ReadXESFile(xes, &dummy_vec, &dummy_mesh, &gDem, &dummy_apt, ConsoleProgressFunc);
		}
		MemFile_Close(xes);
		
		if(strstr(argv[3],".hgt")) {
			fprintf(stderr,"Read HGT\n");		
		if (!ReadRawHGT(gDem[dem_Elevation], argv[3]))
		{
			fprintf(stderr,"Could not read HGT file: %s\n", argv[3]);
			exit(1);
		}
		}
		if(strstr(argv[3],".tif")) {
			fprintf(stderr,"Read GeoTIFF\n");		
			if (!ExtractGeoTiff(gDem[dem_Elevation], argv[3]))
			{
				fprintf(stderr,"Could not read GeoTIFF file: %s\n", argv[3]);
				exit(1);
			}
		}
		
		fprintf(stderr,"Read Script\n");		
		FILE * script = fopen(argv[1], "r");
		if(!script)
		{
			fprintf(stderr, "ERROR: could not open %s\n", argv[1]);
			exit(1);
		}
		char buf[1024];

		int num_cus_terrains = 0;
		Polygon_set_2::Polygon_2   	ring, outer, the_hole;
		std::vector<Polygon_set_2::Polygon_2> holes;
		std::vector<Polygon_set_2::Polygon_with_holes_2> layer;
		std::vector<X_monotone_curve_2> net;
		int				terrain_type;
		double			coords[4];
		char			cus_ter[256];
		char			typ[256];
		int last_ter = -1;
		int proj_pt;
		tex_proj_info pinfo;
		int use_wat;
		int zlimit=0,zmin=30000,zmax=-2000;
		while (fgets(buf, sizeof(buf), script))
		{
			if(sscanf(buf,"DEFINE_CUSTOM_TERRAIN %d %s",&use_wat, cus_ter)==2)
			{
				proj_pt = 0;
				if(LookupToken(cus_ter) != -1)
				{
					fprintf(stderr,"ERROR: terrain '%s' already Defined or name is reserved.\n", cus_ter);
					exit(1);
				}
				int tt = NewToken(cus_ter);
				last_ter = tt;
				NaturalTerrainInfo_t nt = { 0 };
				nt.terrain = tt;
				nt.landuse = NO_VALUE;
				nt.climate = NO_VALUE;
				nt.elev_min = DEM_NO_DATA;;
				nt.elev_max = DEM_NO_DATA;;
				nt.slope_min = DEM_NO_DATA;;
				nt.slope_max = DEM_NO_DATA;;
				nt.temp_min = DEM_NO_DATA;;
				nt.temp_max = DEM_NO_DATA;;
				nt.temp_rng_min = DEM_NO_DATA;;
				nt.temp_rng_max = DEM_NO_DATA;;
				nt.rain_min = DEM_NO_DATA;;
				nt.rain_max = DEM_NO_DATA;;
				nt.near_water = 0;
				nt.slope_heading_min = DEM_NO_DATA;;
				nt.slope_heading_max = DEM_NO_DATA;;
				nt.rel_elev_min = DEM_NO_DATA;;
				nt.rel_elev_max = DEM_NO_DATA;;
				nt.elev_range_min = DEM_NO_DATA;;
				nt.elev_range_max = DEM_NO_DATA;;
				nt.urban_density_min = DEM_NO_DATA;;
				nt.urban_density_max = DEM_NO_DATA;;
				nt.urban_radial_min = DEM_NO_DATA;;
				nt.urban_radial_max = DEM_NO_DATA;;
				nt.urban_trans_min = DEM_NO_DATA;;
				nt.urban_trans_max = DEM_NO_DATA;
				nt.urban_square = 0;
				nt.lat_min = DEM_NO_DATA;
				nt.lat_max = DEM_NO_DATA;
				nt.variant = 0;
				nt.related = -1;
				nt.name = tt;
				nt.layer = 0;
				nt.xon_dist = 0;
				nt.xon_hack = 0;
				nt.custom_ter = use_wat ? tex_custom_water : tex_custom;

				int rn = gNaturalTerrainTable.size();
				gNaturalTerrainTable.insert(gNaturalTerrainTable.begin()+(num_cus_terrains++),nt);
			}
			if(sscanf(buf,"PROJECT_POINT %lf %lf %lf %lf",coords,coords+1,coords+2,coords+3)==4)
			{
				if(last_ter==-1)
				{
					fprintf(stderr,"PROJECT_POINT not allowde until custom terrain defined.\n");
					exit(1);
				}
				pinfo.corners[proj_pt] = Point2(coords[0],coords[1]);
				pinfo.ST[proj_pt] = Point2(coords[2],coords[3]);
				proj_pt++;
				if(proj_pt==4)
				{
					gTexProj[last_ter] = pinfo;
					last_ter = -1;
				}
			}
			if(strncmp(buf,"BEGIN_LAYER",strlen("BEGIN_LAYER"))==0)
			{
				layer.clear();
				ring.clear();
				the_hole.clear();
				holes.clear();
			}

			if(sscanf(buf,"CUSTOM_POLY %s",cus_ter)==1)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = LookupToken(cus_ter);
				zmin=30000,zmax=-2000;
				if(terrain_type == -1)
				{
					fprintf(stderr,"ERROR: cannot find custom terrain type '%s'\n", cus_ter);
					exit(1);
				}
			}

			if(strncmp(buf,"LAND_POLY",strlen("LAND_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Natural;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"WATER_POLY",strlen("WATER_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Water;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"APT_POLY",strlen("APT_POLY"))==0)
			{
				ring.clear();
				the_hole.clear();
				holes.clear();
				terrain_type = terrain_Airport;
				zmin=30000,zmax=-2000;
			}
			if(strncmp(buf,"BEGIN_HOLE",strlen("BEGIN_HOLE"))==0)
			{
				the_hole.clear();
			}
			if(strncmp(buf,"END_HOLE",strlen("END_HOLE"))==0)
			{
				if (the_hole.is_simple()) {
					if (the_hole.orientation() != CGAL::CLOCKWISE)
						the_hole.reverse_orientation();
					holes.push_back(the_hole);
					fprintf(stderr,"_");
				} else {
					fprintf(stderr,"?");
					/*
					 fprintf(stderr,"\nPoly is: ");
					 Polygon_2::Vertex_iterator vit;
					 for (vit=ring.vertices_begin(); vit!=ring.vertices_end(); ++vit) {
					 fprintf(stderr,"(%lf, %lf) ", CGAL::to_double(vit->x()), CGAL::to_double(vit->y()));
					 }
					 fprintf(stderr,"\n");
					 */
				}
			}
			if(strncmp(buf,"END_POLY",strlen("END_POLY"))==0)
			{
				bool zyes = true;
				if (zlimit != 0) {
					Bbox_2 box = ring.bbox();
					double DEG_TO_NM_LON = DEG_TO_NM_LAT * cos(CGAL::to_double(box.ymin()) * DEG_TO_RAD);
					double rhs = (pow((box.xmax()-box.xmin())*DEG_TO_NM_LON*NM_TO_MTR,2) + pow((box.ymax()-box.ymin())*DEG_TO_NM_LAT*NM_TO_MTR,2));
					double lhs = pow((double)(zmax-zmin),2);
					//fprintf(stderr," %9.0lf,%9.0lf ", rhs, lhs);
					if (zlimit*lhs > rhs) zyes = false;
				}
				if (zyes) {
					if (ring.is_simple()) {
						if (ring.orientation() == CGAL::CLOCKWISE)
							ring.reverse_orientation();
						Polygon_set_2::Polygon_with_holes_2 P(ring, holes.begin(), holes.end());
						layer.push_back(P);
						fprintf(stderr,"-");
					} else {
						fprintf(stderr,"!");
						/*
						fprintf(stderr,"\nPoly is: ");
						Polygon_2::Vertex_iterator vit;
						for (vit=ring.vertices_begin(); vit!=ring.vertices_end(); ++vit) {
							fprintf(stderr,"(%lf, %lf) ", CGAL::to_double(vit->x()), CGAL::to_double(vit->y()));
						}
						fprintf(stderr,"\n");
						exit(1);
						 */
					}
				} else {
					fprintf(stderr,"x");
				}
				
					
				#if !DEV 
					#error this is taken from the END_LAYER command.
				#endif
				fprintf(stderr,",");
				Overlay_traits         overlay_traits;
				Polygon_set_2	*temp = new Polygon_set_2(), *temp1 = new Polygon_set_2();
				fprintf(stderr,",");
				if (!layer.empty()) {
					temp->join(layer.begin(), layer.end());
					fprintf(stderr,",");
					
					int nfaces = 0, ntouched = 0;
					Pmwx::Face_iterator ring_face;
					for(ring_face = temp->arrangement().faces_begin(); ring_face != temp->arrangement().faces_end(); ++ring_face, ++nfaces) {
						if (ring_face->contained()) {
							ring_face->data().mTerrainType = terrain_type;
							ntouched++;
						}
					}
					fprintf(stderr,"%d %d %s,", nfaces, ntouched, FetchTokenString(terrain_type));
					
					CGAL::overlay (the_map->arrangement(), temp->arrangement(), temp1->arrangement(), overlay_traits);
					delete the_map;
					the_map = temp1;
				}
				#if !DEV
					#error this is taken from the BEGIN_LAYER command
				#endif
				layer.clear();
				ring.clear();
				the_hole.clear();
				holes.clear();
			}
			if(strncmp(buf,"END_LAYER",strlen("END_LAYER"))==0)
			{
				fprintf(stderr,",");
				Overlay_traits         overlay_traits;
				Polygon_set_2	*temp = new Polygon_set_2(), *temp1 = new Polygon_set_2();
				fprintf(stderr,",");
				if (!layer.empty()) {
					temp->join(layer.begin(), layer.end());
					fprintf(stderr,",");
					
					int nfaces = 0, ntouched = 0;
					Pmwx::Face_iterator ring_face;
					for(ring_face = temp->arrangement().faces_begin(); ring_face != temp->arrangement().faces_end(); ++ring_face, ++nfaces) {
						if (ring_face->contained()) {
							ring_face->data().mTerrainType = terrain_type;
							ntouched++;
						}
					}
					fprintf(stderr,"%d %d %s,", nfaces, ntouched, FetchTokenString(terrain_type));
					
					CGAL::overlay (the_map->arrangement(), temp->arrangement(), temp1->arrangement(), overlay_traits);
					delete the_map;
					the_map = temp1;
				}
			}
			if (sscanf(buf, "POLYGON_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				//fprintf(stderr,".");
				ring.push_back(Point_2(coords[0],coords[1]));
				if (zlimit != 0) {
					int z = gDem[dem_Elevation].xy_nearest(coords[0],coords[1]);
					if (z<zmin) zmin=z;
					if (z>zmax) zmax=z;
				}
			}
			if (sscanf(buf, "HOLE_POINT %lf %lf", &coords[0], &coords[1])==2)
			{
				//fprintf(stderr,".");
				the_hole.push_back(Point_2(coords[0],coords[1]));
			}
			if (sscanf(buf, "ZLIMIT %d", &zlimit)==1)
			{
				fprintf(stderr,"zlimit %d\n", zlimit);
				// store limit^2
				zlimit *= zlimit;
			}
			if(sscanf(buf,"BEGIN_NET %s",typ)==1)
			{
				net.clear();
			}
			if (sscanf(buf, "NET_SEG %lf %lf %lf %lf", &coords[0], &coords[1], &coords[2], &coords[3])==4)
			{
				fprintf(stderr,"=");
				net.push_back(X_monotone_curve_2(Segment_2(Point_2(coords[0],coords[1]), Point_2(coords[2],coords[3])),0));
			}
			if(strncmp(buf,"END_NET",strlen("END_NET"))==0)
			{
				int net_type = LookupToken(typ);
				struct	GISNetworkSegment_t segdata = { net_type, net_type, 0.0, 0.0 };
				fprintf(stderr,",");
				Overlay_traits         overlay_traits;
				Polygon_set_2	*temp = new Polygon_set_2(), *temp1 = new Polygon_set_2();
				fprintf(stderr,",");
				if (!net.empty()) {
					insert_x_monotone_curves(temp->arrangement(), net.begin(), net.end());
					fprintf(stderr,",");
					
					int nfaces = 0, ntouched = 0;
					Pmwx::Edge_iterator the_edge;
					for (the_edge = temp->arrangement().edges_begin(); the_edge != temp->arrangement().edges_end(); ++the_edge, ++nfaces) {
						the_edge->data().mSegments.push_back(GISNetworkSegment_t(segdata));
					}
					fprintf(stderr,"%d %s,", nfaces, FetchTokenString(net_type));
					
					CGAL::overlay (the_map->arrangement(), temp->arrangement(), temp1->arrangement(), overlay_traits);
					delete the_map;
					the_map = temp1;
				}
			}
		}
		fclose(script);

		Pmwx gMap = the_map->arrangement();

		{
			fprintf(stderr,"\n");
			int nfaces = 0, nwet = 0, nzero = 0;
			Pmwx::Face_iterator fit;
			for (fit = gMap.faces_begin(); fit != gMap.faces_end(); ++fit, ++nfaces) {
				if (fit->data().IsWater())
					nwet++;
				if (fit->data().mTerrainType == 0)
					nzero++;
				//fprintf(stderr, "%d ", fit->data().mTerrainType);
			}
			fprintf(stderr,"Zero: %d Wet: %d Faces: %d Land ratio: %f\n", nzero, nwet, nfaces, ((double) (nfaces - nwet))/((double) nfaces));
		}
		
		
//		fprintf(stderr,"Dominance\n");
//		// assuming no dominant flags are set at this point
//		Pmwx::Halfedge_iterator hit;
//		for (hit = gMap.halfedges_begin(); hit != gMap.halfedges_end(); ++hit) {
//			if (!hit->twin()->data().mDominant)
//				hit->data().mDominant = true;
//		}
		
		gNaturalTerrainIndex.clear();
		for(int rn = 0; rn < gNaturalTerrainTable.size(); ++rn)
		if (gNaturalTerrainIndex.count(gNaturalTerrainTable[rn].name) == 0)
			gNaturalTerrainIndex[gNaturalTerrainTable[rn].name] = rn;
		
		fprintf(stderr,"Now to simplify\n");
		// -simplify
		//SimplifyMap(gMap, true, ConsoleProgressFunc);

		//-calcslope
		CalcSlopeParams(gDem, true, ConsoleProgressFunc);

		// -upsample
		UpsampleEnvironmentalParams(gDem, ConsoleProgressFunc);

		// -derivedems
		DeriveDEMs(gMap, gDem,gApts, gAptIndex, ConsoleProgressFunc);

		// -zoning
		ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts,ConsoleProgressFunc);

		// -calcmesh
		TriangulateMesh(gMap, gTriangulationHi, gDem, argv[4], ConsoleProgressFunc);		
		
		CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],ConsoleProgressFunc);
		
		// -assignterrain
		AssignLandusesToMesh(gDem,gTriangulationHi,argv[4],ConsoleProgressFunc);

		#if DEV
		for (CDT::Finite_faces_iterator tri = gTriangulationHi.finite_faces_begin(); tri != gTriangulationHi.finite_faces_end(); ++tri)
		if (tri->info().terrain == terrain_Water)
		{
			DebugAssert(tri->info().terrain == terrain_Water);
		}
		else
		{
			DebugAssert(tri->info().terrain != terrain_Water);
			DebugAssert(tri->info().terrain != -1);
		}
		#endif	
		
		printf("Instantiating objects...\n");
		vector<PreinsetFace>	insets;
		
		set<int>				the_types;
		GetObjTerrainTypes		(the_types);
		
		printf("%d obj types\n", the_types.size());
		for (set<int>::iterator i = the_types.begin(); i != the_types.end(); ++i)
			printf("%s ", FetchTokenString(*i));
		
		Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
		GenerateInsets(gMap, gTriangulationHi, lim, the_types, true, insets, ConsoleProgressFunc);
		
		InstantiateGTPolygonAll(insets, gDem, gTriangulationHi, ConsoleProgressFunc);
		DumpPlacementCounts();
	
		
		// -exportDSF
		BuildDSF(argv[5], argv[5], gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, ConsoleProgressFunc);			

	} catch (exception& e) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR (%d,%d): Caught unknown exception %s.  Exiting.\n", gMapWest, gMapSouth, e.what());
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR (%d,%d): Caught unknown exception %s.  Exiting.\n", gMapWest, gMapSouth, e.what());
		exit(0);
	} catch (...) {
		fprintf(stdout,"****************************************************************************\n");
		fprintf(stdout,"ERROR (%d,%d): Caught unknown exception.  Exiting.\n", gMapWest, gMapSouth);
		fprintf(stdout,"****************************************************************************\n");

		fprintf(stderr,"ERROR (%d,%d): Caught unknown exception.  Exiting.\n", gMapWest, gMapSouth);
		exit(0);
	}
}

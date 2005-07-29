#include "MeshIO.h"
#include "MeshDefs.h"
#include "IODefs.h"
#include "SimpleIO.h"
#include "XChunkyFileUtils.h"

const int kMeshControlID = 'mesh';
const int kMeshData1ID = 'dat1';

void WriteMesh(FILE * fi, CDT& mesh, int inAtomID, ProgressFunc func)
{
	StAtomWriter	meshAtom(fi, inAtomID);

	// These maps assign numbers to all entities in the mesh.
	map<TDS::Vertex_handle, int>	V;
	map<TDS::Face_handle, int>		F;
	TDS::Face_iterator				ib;
	TDS::Vertex_iterator			vit;

	int 							vnum = 0;
	int								fnum = 0;
	
	int j;

	int ctr = 0;
	int tot = mesh.tds().number_of_vertices() * 2 + mesh.tds().number_of_full_dim_faces() * 4;
	int step = tot / 150;
	
	PROGRESS_START(func, 0, 1, "Writing terrain mesh...")

	{	
		// TDS CONTROL ATOM
		// This atom contains the basic mesh structure.
	
		StAtomWriter 	mainAtom(fi, kMeshControlID);
		FileWriter		writer1(fi);
		
		// HEADER - number of vertices, number of full DIM faces,
		// dimension of mesh.

		writer1.WriteInt(mesh.tds().number_of_vertices());
		writer1.WriteInt(mesh.tds().number_of_full_dim_faces());
		writer1.WriteInt(mesh.tds().dimension());
		
		// Write out vertices
		V[mesh.infinite_vertex()] = vnum++;
		
		for (vit = mesh.tds().vertices_begin(); vit != mesh.tds().vertices_end(); ++vit, ++ctr)
		if (!(vit == mesh.infinite_vertex()))
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)
			V[vit] = vnum++;
		}
		
		// Write out faces
		int dim = (mesh.dimension() == -1 ? 1 :  mesh.dimension() + 1);
		for(ib = mesh.tds().face_iterator_base_begin(); ib != mesh.tds().face_iterator_base_end(); ++ib, ++ctr) 
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)
			F[ib] = fnum++;
			for(int j = 0; j < dim ; ++j) 
			{
				writer1.WriteInt(V[ib->vertex(j)]);
			}
		}

		// Write out neighbors		
		for(ib = mesh.tds().face_iterator_base_begin(); ib != mesh.tds().face_iterator_base_end(); ++ib, ++ctr) 
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)
			for(j = 0; j < mesh.tds().dimension()+1; ++j)
				writer1.WriteInt(F[ib->neighbor(j)]);
		}
		
		// Write out constraints
		for(ib = mesh.tds().face_iterator_base_begin(); ib != mesh.tds().face_iterator_base_end(); ++ib, ++ctr) 
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)		
			for (j = 0; j < 3; ++j)
				writer1.WriteInt(ib->is_constrained(j) ? 1 : 0);
		}
	}

	// Reverse the vertex and face tables...
	vector<TDS::Vertex_handle>	VT(vnum);
	vector<TDS::Face_handle>	FT(fnum);
	for (map<TDS::Vertex_handle, int>::iterator viter = V.begin(); viter != V.end(); ++viter)
		VT[viter->second] = viter->first;
	for (map<TDS::Face_handle, int>::iterator fiter = F.begin(); fiter != F.end(); ++fiter)
		FT[fiter->second] = fiter->first;
		
		
	{
		StAtomWriter	data1(fi, kMeshData1ID);
		FileWriter		writer2(fi);
		
		// Write out per-vertex info.		
		for (j = 0; j < vnum; ++j, ++ctr)
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)

			writer2.WriteDouble(VT[j]->point().x());
			writer2.WriteDouble(VT[j]->point().y());
			writer2.WriteDouble(VT[j]->info().height);
			writer2.WriteDouble(VT[j]->info().wave_height);
			writer2.WriteFloat(VT[j]->info().normal[0]);
			writer2.WriteFloat(VT[j]->info().normal[1]);
			writer2.WriteFloat(VT[j]->info().normal[2]);
			
			writer2.WriteInt(VT[j]->info().border_blend.size());
			for (hash_map<int,float>::iterator bb = VT[j]->info().border_blend.begin();
				bb != VT[j]->info().border_blend.end(); ++bb)
			{
				writer2.WriteInt(bb->first);
				writer2.WriteFloat(bb->second);
			}
		}
		// Write out per-face info.
		for (j = 0; j < fnum; ++j, ++ctr)
		{
			PROGRESS_CHECK(func, 0, 1, "Writing terrain mesh...", ctr, tot, step)
		
			writer2.WriteInt(FT[j]->info().terrain_general);
			writer2.WriteInt(FT[j]->info().terrain_specific);
			writer2.WriteInt(FT[j]->info().flag);
			writer2.WriteFloat(FT[j]->info().normal[0]);
			writer2.WriteFloat(FT[j]->info().normal[1]);
			writer2.WriteFloat(FT[j]->info().normal[2]);
			writer2.WriteInt(FT[j]->info().terrain_border.size());
			for (set<int>::iterator tb = FT[j]->info().terrain_border.begin();
				tb != FT[j]->info().terrain_border.end(); ++tb)
			{
				writer2.WriteInt(*tb);
			}
		}
	}
	PROGRESS_DONE(func, 0, 1, "Writing terrain mesh...")
}
		
void ReadMesh(XAtomContainer& container, CDT& mesh, int atomID, const TokenConversionMap& conv, ProgressFunc func)
{
	XAtom			meAtom, ctrlAtom, data1Atom;
	XAtomContainer	meContainer, ctrlContainer, data1Container;
	
	if (!container.GetNthAtomOfID(atomID, 0, meAtom)) return;
	meAtom.GetContents(meContainer);
	
	if (!meContainer.GetNthAtomOfID(kMeshControlID, 0, ctrlAtom)) return;
	ctrlAtom.GetContents(ctrlContainer);

	if (!meContainer.GetNthAtomOfID(kMeshData1ID, 0, data1Atom)) return;
	data1Atom.GetContents(data1Container);
	
	MemFileReader	readCtrl(ctrlContainer.begin, ctrlContainer.end);
	MemFileReader	readData1(data1Container.begin, data1Container.end);

	if (mesh.tds().number_of_vertices() != 0)    mesh.tds().clear();
  
	int n, m, d;	// number of verts, faces, dimension
	int i, j;
	readCtrl.ReadInt(n);
	readCtrl.ReadInt(m);
	readCtrl.ReadInt(d);
	
	if (n == 0) return;

	int ctr = 0;
	int tot = n * 2 + m * 4;
	int step = tot / 150;

	PROGRESS_START(func, 0, 1, "Reading mesh...")

	mesh.tds().set_dimension(d);

	std::vector<TDS::Vertex_handle > V(n);
	std::vector<TDS::Face_handle> 	F(m);

	// Create vertices
	for (i = 0; i < n; ++i, ++ctr)
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)	
		V[i] = mesh.tds().create_vertex();
	}

	// Create faces
	int index;
	int dim = (mesh.tds().dimension() == -1 ? 1 :  mesh.tds().dimension() + 1);

	for(i = 0; i < m; ++i, ++ctr) 
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)	
		F[i] = mesh.tds().create_face() ;
		for(j = 0; j < dim ; ++j)
		{
			readCtrl.ReadInt(index);
			F[i]->set_vertex(j, V[index]);
			V[index]->set_face(F[i]);
		}
	}

	// Setting the neighbor pointers 
	for(i = 0; i < m; ++i, ++ctr) 
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)		
		for(j = 0; j < mesh.tds().dimension()+1; ++j)
		{
			readCtrl.ReadInt(index);
			F[i]->set_neighbor(j, F[index]);
		}
	}
	
	// Read contraints;
	for(i = 0; i < m; ++i, ++ctr)
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)		 
		for(j = 0; j < 3; ++j)
		{
			readCtrl.ReadInt(index);
			F[i]->set_constraint(j, index != 0);
		}
	}
	/////////////////////////////////////////////////////

	// Per Vertex and Per face data
		
	// Write out per-vertex info.		
	for (j = 0; j < n; ++j, ++ctr)
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)
	
		double x, y;
		MeshVertexInfo	vi;
		readData1.ReadDouble(x);
		readData1.ReadDouble(y);
		readData1.ReadDouble(vi.height);
		readData1.ReadDouble(vi.wave_height);
		readData1.ReadFloat(vi.normal[0]);
		readData1.ReadFloat(vi.normal[1]);
		readData1.ReadFloat(vi.normal[2]);
#if DEV		
		if (j != 0)
		{
		DebugAssert(vi.normal[0] > -1.1);
		DebugAssert(vi.normal[0] <  1.1);
		DebugAssert(vi.normal[1] > -1.1);
		DebugAssert(vi.normal[1] <  1.1);
		DebugAssert(vi.normal[2] > -1.1);
		DebugAssert(vi.normal[2] <  1.1);
		}
#endif		
		
		readData1.ReadInt(index);
		while (index--)
		{
			int btype;
			float blev;
			readData1.ReadInt(btype);
			readData1.ReadFloat(blev);
			btype = conv[btype];
			vi.border_blend[btype] = blev;
		}
		
		V[j]->set_point(CDT::Point(x,y));
		V[j]->info() = vi;
		
	}
	// Write out per-face info.
	for (j = 0; j < m; ++j, ++ctr)
	{
		PROGRESS_CHECK(func, 0, 1, "Reading mesh...", ctr, tot, step)
	
		MeshFaceInfo	fi;
		readData1.ReadInt(fi.terrain_general);
		readData1.ReadInt(fi.terrain_specific);
		readData1.ReadInt(fi.flag);
		readData1.ReadFloat(fi.normal[0]);
		readData1.ReadFloat(fi.normal[1]);
		readData1.ReadFloat(fi.normal[2]);

#if DEV
		if (F[j]->vertex(0) != V[0] &&
			F[j]->vertex(1) != V[0] &&
			F[j]->vertex(2) != V[0])
		DebugAssert(fi.normal[0] > -1.1);
		DebugAssert(fi.normal[0] <  1.1);
		DebugAssert(fi.normal[1] > -1.1);
		DebugAssert(fi.normal[1] <  1.1);
		DebugAssert(fi.normal[2] > -1.1);
		DebugAssert(fi.normal[2] <  1.1);
#endif		
		fi.terrain_general = conv[fi.terrain_general];
		fi.terrain_specific = conv[fi.terrain_specific];
		
		readData1.ReadInt(index);
		while (index--)
		{
			int btp;
			readData1.ReadInt(btp);
			btp = conv[btp];
			fi.terrain_border.insert(btp);
		}
		
		F[j]->info() = fi;
	}


  	mesh.set_infinite_vertex(V[0]);
	PROGRESS_DONE(func, 0, 1, "Reading mesh...")
}

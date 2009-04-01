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
#include "MapIO.h"
#include <CGAL/iterator.h>
#include "ProgressUtils.h"
#include "XChunkyFileUtils.h"
#include "SimpleIO.h"
#include "AssertUtils.h"
#include "MapAlgs.h"

#include <CGAL/Arr_accessor.h>

// CGAL 3.3.1 has a data structure for a hole...I am trying to figure out if we need it...had code that APPEARED
// to work without it...this controls whether half-edges link to hole structures.
#define USE_HOLES 1

// Rebuidl map
#define REBUILD_MAP 0

// Ratio of operations to an update of the progress bar.
#define	PROGRESS_RATIO	5000

typedef	CGAL::Inverse_index<Pmwx::Halfedge_const_iterator>		H_Index;
typedef	CGAL::Inverse_index<Pmwx::Vertex_const_iterator>		V_Index;

const int kMainMapID = 'MAPi';
const int kFaceData1 = 'Fac1';
const int kEdgeData1 = 'Edg1';
const int kVertData1 = 'Ver1';

int	CountCirculator(Pmwx::Ccb_halfedge_const_circulator circ)
{
	Pmwx::Ccb_halfedge_const_circulator stop = circ;
	int n = 0;
	do {
		++n;
		if (circ->face() != stop->face())
			printf("Strange circulator error.\n");
		++circ;
	} while (stop != circ);
	return n;
}


int	CountCirculator(Halfedge_handle circ)
{
	Halfedge_handle stop = circ;
	int n = 0;
	do {
		++n;
		if (circ->face() != stop->face())
			printf("Strange circulator error.\n");
		circ = circ->next();
	} while (stop != circ);
	return n;
}

template <class	T, class F>
void WriteVector(IOWriter& writer, const T& v, F func)
{
	writer.WriteInt(v.size());
	for (typename T::const_iterator i = v.begin(); i != v.end(); ++i)
	{
		func(writer, *i);
	}
}

template <class T, class F>
void ReadVector(IOReader& reader, T& v, F func, const TokenConversionMap& c)
{
	int counter;
	v.clear();
	reader.ReadInt(counter);
	while (counter--)
	{
		typename T::value_type item;
		func(reader, item, c);
		v.push_back(item);
	}
}

void WriteNetworkSegment(IOWriter& inWriter, const GISNetworkSegment_t& i)
{
	inWriter.WriteInt(i.mFeatType);
	inWriter.WriteInt(i.mRepType);
	inWriter.WriteDouble(i.mSourceHeight);
	inWriter.WriteDouble(i.mTargetHeight);
}

void ReadNetworkSegment(IOReader& inReader, GISNetworkSegment_t& seg, const TokenConversionMap& c)
{
	inReader.ReadInt(seg.mFeatType);
	inReader.ReadInt(seg.mRepType);
	seg.mFeatType = c[seg.mFeatType];
	seg.mRepType = c[seg.mRepType];
	inReader.ReadDouble(seg.mSourceHeight);
	inReader.ReadDouble(seg.mTargetHeight);
}

void WriteParamMap				(IOWriter& inWriter, const GISParamMap& m)
{
	inWriter.WriteInt(m.size());
	for (GISParamMap::const_iterator i = m.begin(); i != m.end(); ++i)
	{
		DebugAssert(i->first >= 0 && i->first < gTokens.size());
		inWriter.WriteInt(i->first);
		inWriter.WriteDouble(i->second);
	}
}

void ReadParamMap				(IOReader& inReader, GISParamMap& m, const TokenConversionMap& c)
{
	int counter;
	m.clear();
	inReader.ReadInt(counter);
	while(counter--)
	{
		int		e;
		double	v;
		inReader.ReadInt(e);
		DebugAssert(e >= 0 && e < c.size());
		e = c[e];
		inReader.ReadDouble(v);
		m.insert(GISParamMap::value_type(e,v));
	}
}

void	WriteObjPlacement(IOWriter& inWriter, const GISObjPlacement_t& i)
{
	inWriter.WriteInt(i.mRepType);
	inWriter.WriteDouble(CGAL::to_double(i.mLocation.x()));
	inWriter.WriteDouble(CGAL::to_double(i.mLocation.y()));
	inWriter.WriteDouble(i.mHeading);
	inWriter.WriteInt(i.mDerived ? 1 : 0);
}

void ReadObjPlacement(IOReader& inReader, GISObjPlacement_t& p, const TokenConversionMap& c)
{
	double	x,y;
	inReader.ReadInt(p.mRepType);
	p.mRepType = c[p.mRepType];
	inReader.ReadDouble(x);
	inReader.ReadDouble(y);
	p.mLocation = Point_2(x, y);
	inReader.ReadDouble(p.mHeading);
	int derived;
	inReader.ReadInt(derived);
	p.mDerived = (derived != 0);
}

void WritePolyObjPlacement(IOWriter& inWriter, const GISPolyObjPlacement_t& i)
{
	inWriter.WriteInt(i.mRepType);
	inWriter.WriteInt(1 + distance(i.mShape.holes_begin(),i.mShape.holes_end()));

	inWriter.WriteInt(i.mShape.outer_boundary().size());
	for (Polygon_2::Vertex_iterator vv = i.mShape.outer_boundary().vertices_begin(); vv != i.mShape.outer_boundary().vertices_end(); ++vv)
	{
		inWriter.WriteDouble(CGAL::to_double(vv->x()));
		inWriter.WriteDouble(CGAL::to_double(vv->y()));
	}

	for(Polygon_with_holes_2::Hole_const_iterator h = i.mShape.holes_begin(); h != i.mShape.holes_end(); ++h)
	{
		inWriter.WriteInt(h->size());
		for (Polygon_2::Vertex_iterator vv = h->vertices_begin(); vv != h->vertices_end(); ++vv)
		{
			inWriter.WriteDouble(CGAL::to_double(vv->x()));
			inWriter.WriteDouble(CGAL::to_double(vv->y()));
		}
	}
	
	inWriter.WriteDouble(i.mHeight);
	inWriter.WriteInt(i.mDerived ? 1 : 0);
}

void ReadPolyObjPlacement(IOReader& inReader, GISPolyObjPlacement_t& obj, const TokenConversionMap& c)
{
	inReader.ReadInt(obj.mRepType);
	obj.mRepType = c[obj.mRepType];
	int	ptcount, rcount;
	inReader.ReadInt(rcount);
	DebugAssert(rcount >= 1);
	--rcount;
	
	Polygon_2	ring;

	inReader.ReadInt(ptcount);
	while(ptcount--)
	{
		double	x,y;
		inReader.ReadDouble(x);
		inReader.ReadDouble(y);
		ring.push_back(Point_2((x), (y)));
	}
	
	vector<Polygon_2>	holes;

	while(rcount--)
	{
		holes.push_back(Polygon_2());
		inReader.ReadInt(ptcount);
		while(ptcount--)
		{
			double	x,y;
			inReader.ReadDouble(x);
			inReader.ReadDouble(y);
			holes.back().push_back(Point_2((x), (y)));
		}
	}
	
	obj.mShape = Polygon_with_holes_2(ring,holes.begin(),holes.end());

	inReader.ReadDouble(obj.mHeight);
	int derived;
	inReader.ReadInt(derived);
	obj.mDerived = (derived != 0);
}

void WritePointFeature(IOWriter& inWriter, const GISPointFeature_t& i)
{
	inWriter.WriteInt(i.mFeatType);
	WriteParamMap(inWriter, i.mParams);
	inWriter.WriteDouble(CGAL::to_double(i.mLocation.x()));
	inWriter.WriteDouble(CGAL::to_double(i.mLocation.y()));
}

void ReadPointFeature(IOReader& inReader, GISPointFeature_t& feature, const TokenConversionMap& c)
{
	inReader.ReadInt(feature.mFeatType);
	feature.mFeatType = c[feature.mFeatType];
	ReadParamMap(inReader, feature.mParams, c);
	double x, y;
	inReader.ReadDouble(x);
	inReader.ReadDouble(y);
	feature.mLocation = Point_2(x,y);
	feature.mInstantiated = false;
}

void WritePolygonFeature(IOWriter& inWriter, const GISPolygonFeature_t& i)
{
	inWriter.WriteInt(i.mFeatType);
	WriteParamMap(inWriter, i.mParams);
	inWriter.WriteInt(i.mShape.outer_boundary().size());
	for (Polygon_2::const_iterator vv = i.mShape.outer_boundary().vertices_begin();
		vv != i.mShape.outer_boundary().vertices_end(); ++vv)
	{
		inWriter.WriteDouble(CGAL::to_double(vv->x()));
		inWriter.WriteDouble(CGAL::to_double(vv->y()));
	}
}

void ReadPolygonFeature(IOReader& inReader, GISPolygonFeature_t& obj, const TokenConversionMap& c)
{
	inReader.ReadInt(obj.mFeatType);
	obj.mFeatType = c[obj.mFeatType];
	ReadParamMap(inReader, obj.mParams, c);
	int	ptcount;
	inReader.ReadInt(ptcount);
	while(ptcount--)
	{
		double	x,y;
		inReader.ReadDouble(x);
		inReader.ReadDouble(y);
		obj.mShape.outer_boundary().push_back(Point_2((x),(y)));
		obj.mInstantiated = false;
	}
}

void WriteAreaFeature(IOWriter& inWriter, const GISAreaFeature_t& i)
{
	inWriter.WriteInt(i.mFeatType);
	WriteParamMap(inWriter, i.mParams);
}

void ReadAreaFeature(IOReader& inReader, GISAreaFeature_t& obj, const TokenConversionMap& c)
{
	inReader.ReadInt(obj.mFeatType);
	obj.mFeatType = c[obj.mFeatType];
	ReadParamMap(inReader, obj.mParams, c);
}

#pragma mark -

void	WriteMap(FILE * fi, const Pmwx& inMap, ProgressFunc inProgress, int atomID)
{
	Pmwx::Face_const_iterator f;

	StAtomWriter	mapAtom(fi, atomID);

	if (inProgress)	inProgress(0, 1, "Writing", 0.0);

	double	total = (inMap.number_of_faces() + inMap.number_of_halfedges() + inMap.number_of_vertices()) * 2.0;
	int	ctr = 0;

	{
			StAtomWriter 	mainMap(fi, kMainMapID);
			FileWriter		writer(fi);

		V_Index	v_index(inMap.vertices_begin(), inMap.vertices_end());
		H_Index	h_index(inMap.halfedges_begin(), inMap.halfedges_end());

		writer.WriteInt(inMap.number_of_vertices());
		writer.WriteInt(inMap.number_of_halfedges());
		writer.WriteInt(inMap.number_of_faces());

		for (Pmwx::Vertex_const_iterator v = inMap.vertices_begin();
			v != inMap.vertices_end(); ++v, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);
		
			writer.WriteDouble(CGAL::to_double(v->point().x()));
			writer.WriteDouble(CGAL::to_double(v->point().y()));
		}

		for (Pmwx::Halfedge_const_iterator he = inMap.halfedges_begin();
			he != inMap.halfedges_end(); ++he, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);

			writer.WriteInt(v_index[he->target()]);
			writer.WriteDouble(CGAL::to_double(he->source()->point().x()));
			writer.WriteDouble(CGAL::to_double(he->source()->point().y()));
			writer.WriteDouble(CGAL::to_double(he->target()->point().x()));
			writer.WriteDouble(CGAL::to_double(he->target()->point().y()));
			
			// Ben says: we used to mark every other edge as dominant.  Now..since we are going to put vector data on BOTH halves of the half-edge
			// (for the purpose of directionality) we're going to be a bit hosed anyway, but at least maintain the every-other semantics.
			bool	was_dom = ctr % 2;
			writer.WriteInt(was_dom);	//he->data().mDominant);
			writer.WriteInt(he->data().mTransition);
			writer.WriteDouble(he->data().mInset);		
		}

		for (f = inMap.faces_begin();
			f != inMap.faces_end(); ++f, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);

			if (f->is_unbounded())
				writer.WriteInt(0);
			else {
				writer.WriteInt(CountCirculator(f->outer_ccb()));
				Pmwx::Ccb_halfedge_const_circulator	edge, last;
				last = edge = f->outer_ccb();
				do {
					// This warrants a comment - in porting to a native PM type: the reverse indexer uses
					// iterators because it likes iterator tags to know if it can run in constant time (it can't).
					// So we take the circulator, get the edge, take a ptr and force an iterator.  Since we use inline lists this works.
					writer.WriteInt(h_index[Pmwx::Halfedge_const_iterator(&*edge)]);
					++edge;
				} while (last != edge);
			}

			writer.WriteInt(distance(f->holes_begin(), f->holes_end()));
			for (Pmwx::Hole_const_iterator hole = f->holes_begin();
				hole != f->holes_end(); ++hole)
			{
				writer.WriteInt(CountCirculator(*hole));
				Pmwx::Ccb_halfedge_const_circulator	edge, last;
				last = edge = *hole;
				do {
					writer.WriteInt(h_index[Pmwx::Halfedge_const_iterator(&*edge)]);
					++edge;
				} while (last != edge);
			}
			
			int dummy = f->data().IsWater();
			writer.WriteInt(dummy);
//			writer.WriteInt(f->mIsWater);
			writer.WriteInt(f->data().mTerrainType);
		}
	}

	{
		StAtomWriter	edges1(fi, kEdgeData1);
		FileWriter		writer(fi);
		for (Pmwx::Halfedge_const_iterator he = inMap.halfedges_begin();
			he != inMap.halfedges_end(); ++he, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);
		
			WriteVector(writer, he->data().mSegments, WriteNetworkSegment);
			WriteParamMap(writer, he->data().mParams);		
		}
	}

	{
		StAtomWriter	faces1(fi, kFaceData1);
		FileWriter		writer(fi);
		for (f = inMap.faces_begin();
			f != inMap.faces_end(); ++f, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);
		
			WriteParamMap(writer, f->data().mParams);
			WriteVector(writer, f->data().mObjs, WriteObjPlacement);
			WriteVector(writer, f->data().mPolyObjs, WritePolyObjPlacement);
			WriteVector(writer, f->data().mPointFeatures, WritePointFeature);
			WriteVector(writer, f->data().mPolygonFeatures, WritePolygonFeature);
			vector<GISAreaFeature_t>	fakeVector;
			//fakeVector.push_back(f->data().mAreaFeature[0]);
			WriteVector(writer, fakeVector, WriteAreaFeature);
		}
	}
	{
		StAtomWriter	vertices1(fi, kVertData1);
		FileWriter		writer(fi);
		for (Pmwx::Vertex_const_iterator v = inMap.vertices_begin();
			v != inMap.vertices_end(); ++v, ++ctr)
		{
			if (inProgress && total && (PROGRESS_RATIO) && (ctr % PROGRESS_RATIO) == 0) inProgress(0, 1, "Writing", (float) ctr / total);
			
			writer.WriteBulk((char *) &v->data().mTunnelPortal, 1, false);
		}
	}

	if (inProgress) inProgress(0, 1, "Writing", 1.0);
}

#pragma mark -

class	MapScanner {
public:

	int			mVertices;
	int			mFaces;
	int			mHalfedges;
	int			mTotal;
	int			mCount;
	IOReader *	mReader;
	// yes really, pointers not handles
	Arrangement_2&           m_arr;
	Arr_accessor             m_arr_access;
	vector<DFace*>		mFacesVec;
	vector<DHalfedge*>	mHalfedgesVec;
	vector<DVertex*>	mVerticesVec;
	ProgressFunc 	mProgress;
	bool			mLegacy;

	MapScanner(IOReader * inReader, Pmwx & the_map, ProgressFunc func) : mReader(inReader),
		mVertices(0), mFaces(0), mHalfedges(0), mProgress(func), mTotal(0), mCount(0), mLegacy(false),
	    m_arr (the_map), m_arr_access (m_arr)
	{
		if (mProgress) mProgress(0, 1, "Reading", 0.0);

	}

	~MapScanner()
	{
		if (mProgress) mProgress(0, 1, "Reading", 1.0);
	}

	std::size_t   number_of_vertices()   const { return mVertices; }
	std::size_t   number_of_halfedges()  const { return mHalfedges; }
	std::size_t   number_of_faces()     const { return mFaces; }


	void scan_pm_vhf_sizes(void)
	{
		mReader->ReadInt(mVertices);
		mReader->ReadInt(mHalfedges);
		mReader->ReadInt(mFaces);
		mTotal = mVertices + mHalfedges + mFaces;
	}
	
	DVertex* scan_vertex (Pmwx& the_map)
	{	
		++mCount;
		if (mProgress && mTotal && (PROGRESS_RATIO) && (mCount % PROGRESS_RATIO) == 0) mProgress(0, 1, "Reading", (double) mCount / (double) mTotal);

		double	x, y;
		mReader->ReadDouble(x);
		mReader->ReadDouble(y);
		
//		if (p == Point_2(0.0, 0.0))
//			printf("WARNING: got null pt.\n");
//		DVertex* v = &*CGAL::insert_point(the_map, Point_2((x),(y)));
		DVertex * v = m_arr_access.new_vertex(Point_2(x,y));
		mVerticesVec.push_back(v);
		return v;
	}

	DHalfedge* scan_halfedge (DHalfedge * me)
	{
		++mCount;
		if (mProgress && mTotal && (PROGRESS_RATIO) && (mCount % PROGRESS_RATIO) == 0) mProgress(0, 1, "Reading", (double) mCount / (double) mTotal);

		double	x1, y1, x2, y2;
		X_monotone_curve_2 cv;
		mReader->ReadDouble(x1);
		mReader->ReadDouble(y1);
		mReader->ReadDouble(x2);
		mReader->ReadDouble(y2);

//					FastKernel kernel;
  //      FastKernel::Compare_xy_2 compare_xy = 
    //                                              kernel.compare_xy_2_object();
		
		
		bool larger = ((x1 == x2 && y2 < y1) || (x2 < x1));
		cv = X_monotone_curve_2(Segment_2(Point_2(x1, y1), Point_2(x2, y2)));
		DHalfedge* h = me ? me : m_arr_access.new_edge(cv);
//		CGAL::Comparison_result r;
//		h->set_direction(r = compare_xy(cv.source(),cv.target()));
		h->set_direction(larger ? CGAL::LARGER : CGAL::SMALLER);
//		if(r == CGAL::LARGER) DebugAssert(!larger);
//		if(r == CGAL::SMALLER) DebugAssert(larger);
		
		int	dominant;		
		mReader->ReadInt(dominant);
//		h->data().mDominant = (dominant != 0);
		mReader->ReadInt(h->data().mTransition);
		mReader->ReadDouble(h->data().mInset);		


		mHalfedgesVec.push_back(h);
		return h;
	}

	void scan_face(DFace* f) 
	{
		++mCount;
		if (mProgress && mTotal && (PROGRESS_RATIO) && (mCount % PROGRESS_RATIO) == 0) mProgress(0, 1, "Reading", (double) mCount / (double) mTotal);

		int  num_of_holes, num_halfedges_on_outer_ccb, i = 0;

		mReader->ReadInt(num_halfedges_on_outer_ccb);

		if (num_halfedges_on_outer_ccb > 0)
		{
			int  index, prev_index = 0, first_index;
			
			for (unsigned int j = 0; j < num_halfedges_on_outer_ccb; j++) 
			{
				mReader->ReadInt(index);
				DHalfedge* nh = mHalfedgesVec[index];

				if (j > 0)
				{
					DHalfedge* prev_nh = mHalfedgesVec[prev_index];
					prev_nh->set_next(nh);
					nh->set_prev(prev_nh);
				} else {
					f->set_halfedge(nh);
					first_index = index;
				}
				nh->set_face(f);
				prev_index = index;
			}

			// making the last halfedge point to the first one (cyclic order).
			DHalfedge* nh = mHalfedgesVec[first_index];
			DHalfedge* prev_nh = mHalfedgesVec[prev_index];
			prev_nh->set_next(nh);
			nh->set_prev(prev_nh);
		}

		mReader->ReadInt(num_of_holes);
		for (unsigned int k = 0; k < num_of_holes; k++)
		{
		#if USE_HOLES
			DHole * new_hole = m_arr_access.new_hole();
			new_hole->set_face(f);
		#endif
			int  num_halfedges_on_inner_ccb;
			mReader->ReadInt(num_halfedges_on_inner_ccb);

			int  index, prev_index, first_index;
			for (unsigned int j = 0; j < num_halfedges_on_inner_ccb; j++)
			{
				mReader->ReadInt(index);

				DHalfedge* nh = mHalfedgesVec[index];
				if (j > 0) 
				{
					DHalfedge* prev_nh = mHalfedgesVec[prev_index];
					prev_nh->set_next(nh);
					nh->set_prev(prev_nh);
				} else {
#if  USE_HOLES
					new_hole->set_iterator(f->add_hole(nh));
#else
					f->add_hole(nh);
#endif					
					first_index = index;
				}


#if USE_HOLES
				nh->set_hole(new_hole);
#else
				nh->set_face(f);
#endif
				prev_index = index;
			}

			// making the last halfedge point to the first one (cyclic order).
			DHalfedge* nh = mHalfedgesVec[first_index];
			DHalfedge* prev_nh = mHalfedgesVec[prev_index];
			prev_nh->set_next(nh);
			nh->set_prev(prev_nh);
		}

		// Other params
		int dummy;
//		mReader->ReadInt(f->mIsWater);
		mReader->ReadInt(dummy);
		mReader->ReadInt(f->data().mTerrainType);
		if (f->data().mTerrainType == NO_VALUE) 
		{
			if (dummy) mLegacy = true;
			f->data().mTerrainType = dummy ? terrain_Water : terrain_Natural;
		}
		mFacesVec.push_back(f);
	}

	void scan_index(std::size_t& index)
	{
		int	n;
		mReader->ReadInt(n);
		index = n;
	}

	void read(Pmwx& the_map)
	{
		std::vector<DHalfedge* >  halfedges_vec;  
		std::vector<DVertex* >    vertices_vec; 

		scan_pm_vhf_sizes();

		unsigned int  i;
		for (i = 0; i < number_of_vertices(); i++)
		{
			DVertex * nv = scan_vertex (the_map);
			nv->set_halfedge((DHalfedge *) 0x0BADF00D);
			vertices_vec.push_back(nv);
		}

		for (i = 0; i < number_of_halfedges(); i++, i++)
		{
			DHalfedge * nh, * no;
			DVertex * nv1, * nv2;
			std::size_t index1, index2;

			//nh = m_arr_access.new_edge();
			scan_index(index1);
			nh = scan_halfedge(NULL);
			scan_index (index2);
			no = scan_halfedge(nh->opposite());

			nv1 = vertices_vec[index1];
			nv1->set_halfedge(nh); 
			nh->set_vertex(nv1);

			nv2 = vertices_vec[index2];
			nv2->set_halfedge(no); 
			no->set_vertex(nv2);

			halfedges_vec.push_back(nh);
			halfedges_vec.push_back(no);
		}

		for (i = 0; i < number_of_faces(); i++)
		{
			DFace * nf =  m_arr_access.un_face();
			if(i == 0)
			{
				DHalfedge * h = *nf->holes_begin();
				nf = h->opposite()->face();
			}
			
			if(i != 0) nf = m_arr_access.new_face() ;

			// BEN SAYS: back in the day, if i == 0 (unbounded face) we would set its half edge to NULL.
			// NOT ANY MORE!  In the new CGAL 3.3.1 the unbounded face(s) are bounded by ficticious edges that make up a single hole
			// in the "ficticious face".  Now..this is ALREADY set up for us - in an EMPTY dcel we have two faces:
			// - the ficticious face, and the unbounded face (inside it).  A rectangle around infinity separates them.
			// So we LEAVE THIS THE HELL ALONE.  Nuking the half-edge around the unbounded face is, like, bad and drives CGAL insane.
			// Only the ficticious face can really be unbounded.
//			if (i == 0) //this is the unbounded face.
//				nf->set_halfedge (NULL);

			scan_face(nf);
		}
	}

	

};

void	ReadMap(XAtomContainer& container, Pmwx& inMap, ProgressFunc inProgress, int atomID, const TokenConversionMap& c)
{
	XAtom			meAtom, mapAtom, faceAtom, edgeAtom, vertAtom;
	XAtomContainer	meContainer, mapContainer, faceContainer, edgeContainer, vertContainer;

	if (!container.GetNthAtomOfID(atomID, 0, meAtom)) return;
	meAtom.GetContents(meContainer);

	if (!meContainer.GetNthAtomOfID(kMainMapID, 0, mapAtom)) return;
	mapAtom.GetContents(mapContainer);
	MemFileReader	readMainMap(mapContainer.begin, mapContainer.end);
#if REBUILD_MAP	
	Pmwx	tempMap;
#else
	#define tempMap inMap
#endif	
	MapScanner	scanner(&readMainMap, tempMap, inProgress);
	scanner.read(tempMap);

	if (meContainer.GetNthAtomOfID(kFaceData1, 0, faceAtom))
	{
		faceAtom.GetContents(faceContainer);
		MemFileReader	readFaceData(faceContainer.begin, faceContainer.end);
		for (vector<DFace*>::iterator f = scanner.mFacesVec.begin(); f != scanner.mFacesVec.end(); ++f)
		{
			ReadParamMap(readFaceData, (*f)->data().mParams, c);
			ReadVector(readFaceData, (*f)->data().mObjs, ReadObjPlacement, c);
			ReadVector(readFaceData, (*f)->data().mPolyObjs, ReadPolyObjPlacement, c);
			ReadVector(readFaceData, (*f)->data().mPointFeatures, ReadPointFeature, c);
			ReadVector(readFaceData, (*f)->data().mPolygonFeatures, ReadPolygonFeature, c);
			vector<GISAreaFeature_t>	fakeVector;
			ReadVector(readFaceData, fakeVector, ReadAreaFeature, c);
			if (!fakeVector.empty()) {
				(*f)->data().mAreaFeature = fakeVector[0];
			} else {
				(*f)->data().mAreaFeature.mFeatType = NO_VALUE;		
			}
			if (!scanner.mLegacy)		
				(*f)->data().mTerrainType = c[(*f)->data().mTerrainType];
		}
	}

	if (meContainer.GetNthAtomOfID(kEdgeData1, 0, edgeAtom))
	{
		edgeAtom.GetContents(edgeContainer);
		MemFileReader	readEdgeData(edgeContainer.begin, edgeContainer.end);
		for (vector<DHalfedge*>::iterator h = scanner.mHalfedgesVec.begin(); h != scanner.mHalfedgesVec.end(); ++h)
		{
			ReadVector(readEdgeData, (*h)->data().mSegments, ReadNetworkSegment, c);
			ReadParamMap(readEdgeData, (*h)->data().mParams, c);
		}
	}
	if (meContainer.GetNthAtomOfID(kVertData1, 0, vertAtom))
	{
		vertAtom.GetContents(vertContainer);
		MemFileReader	readVertData(vertContainer.begin, vertContainer.end);
		for (vector<DVertex*>::iterator v = scanner.mVerticesVec.begin(); v != scanner.mVerticesVec.end(); ++v)
		{
			readVertData.ReadBulk((char *) &(*v)->data().mTunnelPortal, 1, false);
		}
	}
#if REBUILD_MAP	
	RebuildMap(tempMap, inMap);
#endif
}	


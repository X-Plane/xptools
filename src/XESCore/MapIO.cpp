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
#include <CGAL/IO/Arrangement_2_writer.h>
#include <CGAL/IO/Arrangement_2_reader.h>

// Ratio of operations to an update of the progress bar.
#define	PROGRESS_RATIO	5000

const int kMainMapID = 'MAP2';

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

// Numeric type for now is a lazy wrapper around quotient around mp_float.
// lazy wrapper = ptr with lazy eval and lazy op tree.  Use ctor and exact() to get around this.
// quotient is a numerator/denominator pair to make division exact.  I/O both members.
// MP_Float is manual floating point - mantissa is vector of shorts, exponent is double.  Just IO it all.

/*
	// This is for debugging- use without the lazy wrappers around quotient+MP_float
void hex_print(const NT& c)
{
	printf("%lf/%lf ",c.num.exp,c.den.exp);
	for(int n = 0; n < c.num.v.size(); ++n)
		printf("%02X",c.num.v[n]);
	printf(" ");
	for(int n = 0; n < c.den.v.size(); ++n)
		printf("%02X",c.den.v[n]);
	printf("\n");
}
*/
void WriteCoordinate(IOWriter& inWriter, const NT& c)
{
	NT::ET e = c.exact();
	NT::ET::NT num = e.num;
	NT::ET::NT den = e.den;
//	NT e = c;
//	NT::NT num = e.num;
//	NT::NT den = e.den;
	
	inWriter.WriteDouble(num.exp);
	inWriter.WriteInt(num.v.size());
	for(int n = 0; n < num.v.size(); ++n)
		inWriter.WriteShort(num.v[n]);

	inWriter.WriteDouble(den.exp);
	inWriter.WriteInt(den.v.size());
	for(int n = 0; n < den.v.size(); ++n)
		inWriter.WriteShort(den.v[n]);
}

void ReadCoordinate(IOReader& inReader, NT& c)
{
	int n;
	
	NT::ET et;
//	NT et;
	
	inReader.ReadDouble(et.num.exp);
	inReader.ReadInt(n);
	et.num.v.clear();
	while(n--)
	{
		et.num.v.push_back(short());
		inReader.ReadShort(et.num.v.back());
	}

	inReader.ReadDouble(et.den.exp);
	inReader.ReadInt(n);
	et.den.v.clear();
	while(n--)
	{
		et.den.v.push_back(short());
		inReader.ReadShort(et.den.v.back());
	}
	
	c = et;
}

void WritePoint(IOWriter& inWriter, const Point_2& p)
{
	WriteCoordinate(inWriter,p.x());
	WriteCoordinate(inWriter,p.y());
}

void ReadPoint(IOReader& inReader, Point_2& p)
{
	NT	x, y;
	ReadCoordinate(inReader,x);
	ReadCoordinate(inReader,y);
	p = Point_2(x,y);
}


#pragma mark -

#define VERSION_VERTEX 1
#define VERSION_HALFEDGE 1
#define VERSION_FACE 1

class PmwxFmt { 
public:

	typedef Pmwx										   Arrangement_2;
	typedef Arrangement_2::Size                   Size;
	typedef Arrangement_2::Dcel                   Dcel;
	typedef Arrangement_2::X_monotone_curve_2     X_monotone_curve_2;
	typedef Arrangement_2::Point_2                Point_2;

	typedef Arrangement_2::Vertex_handle          Vertex_handle;
	typedef Arrangement_2::Halfedge_handle        Halfedge_handle;
	typedef Arrangement_2::Face_handle            Face_handle;

	typedef Arrangement_2::Vertex_const_handle    Vertex_const_handle;
	typedef Arrangement_2::Halfedge_const_handle  Halfedge_const_handle;
	typedef Arrangement_2::Face_const_handle      Face_const_handle;

	IOReader *					reader;
	IOWriter *					writer;
	const TokenConversionMap * 	token_map;
	PmwxFmt(IOReader * r, const TokenConversionMap * t) : reader(r), writer(NULL), token_map(t) { }
	PmwxFmt(IOWriter * w) : reader(NULL), writer(w), token_map(NULL) { }

	void write_size (const char *label, Size size)
	{
		writer->WriteInt(size);
	}

	void write_arrangement_begin () { }
	void write_arrangement_end() { }
	void write_vertices_begin () { }
	void write_vertices_end () { }
	void write_edges_begin () { }
	void write_edges_end () { }
	void write_faces_begin () { }
	void write_faces_end () { }
	void write_vertex_begin () { }
	void write_vertex_end () { }
	void write_edge_begin () { }
	void write_edge_end () { }
	void write_face_begin () { }
	void write_face_end () { }
	void write_outer_ccbs_begin () { }
	void write_outer_ccbs_end () { }
	void write_inner_ccbs_begin () { }
	void write_inner_ccbs_end () { }
	void write_ccb_halfedges_begin() { }
	void write_ccb_halfedges_end() { }
	void write_isolated_vertices_begin () { }
	void write_isolated_vertices_end () { }


	virtual void write_point (const Point_2& p)
	{
		WritePoint(*writer, p);
	}

	virtual void write_vertex_data (Vertex_const_handle  v)
	{
		writer->WriteInt(VERSION_VERTEX);
		// Version 1
		writer->WriteInt(v->data().mTunnelPortal ? 1 : 0);
	}

	void write_vertex_index (int idx)
	{
		writer->WriteInt(idx);
	}

	virtual void write_x_monotone_curve (const X_monotone_curve_2& cv)
	{
		WritePoint(*writer, cv.source());
		WritePoint(*writer, cv.target());
		writer->WriteInt(cv.data().size());
		for(EdgeKey_container::const_iterator e = cv.data().begin(); e != cv.data().end(); ++e)
			writer->WriteInt(*e);
	}

	virtual void write_halfedge_data (Halfedge_const_handle e)
	{
		writer->WriteInt(VERSION_HALFEDGE);
		// Version 1
		writer->WriteInt(e->data().mTransition);
		WriteVector(*writer, e->data().mSegments, WriteNetworkSegment);
		WriteParamMap(*writer, e->data().mParams);
		writer->WriteDouble(e->data().mInset);
	}

	virtual void write_face_data (Face_const_handle f)
	{
		writer->WriteInt(VERSION_FACE);
		// Version 1
		writer->WriteInt(f->data().mTerrainType);
		WriteParamMap(*writer, f->data().mParams);
		WriteVector(*writer, f->data().mPointFeatures, WritePointFeature);
		WriteVector(*writer, f->data().mPolygonFeatures, WritePolygonFeature);
		WriteAreaFeature(*writer, f->data().mAreaFeature);
		WriteVector(*writer, f->data().mObjs, WriteObjPlacement);
		WriteVector(*writer,f->data().mPolyObjs,WritePolyObjPlacement);
	}

	void write_halfedge_index (int idx)
	{
		writer->WriteInt(idx);
	}


	void read_arrangement_begin () { } 
	void read_arrangement_end() { } 
	void read_vertices_begin() { }
	void read_vertices_end() { }
	void read_edges_begin() { }
	void read_edges_end() { }
	void read_faces_begin() { }
	void read_faces_end() { }
	void read_vertex_begin () { }
	void read_vertex_end () { }
	void read_edge_begin () { }
	void read_edge_end () { }
	void read_face_begin () { }
	void read_face_end () { }
	void read_outer_ccbs_begin () { }
	void read_outer_ccbs_end () { }
	void read_inner_ccbs_begin () { }
	void read_inner_ccbs_end () { }
	void read_ccb_halfedges_begin() { }
	void read_ccb_halfedges_end() { } 
	void read_isolated_vertices_begin () { }
	void read_isolated_vertices_end () { } 



	Size read_size (const char* /* title */ = NULL)
	{
		int n;
		reader->ReadInt(n);
		return n;
	}

	virtual void read_point (Point_2& p) 
	{
		ReadPoint(*reader,p);
	}

	virtual void read_vertex_data (Vertex_handle v)
	{
		int vers, i;
		reader->ReadInt(vers);
		if(vers >= 1)
		{
			reader->ReadInt(i);
			v->data().mTunnelPortal = i != 0;
			
		}
	}

	int read_vertex_index () 
	{
		int n;
		reader->ReadInt(n);
		return n;
	}

	virtual void read_x_monotone_curve (X_monotone_curve_2& cv) 
	{
		Point_2 s, t;
		int n, v;
		EdgeKey_container d;
		ReadPoint(*reader,s);
		ReadPoint(*reader,t);
		reader->ReadInt(n);
		while(n--)
		{
			reader->ReadInt(v);
			d.insert(v);
		}
	
		cv = X_monotone_curve_2(Segment_2(s,t),d);
	}	
		
	virtual void read_halfedge_data (Halfedge_handle e)
	{
		int vers;
		reader->ReadInt(vers);
		if(vers >= 1)
		{
			reader->ReadInt(e->data().mTransition);
			ReadVector(*reader, e->data().mSegments, ReadNetworkSegment, *token_map);
			ReadParamMap(*reader, e->data().mParams, *token_map);
			reader->ReadDouble(e->data().mInset);
		}
	}

	int read_halfedge_index ()
	{ 
		int n;
		reader->ReadInt(n);
		return n;
	}

	virtual void read_face_data (Face_handle f)
	{
		int vers;
		reader->ReadInt(vers);
		if(vers >= 1)
		{
			reader->ReadInt(f->data().mTerrainType);
			f->data().mTerrainType = token_map->at(f->data().mTerrainType);			
			ReadParamMap(*reader, f->data().mParams,*token_map);
			ReadVector(*reader, f->data().mPointFeatures, ReadPointFeature, *token_map);
			ReadVector(*reader, f->data().mPolygonFeatures, ReadPolygonFeature, *token_map);
			ReadAreaFeature(*reader, f->data().mAreaFeature, *token_map);
			ReadVector(*reader, f->data().mObjs, ReadObjPlacement, *token_map);
			ReadVector(*reader,f->data().mPolyObjs,ReadPolyObjPlacement, *token_map);
		}
	}

	
private:

	PmwxFmt();
	
};	


#pragma mark -

void	WriteMap(FILE * fi, const Pmwx& inMap, ProgressFunc inProgress, int atomID)
{
	StAtomWriter	mapAtom(fi, atomID);

	if (inProgress)	inProgress(0, 1, "Writing", 0.0);

	double	total = inMap.number_of_faces() + inMap.number_of_halfedges() + inMap.number_of_vertices();
	int	ctr = 0;

	{
		StAtomWriter 	mainMap(fi, kMainMapID);
		FileWriter		writer(fi);

		PmwxFmt	write_formatter(&writer);
		
		CGAL::Arrangement_2_writer<Pmwx>	arr_writer(inMap);
		
		arr_writer(write_formatter);
	}

	if (inProgress) inProgress(0, 1, "Writing", 1.0);
}


void	ReadMap(XAtomContainer& container, Pmwx& inMap, ProgressFunc inProgress, int atomID, const TokenConversionMap& c)
{
	XAtom			meAtom, mapAtom, faceAtom, edgeAtom, vertAtom;
	XAtomContainer	meContainer, mapContainer, faceContainer, edgeContainer, vertContainer;

	if (!container.GetNthAtomOfID(atomID, 0, meAtom)) return;
	meAtom.GetContents(meContainer);

	if (!meContainer.GetNthAtomOfID(kMainMapID, 0, mapAtom)) return;
	mapAtom.GetContents(mapContainer);
	MemFileReader	readMainMap(mapContainer.begin, mapContainer.end);
	
	PmwxFmt	read_formatter(&readMainMap, &c);
		
	CGAL::Arrangement_2_reader<Pmwx>	arr_reader(inMap);
		
	arr_reader(read_formatter);
}



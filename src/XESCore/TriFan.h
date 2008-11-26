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
#ifndef TRIFAN_H
#define TRIFAN_H

#include <list>
using std::list;

#include "MeshDefs.h"

#define STUB_FANS 1

struct	TriFan_t;

typedef multimap<int, TriFan_t *>				TriFanQueue;		// Tri fans sorted by number of tris, best are last
typedef multimap<CDT::Face_handle, TriFan_t *>	TriFanTable;		// Index from a face to each tri fan using it

struct	TriFan_t {
	CDT::Vertex_handle				center;		// Our center
	list<CDT::Face_handle>			faces;		// Our faces in traversal order
	bool							circular;	// True if this is circular - makes it easier to remove tris
	TriFanQueue::iterator			self;		// Each tri fan has a link back to itself in the queue
												// so it can readjust itself.  (Poor man's priority q)
};

struct TriStrip_t {
	list<CDT::Vertex_handle>		strip;
};

class	TriFanBuilder {
public:
	TriFanBuilder(CDT * inMesh);
	~TriFanBuilder();

	void		AddTriToFanPool(CDT::Face_handle inFace);
	void		CalcFans(void);

	int					GetNextPrimitive(list<CDT::Vertex_handle>& out_handles);

	void				GetNextTriFan(list<CDT::Vertex_handle>& out_handles);
	void				GetRemainingTriangles(list<CDT::Vertex_handle>& out_handles);

	void				Validate(void);

private:

#if STUB_FANS
	vector<CDT::Face_handle>		faces;
#else

	TriFan_t *			GetNextFan(void);
	void				DoneWithFan(TriFan_t * inFan);
	CDT::Face_handle	GetNextRemainingTriangle(void);


	void		PullFaceFromFan(CDT::Face_handle f, TriFan_t * fan);

	TriFanQueue					queue;				// Our tri fans in priority order
	TriFanTable					index;				// Index of who is using what tri fans
	set<CDT::Vertex_handle>		vertices;			// Vertices that we need to tri fan for building up the struct
#endif	
	CDT *						mesh;				// Our mesh
};

#endif


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
#include "HackExport.h"
#include "EnvParser.h"
#include "EnvWrite.h"
#include "CompGeom.h"
#include "Persistence.h"
#include "MapDefs.h"
#include "ParamDefs.h"

const	double	kINSET = 90.0 / (6080.0 * 60.0);
const double	kSPACING = 100.0 / (6080.0 * 60.0);

int	GetRoadType(Pmwx::Halfedge_handle e)
{
	if (!e->mDominant) e = e->twin();
	if (e->mMark) return 0;
	if (e->mSegments.empty()) return 0;
	return e->mSegments[0].mFeatType;
}

Pmwx::Halfedge_handle	next_he_of_type(Pmwx::Halfedge_handle e, int type)
{
	Pmwx::Halfedge_handle n = e->next_halfedge();
	if (GetRoadType(n) != type)
		return Pmwx::Halfedge_handle();
	return n;
}

Pmwx::Halfedge_handle	prev_he_of_type(Pmwx::Halfedge_handle e, int type)
{
	Pmwx::Halfedge_handle	n = e;
	while (n->next_halfedge() != e)
		n = n->next_halfedge();

	if (GetRoadType(n) != type)
		return Pmwx::Halfedge_handle();
	return n;
}

void	HackExport(Pmwx& ioMap, const char * inFileName)
{
	ClearEnvData();

	if (ReadEnvFile(inFileName) != 0)
	{
		printf("ERROR: could not read %s\n", inFileName);
		return;
	}

	gRoads.clear();
	gTrails.clear();
	gTrains.clear();
	gLines.clear();
	gTaxiways.clear();
	gRiverVectors.clear();


	for (Pmwx::Halfedge_iterator e = ioMap.halfedges_begin();
		e != ioMap.halfedges_end(); ++e)
	{
		e->mMark = false;
	}

	for (Pmwx::Halfedge_iterator e = ioMap.halfedges_begin();
		e != ioMap.halfedges_end(); ++e)
	{
		if (e->mDominant)
		{
			Pmwx::Halfedge_handle	fst = e, lst = e, foo;
			int tp = GetRoadType(e);
			e->mMark = true;
			if (Road_IsHighway(tp) ||
			    Road_IsMainDrag(tp) ||
			    Road_IsLocal(tp) ||
			    Road_IsAccess(tp))
		   {
				foo = prev_he_of_type(fst, tp);
				while (foo != Pmwx::Halfedge_handle() && foo != lst)
				{
					fst = foo;
					fst->mMark = true;
					foo = prev_he_of_type(fst, tp);
				}
				foo = next_he_of_type(lst, tp);
				while (foo != Pmwx::Halfedge_handle() && foo != fst)
				{
					lst = foo;
					lst->mMark = true;
					foo = next_he_of_type(lst, tp);
				}
				PathInfo	p;
				p.term = 0;
				p.latitude = CGAL::to_double(fst->source()->point().y());
				p.longitude = CGAL::to_double(fst->source()->point().x());
				if (Road_IsHighway(tp))
					gRoads.push_back(p);
				else
					gTrains.push_back(p);

				for (foo = fst; foo != lst; foo = foo->next_halfedge())
				{
					p.latitude = CGAL::to_double(foo->target()->point().y());
					p.longitude = CGAL::to_double(foo->target()->point().x());
					if (Road_IsHighway(tp))
						gRoads.push_back(p);
					else
						gTrains.push_back(p);
				}

				p.latitude = CGAL::to_double(lst->target()->point().y());
				p.longitude = CGAL::to_double(lst->target()->point().x());
				p.term = 1;
				if (Road_IsHighway(tp))
					gRoads.push_back(p);
				else
					gTrains.push_back(p);
		   }
		}
	}

	for (int n = 0; n < (201 * 151); ++n)
	{
		if (gVertices[n].landUse == 1)
			gVertices[n].landUse = 2;
	}

	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	{
		if (!f->IsWater())
		{
			vector<double>	insets;
			Polygon_2	p, p_inset;
			Pmwx::Ccb_halfedge_circulator	cur, last;
			cur = last = f->outer_ccb();
			do {
				p.push_back(cur->source()->point());
				insets.push_back(1.0);
				++cur;
			} while (cur != last);

			ObjectInfo	obj;
			obj.kind = 8;
			obj.name = "building";

			InsetPolygon(p, insets, kINSET, true, p_inset);

			for (Polygon_2::Edge_const_iterator e = p_inset.edges_begin();
				e != p_inset.edges_end(); ++e)
			{
				Direction_2	dir(*e);
				obj.elevation = -atan2(CGAL::to_double(dir.dy()), CGAL::to_double(dir.dx())) *
				180.0 / 3.14159265;
				Vector_2	vec(*e);

				double	len = sqrt(CGAL::to_double(e->squared_length()));

				double	steps = floor(len / kSPACING);
				double	step = 1.0 / steps;
				for (double t = 0; t < 1.0; t += step)
				{
					Point_2	objLoc = e->source() + vec * t;
					obj.latitude = CGAL::to_double(objLoc.y());
					obj.longitude = CGAL::to_double(objLoc.x());

					if (obj.latitude < 38.0 &&
						obj.latitude > 37.0 &&
						obj.longitude < -122.0 &&
						obj.longitude > -123.0)

					gObjects.push_back(obj);
				}
			}
		}
	}

	if (EnvWrite(inFileName) != 0)
		printf("Error writing file.\n");
}

/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_DrapedOrthophoto.h"
#include "WED_GISUtils.h"
#include "XESConstants.h"
#include "MathUtils.h"

DEFINE_PERSISTENT(WED_DrapedOrthophoto)
TRIVIAL_COPY(WED_DrapedOrthophoto,WED_GISPolygon)

WED_DrapedOrthophoto::WED_DrapedOrthophoto(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	resource(this,"Resource",       XML_Name("draped_orthophoto","resource"),  ""),
	heading(this,"Texture Heading", XML_Name("draped_orthophoto","heading"),   0.0,5,1),
	width(this,"Texture Width",     XML_Name("draped_orthophoto","width"),     0.0,5,2),
	length(this,"Texture Length",   XML_Name("draped_orthophoto","length"),    0.0,5,2),
	top(this,"Texture Top",         XML_Name("draped_orthophoto","tex_top"),   0.0,5,3),
	bottom(this,"Texture Bottom",   XML_Name("draped_orthophoto","tex_bottom"),0.0,5,3),
	left(this,"Texture Left",       XML_Name("draped_orthophoto","tex_left"),  0.0,5,3),
	right(this,"Texture Right",     XML_Name("draped_orthophoto","tex_right"), 0.0,5,3)
{
}

WED_DrapedOrthophoto::~WED_DrapedOrthophoto()
{
}

void		WED_DrapedOrthophoto::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_DrapedOrthophoto::SetResource(const string& r)
{
	resource = r;
}

double WED_DrapedOrthophoto::GetHeading(void) const
{
	return heading.value;
}

void WED_DrapedOrthophoto::SetHeading(double h)
{
	heading = h;
}

void WED_DrapedOrthophoto::SetSizeDisp(double w, double l)
{
	length = l;
	width = w;
}

// this function tells if the resource is a .POL definition (true) or
// something having a different or no suffix at all (false)
// Its important, as orthophoto's are allowed to directly refer to the image. In such cases,
// the .POL, along with a .DDS version of the image, is created when writing the .DSF

bool WED_DrapedOrthophoto::IsNew(string * out_suffix) 
{
	//Find position
	int pos = resource.value.find_last_of('.',resource.value.size());
	if(pos == resource.value.npos)
		return false;
	
	//get the ending extension
	string testString = resource.value.substr(pos);
	
	//If it is not .pol
	
	if(testString != ".pol")
	{
		
		if(out_suffix != NULL)
		{
			*out_suffix = testString;
		}
		//it is new, therefore true
		return true;
	}
	else
	{
		//It is an old .pol file, therefore false
		return false;
	}
}

void  WED_DrapedOrthophoto::GetSubTexture(Bbox2& b)
{
	b.p1.x_ = left;
	b.p1.y_ = bottom;
	b.p2.x_ = right;
	b.p2.y_ = top;
}

void  WED_DrapedOrthophoto::SetSubTexture(const Bbox2& b)
{
	top    = b.p2.y();
	bottom = b.p1.y();
	right  = b.p2.x();
	left   = b.p1.x();
}

// This function recalculates the UV map, stretching the texture to fully cover the polygon.
//
// Unless its a quadrilateral (4-sided non-bezier polygons), the strecth is linear.
// I.e. the texture is scaled independently in u+v directions (aspect changes), until it covers
// all parts of the irregular shaped polygon. Resultingly some parts of the texture are usually
// not visible, but the texture appears 'undistorted'.
// Qudrilateral orthos are special - they are always streched to the corners, i.e. distorted as needed
// to exactly fit the poligon with all of the texture visible.

void WED_DrapedOrthophoto::Redrape(bool updProp)
{ 
	// need to do some sanity checking:
	// During DSF import, properties are being set, even before any nodes have been read, i.e. any OuterRing is set
	if (GetNthChild(0) && HasLayer(gis_UV))
	{
		Bbox2  ll_box;           // the lon/lat bounding box of the poly - this is what the texture needs to cover
		Point2 ctr;

		Bbox2  uv_box;           // the part of the texture we are actually suppposed to use.
		GetSubTexture(uv_box);
		if (uv_box.is_empty())
			return;              // allows turning off the auto-update by setting the UV coordinates of the polygon to all zero.
			                     // usefull if UV mapping is set by hand for each node, like it was mandatory in WED 1.5.

		// We want to allow for rotated textures. Thus we have to rotate the coordinates before UV calculation
		// really doesn't matter around what point we rotate, as long it is somewehre nearby
		double angle = GetHeading();

		GetOuterRing()->GetNthPoint(0)->GetLocation(gis_Geo,ctr);   // simply choose the first point as coordinate rotation center

		int nh = GetNumEntities();

		for (int h = 0; h < nh; h++)
		{
			IGISPointSequence *ring = h ? GetNthHole(h-1) : GetOuterRing();
			int np = ring->GetNumPoints();
			vector <BezierPoint2> pt_bak;                    // backup of the coordinates we're going to rotate
			for(int n = 0; n < np; ++n)
			{
				IGISPoint_Bezier *s = dynamic_cast<IGISPoint_Bezier *> (ring->GetNthPoint(n));
				BezierPoint2 pt;
				if (s)
					s->GetBezierLocation(gis_Geo,pt);
				else
				{
					IGISPoint *sp = ring->GetNthPoint(n);
					Point2 p;
					sp->GetLocation(gis_Geo,p);
					pt.pt = p;
				}
				pt_bak.push_back(pt);
			}
			                                        // now that we have a backup, we can mess with the original without guilt
			ring->Rotate(gis_Geo, ctr, -angle);     // rotate coordinates to match desired texture heading
			if (h==0)
			{
				ring->GetBounds(gis_Geo, ll_box);     // get the bounding box in _rotated_ coordinates
				double w=ll_box.xspan()*DEG_TO_MTR_LAT*cos(ctr.y()*DEG_TO_RAD);
				double l=ll_box.yspan()*DEG_TO_MTR_LAT;
				if (updProp) SetSizeDisp(w,l);
			}
			for(int n = 0; n < np; ++n)
			{
				IGISPoint *src = ring->GetNthPoint(n);
				Point2 st,uv;
				
				// 4-sided orthos w/no bezier nodes are special. They are always streched to these corners, i.e. distorted.
				if(h == 0 && np == 4 && !WED_HasBezierSeq(GetOuterRing()))
				{
					switch (n)
					{
						case 0: uv=uv_box.bottom_left();  break;
						case 1: uv=uv_box.bottom_right(); break;
						case 2: uv=uv_box.top_right();    break;
						case 3: uv=uv_box.top_left();
								if (updProp)
								{
									st = pt_bak[3].pt;
									double hdg = 0.0;
									if (st.y()-ctr.y() != 0.0)
										hdg = RAD_TO_DEG*atan((st.x()-ctr.x())*cos(ctr.y()*DEG_TO_RAD)/(st.y()-ctr.y()));  // very crude heading calculation
									SetHeading(hdg);
								}
					}
				}
				else
				{
					src->GetLocation(gis_Geo,st);
					uv = Point2((st.x() - ll_box.xmin()) / ll_box.xspan() * uv_box.xspan() + uv_box.xmin(),
								(st.y() - ll_box.ymin()) / ll_box.yspan() * uv_box.yspan() + uv_box.ymin());
				}
				src->SetLocation(gis_UV,uv);
				
				IGISPoint_Bezier * srcb  = dynamic_cast <IGISPoint_Bezier *> (ring->GetNthPoint(n));
				if(srcb)
				{
					if(srcb->GetControlHandleHi(gis_Geo,st))
					{
						srcb->SetControlHandleHi(gis_UV,Point2(
							(st.x() - ll_box.xmin()) / ll_box.xspan() * uv_box.xspan() + uv_box.xmin(),
							(st.y() - ll_box.ymin()) / ll_box.yspan() * uv_box.yspan() + uv_box.ymin()));
					}
					if(srcb->GetControlHandleLo(gis_Geo,st))
					{
						srcb->SetControlHandleLo(gis_UV,Point2(
							(st.x() - ll_box.xmin()) / ll_box.xspan() * uv_box.xspan() + uv_box.xmin(),
							(st.y() - ll_box.ymin()) / ll_box.yspan() * uv_box.yspan() + uv_box.ymin()));
					}
					srcb->SetBezierLocation(gis_Geo,pt_bak[n]);    // restore to coordinate to what they were from the backup
				}
				else
					src->SetLocation(gis_Geo,pt_bak[n].pt);
			}
		}
	}
}

void  WED_DrapedOrthophoto::PropEditCallback(int before)
{                                             // we want to catch changes of the heading property only, for now
#if 0 // DEV
	static double old = 0.0;
	if (before)                               // we will _always_ get called twice in succession. Before the edit takes place and after the update.
		old = heading.value;                  // so memorize the heading to see if it changed
	else
	{
		double new_heading = heading.value;
		if(fabs(old - new_heading) > 0.1)     // It changed. Nice to know, since we are called here even if some other property changed ..
		{
			printf("%9lx %12.9lf %12.9lf ch=%i\n",(long int) this, old, new_heading, GetNumEntities());
			Redrape(0);
		}
		else
			printf("false alarm\n");
		fflush(stdout);
	}
#else
	if (!before) Redrape(0);  // updProp=0 prevents the function to issue any property updates.
	                          // Since we're here in a callback called from a property update, we would get into a infinite reciursive call loop
#endif
}

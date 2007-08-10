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

#include "WED_AptIE.h"
#include "WED_Airport.h"
#include "WED_AirportBeacon.h"
#include "WED_AirportBoundary.h"
#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
#include "WED_AirportSign.h"
#include "WED_ATCFrequency.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_RampPosition.h"
#include "WED_Runway.h"
#include "WED_RunwayNode.h"
#include "WED_Sealane.h"
#include "WED_Taxiway.h"
#include "WED_TowerViewpoint.h"
#include "WED_Windsock.h"
#include "WED_EnumSystem.h"
#include "AptIO.h"
#include "WED_ToolUtils.h"
#include "WED_UIDefs.h"
#include "PlatformUtils.h"

#if ERROR_CHECK
error checking here and in apt-io
#endif

inline Point2	recip(const Point2& pt, const Point2& ctrl) { return pt + Vector2(ctrl,pt); }
inline	void	accum(AptPolygon_t& poly, int code, const Point2& pt, const Point2& ctrl, const set<int>& attrs)
{
	poly.push_back(AptLinearSegment_t());
	poly.back().code = code;
	poly.back().pt = pt;
	poly.back().ctrl = ctrl;
	poly.back().attributes = attrs;
}

inline bool is_curved(int code) { return code == apt_lin_crv || code == apt_rng_crv ||  code == apt_end_crv; }


static void ExportLinearPath(WED_AirportChain * chain, AptPolygon_t& poly)
{
	int n = chain->GetNumPoints();
	int l = n-1;
	bool closed = chain->IsClosed();
	set<int>	no_attrs;
	
	Point2	pt, hi, lo;
	for (int i = 0; i < n; ++i)
	{
		bool last = i == l;
		bool first = i == 0;
		WED_AirportNode * node = dynamic_cast<WED_AirportNode *>(chain->GetNthPoint(i));
		if (node)
		{
			node->GetLocation(pt);
			bool has_hi = node->GetControlHandleHi(hi);
			bool has_lo = node->GetControlHandleLo(lo);
			bool is_split = node->IsSplit();
			if (!closed && last)
			{
				// Special case - write out the last point...that's all we have to do.
				accum(poly, has_lo ? apt_end_crv : apt_end_seg, pt, recip(pt,lo), no_attrs);
			}
			else
			{
				set<int>	attrs, no_attrs;
				set<int>	enums;
				node->GetAttributes(enums);
				for (set<int>::iterator e = enums.begin(); e != enums.end(); ++e)
					attrs.insert(ENUM_Export(*e));
				
				if (first && !closed) is_split = false;	// optimization - even if user split the first point, who cares - hi control is all that used, so we do not
				// need to separately export the low one!
				
				if (is_split)
				{
					// Note that we have to know which is the last of the three we write and write the ring if needed.
					
					if (has_lo)	accum(poly,									  apt_lin_crv, pt, recip(pt, lo)	,		  no_attrs		   );
								accum(poly, (!has_hi && last) ? apt_rng_seg : apt_lin_seg, pt, pt				, has_hi ? no_attrs : attrs);
					if (has_hi) accum(poly,             last  ? apt_rng_crv : apt_lin_crv, pt, hi				,					  attrs);
				}
				else
				{
					// Simple case: we aren't split, so one point does it.
					if (has_hi)	accum(poly, last ? apt_rng_crv : apt_lin_crv, pt, hi, attrs);
					else		accum(poly, last ? apt_rng_seg : apt_lin_seg, pt, pt, attrs);
				}

			}
		}
		
	}
}

static void	AptExportRecursive(WED_Thing * what, AptVector& apts)
{
	WED_Airport *			apt;
	WED_AirportBeacon *		bcn;
	WED_AirportBoundary *	bou;
	WED_AirportChain *		cha;
	WED_AirportSign *		sgn;
	WED_ATCFrequency *		atc;
	WED_Helipad *			hel;
	WED_LightFixture *		lit;
	WED_RampPosition *		ram;
	WED_Runway *			rwy;
	WED_Sealane *			sea;
	WED_Taxiway *			tax;
	WED_TowerViewpoint *	twr;
	WED_Windsock *			win;

	int holes, h;
	
	if (apt = dynamic_cast<WED_Airport *>(what))
	{
		apts.push_back(AptInfo_t());
		apt->Export(apts.back());
	}
	else if (bcn = dynamic_cast<WED_AirportBeacon *>(what))
	{
		bcn->Export(apts.back().beacon);
	}
	else if (bou = dynamic_cast<WED_AirportBoundary *>(what))
	{
		apts.back().boundaries.push_back(AptBoundary_t());		
		bou->Export(apts.back().boundaries.back());
		cha = dynamic_cast<WED_AirportChain*>(bou->GetOuterRing());
		if (cha) ExportLinearPath(cha, apts.back().boundaries.back().area);
		holes = bou->GetNumHoles();
		for (h = 0; h < holes; ++h)
		{
			cha = dynamic_cast<WED_AirportChain *>(bou->GetNthHole(h));
			if (cha) ExportLinearPath(cha, apts.back().boundaries.back().area);			
		}
		return;	// bail out - we already got the children.
	
	}
	else if (cha = dynamic_cast<WED_AirportChain *>(what))
	{
		apts.back().lines.push_back(AptMarking_t());		
		cha->Export(apts.back().lines.back());
		ExportLinearPath(cha, apts.back().lines.back().area);
		return;	// don't waste time with nodes - for speed
	}
	else if (sgn = dynamic_cast<WED_AirportSign *>(what))
	{
		apts.back().signs.push_back(AptSign_t());
		sgn->Export(apts.back().signs.back());
	}
	else if (hel = dynamic_cast<WED_Helipad *>(what))
	{
		apts.back().helipads.push_back(AptHelipad_t());
		hel->Export(apts.back().helipads.back());
	}
	else if (lit = dynamic_cast<WED_LightFixture *>(what))
	{
		apts.back().lights.push_back(AptLight_t());
		lit->Export(apts.back().lights.back());
	}
	else if (ram = dynamic_cast<WED_RampPosition *>(what))
	{
		apts.back().gates.push_back(AptGate_t());
		ram->Export(apts.back().gates.back());
	}
	else if (rwy = dynamic_cast<WED_Runway *>(what))
	{
		apts.back().runways.push_back(AptRunway_t());
		rwy->Export(apts.back().runways.back());
	}
	else if (sea = dynamic_cast<WED_Sealane *>(what))
	{
		apts.back().sealanes.push_back(AptSealane_t());
		sea->Export(apts.back().sealanes.back());
	}
	else if (tax = dynamic_cast<WED_Taxiway *>(what))
	{
		apts.back().taxiways.push_back(AptTaxiway_t());
		tax->Export(apts.back().taxiways.back());
		
		cha = dynamic_cast<WED_AirportChain*>(tax->GetOuterRing());
		if (cha) ExportLinearPath(cha, apts.back().taxiways.back().area);
		holes = tax->GetNumHoles();
		for (h = 0; h < holes; ++h)
		{
			cha = dynamic_cast<WED_AirportChain *>(tax->GetNthHole(h));
			if (cha) ExportLinearPath(cha, apts.back().taxiways.back().area);			
		}
		return; // bail out - we already got the children
	}
	else if (twr = dynamic_cast<WED_TowerViewpoint *>(what))
	{
		twr->Export(apts.back().tower);
	}
	else if (win = dynamic_cast<WED_Windsock *>(what))
	{
		apts.back().windsocks.push_back(AptWindsock_t());
		win->Export(apts.back().windsocks.back());
	} 
	else if (atc = dynamic_cast<WED_ATCFrequency *>(what))
	{
		apts.back().atc.push_back(AptATCFreq_t());
		atc->Export(apts.back().atc.back());
	}
	
	
	int cc = what->CountChildren();
	for (int i = 0; i < cc; ++i)
		AptExportRecursive(what->GetNthChild(i), apts);
}

void	WED_AptExport(
				WED_Thing *		container,
				const char *	file_path)
{
	AptVector	apts;
	AptExportRecursive(container, apts);
	WriteAptFile(file_path,apts);	
}

int		WED_CanExportApt(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	return wrl->CountChildren() > 0;
}

void	WED_DoExportApt(IResolver * resolver)
{
	if (!WED_ValidateApt(resolver)) return;
	
	WED_Thing * wrl = WED_GetWorld(resolver);
	char path[1024];
	strcpy(path,"apt.dat");
	if (GetFilePathFromUser(getFile_Save,"Export airport data as...", "Export", FILE_DIALOG_EXPORT_APTDAT, path, sizeof(path)))
	{
		WED_AptExport(wrl, path);
	}
}

static WED_AirportChain * ImportLinearPath(const AptPolygon_t& path, WED_Archive * archive, WED_Thing * parent, vector<WED_AirportChain *> * chains, void (* print_func)(void *, const char *, ...), void * ref)
{
	WED_AirportChain * chain = NULL;
	WED_AirportChain * ret = NULL;
	for (AptPolygon_t::const_iterator cur = path.begin(); cur != path.end(); ++cur)
	{
		if (chain == NULL)
		{
			chain = WED_AirportChain::CreateTyped(archive);
			chain->SetParent(parent, parent->CountChildren());
			ret = chain;
			if (chains) chains->push_back(chain);
			chain->SetClosed(false);
		}

		// Ben says: Okay, what is about to happen here??  Well, due to my own stupidity, you can't have "split beziers" in an apt.dat file.  (A split bezier is one that has
		// bezier control points on one side of a vertex but not the other, or different bezier control points.  So we encode this in an apt.dat by a number of points with the
		// same location, attributes, but not the same control points.  Typical patterns:
		// - Split bezier, curve on both sides: curve, segment, curve
		// - Split bezier, curve on one side: curve, segment
		// - Split bezier, curve on other side: segment, curve
		// So what we do here is:
		// 1. We find our "low-side" control point first.
		// 2. We scan forward for as many colocated points as possible that form zero-length segments (which we skip).  BUT: two control points on two different points DO
		//    make a loop even if the points are colocated.
		// 3. The final point is used for the "high-side" control point.
				
		// 1. Low side control point
		bool has_lo = is_curved(cur->code);
		Point2	lo_pt;
		if (has_lo) lo_pt = recip(cur->pt, cur->ctrl);

//		printf("Low pt is: %d %lf,%lf (%lf,%lf)\n",cur->code, cur->pt.x,cur->pt.y,cur->ctrl.x,cur->ctrl.y);

		// 2. Iterate forward: run until our point changes or we hit a span that forms a non-zero-length curve.
		AptPolygon_t::const_iterator next = cur, orig = cur, prev = cur;
		++next;
		while (next != path.end() && next->pt == cur->pt &&
			(!is_curved(prev->code) || !is_curved(next->code)))
		{
			prev = next;
//			printf("Skip: %d %lf,%lf (%lf,%lf)\n", next->code, next->pt.x,next->pt.y,next->ctrl.x,next->ctrl.y);
			++next;
		}
//		printf("stopped due to: %d %lf,%lf (%lf,%lf)\n", next->code, next->pt.x,next->pt.y,next->ctrl.x,next->ctrl.y);
		--next;			
		cur = next;
//		printf("hi-end pt is: %d %lf,%lf (%lf,%lf)\n",cur->code, cur->pt.x,cur->pt.y,cur->ctrl.x,cur->ctrl.y);

		
		// 3. High side control point.
		bool has_hi = is_curved(cur->code);
		Point2 hi_pt;
		if (has_hi) hi_pt = cur->ctrl;
		
		// Convert attributes
		set<int>	attrs;
		for (set<int>::const_iterator e = cur->attributes.begin(); e != cur->attributes.end(); ++e)
		{
			int i = ENUM_Import(LinearFeature, *e);
			if (i != -1)
			attrs.insert(i);
			else if (i != 0) print_func(ref,"Ignoring unknown airport line attribute %d.\n", i);
		}
		//	Now we can form a node.
		WED_AirportNode * n = WED_AirportNode::CreateTyped(archive);
		n->SetParent(chain, chain->CountChildren());
		n->SetLocation(cur->pt);
		n->SetSplit(has_lo != has_hi || orig != cur);
		if (has_lo) n->SetControlHandleLo(lo_pt);
		if (has_hi) n->SetControlHandleHi(hi_pt);
		
		n->SetAttributes(attrs);
		
		if (cur->code == apt_end_seg || cur->code == apt_end_crv)
		{
			chain = NULL;
		}
		else if (cur->code == apt_rng_seg || cur->code == apt_rng_crv)
		{	
			chain->SetClosed(true); 
			chain = NULL; 
		}
	}
	return ret;
}

struct	LazyLog_t {
	const char *	path;
	FILE *			fi;
};

void LazyPrintf(void * ref, const char * fmt, ...)
{
	va_list	arg;
	va_start(arg, fmt);
	LazyLog_t * l = (LazyLog_t *) ref;
	if (l->fi == NULL) l->fi = fopen(l->path,"w");
	if (l->fi) vfprintf(l->fi,fmt,arg);
}

void	WED_AptImport(
				WED_Archive *	archive,
				WED_Thing *		container,
				const char *	file_path)
{
	AptVector	apts;
	ReadAptFile(file_path, apts);
	
	char path[1024];
	strcpy(path,file_path);
	strcat(path,".log");
	
	LazyLog_t log = { path, NULL };
	
	for (AptVector::iterator apt = apts.begin(); apt != apts.end(); ++apt)
	{
		ConvertForward(*apt);
		
		WED_Airport * new_apt = WED_Airport::CreateTyped(archive);
		new_apt->SetParent(container,container->CountChildren());
		new_apt->Import(*apt, LazyPrintf, &log);

		for (AptRunwayVector::iterator rwy = apt->runways.begin(); rwy != apt->runways.end(); ++rwy)
		{
			WED_Runway *		new_rwy = WED_Runway::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			new_rwy->SetParent(new_apt,new_apt->CountChildren());
			source->SetParent(new_rwy,0);
			target->SetParent(new_rwy,1);
			new_rwy->Import(*rwy, LazyPrintf, &log);
		}

		for (AptSealaneVector::iterator sea = apt->sealanes.begin(); sea != apt->sealanes.end(); ++sea)
		{
			WED_Sealane *		new_sea = WED_Sealane::CreateTyped(archive);
			WED_RunwayNode *	source = WED_RunwayNode::CreateTyped(archive);
			WED_RunwayNode *	target = WED_RunwayNode::CreateTyped(archive);
			new_sea->SetParent(new_apt,new_apt->CountChildren());
			source->SetParent(new_sea,0);
			target->SetParent(new_sea,1);
			new_sea->Import(*sea, LazyPrintf, &log);
		}

		for (AptHelipadVector::iterator hel = apt->helipads.begin(); hel != apt->helipads.end(); ++hel)
		{
			WED_Helipad * new_hel = WED_Helipad::CreateTyped(archive);
			new_hel->SetParent(new_apt,new_apt->CountChildren());
			new_hel->Import(*hel, LazyPrintf, &log);
		}

		for (AptTaxiwayVector::iterator tax = apt->taxiways.begin(); tax != apt->taxiways.end(); ++tax)
		{
			WED_Taxiway * new_tax = WED_Taxiway::CreateTyped(archive);
			new_tax->SetParent(new_apt,new_apt->CountChildren());
			new_tax->Import(*tax, LazyPrintf, &log);
			
			if (!ImportLinearPath(tax->area, archive, new_tax, NULL, LazyPrintf, &log))
			{
				new_tax->SetParent(NULL,0);
				new_tax->Delete();
			}
		}

		for (AptBoundaryVector::iterator bou = apt->boundaries.begin(); bou != apt->boundaries.end(); ++bou)
		{
			WED_AirportBoundary * new_bou = WED_AirportBoundary::CreateTyped(archive);
			new_bou->SetParent(new_apt,new_apt->CountChildren());
			new_bou->Import(*bou, LazyPrintf, &log);
			
			if (!ImportLinearPath(bou->area, archive, new_bou, NULL, LazyPrintf, &log))
			{
				new_bou->SetParent(NULL,0);
				new_bou->Delete();
			}
		}

		for (AptMarkingVector::iterator lin = apt->lines.begin(); lin != apt->lines.end(); ++lin)
		{			
			vector<WED_AirportChain *> new_lin;
			ImportLinearPath(lin->area, archive, new_apt, &new_lin, LazyPrintf, &log);
			for (vector<WED_AirportChain *>::iterator li = new_lin.begin(); li != new_lin.end(); ++li)
				(*li)->Import(*lin, LazyPrintf, &log);
		}

		for (AptLightVector::iterator lit = apt->lights.begin(); lit != apt->lights.end(); ++lit)
		{
			WED_LightFixture * new_lit = WED_LightFixture::CreateTyped(archive);
			new_lit->SetParent(new_apt,new_apt->CountChildren());
			new_lit->Import(*lit, LazyPrintf, &log);
		}
		
		for (AptSignVector::iterator sin = apt->signs.begin(); sin != apt->signs.end(); ++sin)
		{
			WED_AirportSign * new_sin = WED_AirportSign::CreateTyped(archive);
			new_sin->SetParent(new_apt,new_apt->CountChildren());
			new_sin->Import(*sin, LazyPrintf, &log);
		}

		for (AptGateVector::iterator gat = apt->gates.begin(); gat != apt->gates.end(); ++gat)
		{
			WED_RampPosition * new_gat = WED_RampPosition::CreateTyped(archive);
			new_gat->SetParent(new_apt,new_apt->CountChildren());
			new_gat->Import(*gat, LazyPrintf, &log);
		}
		
		if (apt->tower.draw_obj != -1)
		{
			WED_TowerViewpoint * new_twr = WED_TowerViewpoint::CreateTyped(archive);
			new_twr->SetParent(new_apt,new_apt->CountChildren());
			new_twr->Import(apt->tower, LazyPrintf, &log);
		}
		
		if (apt->beacon.color_code != apt_beacon_none)
		{
			WED_AirportBeacon * new_bea = WED_AirportBeacon::CreateTyped(archive);
			new_bea->SetParent(new_apt,new_apt->CountChildren());
			new_bea->Import(apt->beacon, LazyPrintf, &log);
		}
	
		for (AptWindsockVector::iterator win = apt->windsocks.begin(); win != apt->windsocks.end(); ++win)
		{
			WED_Windsock * new_win = WED_Windsock::CreateTyped(archive);
			new_win->SetParent(new_apt,new_apt->CountChildren());
			new_win->Import(*win, LazyPrintf, &log);
		}

		for (AptATCFreqVector::iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
		{
			WED_ATCFrequency * new_atc = WED_ATCFrequency::CreateTyped(archive);
			new_atc->SetParent(new_apt,new_apt->CountChildren());
			new_atc->Import(*atc, LazyPrintf, &log);
		}
	}
	
	if (log.fi)
	{
		fclose(log.fi);
		DoUserAlert("There were problems during the import.  A log file has been created in the same file as the apt.dat file.");
	}
}

int		WED_CanImportApt(IResolver * resolver)
{
	return 1;	
}

void	WED_DoImportApt(IResolver * resolver, WED_Archive * archive)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	char path[1024];
	strcpy(path,"");
	if (GetFilePathFromUser(getFile_Open,"Import apt.dat...", "Import", FILE_DIALOG_IMPORT_APTDAT, path, sizeof(path)))
	{
		wrl->StartOperation("Import apt.dat");
		WED_AptImport(archive, wrl, path);
		WED_SetAnyAirport(resolver);
		wrl->CommitOperation();
	}	
}

static set<string>	s_used_rwy;
static set<string>	s_used_hel;

static WED_Thing * ValidateRecursive(WED_Thing * who)
{
	string name, n1, n2;
	string::size_type p;
	int i;
	who->GetName(name);
	string msg;
	if (who->GetClass() == WED_Runway::sClass || who->GetClass() == WED_Sealane::sClass)
	{
		if (s_used_rwy.count(name))	msg = "The runway/sealane name '" + name + "' has already been used.";
		s_used_rwy.insert(name);
		p = name.find("/");
		if (p != name.npos)
		{
			n1 = name.substr(0,p);
			n2 = name.substr(p+1);
		} else
			n1 = name;
			
		int suf1 = 0, suf2 = 0;
		int	num1 = -1, num2 = -1;
		
		if (n1.empty())	msg = "The runway/sealane '" + name + "' has an empty low-end name.";
		else {
			int suffix = n1[n1.length()-1];
			if (suffix < '0' || suffix > '9')
			{
				if (suffix == 'L' || suffix == 'R' || suffix == 'C') suf1 = suffix;
				else msg = "The runway/sealane '" + name + "' has an illegal suffix for the low-end runway.";				
				n1.erase(n1.length()-1);				
			}
			
			for (i = 0; i < n1.length(); ++i)			
			if (n1[i] < '0' || n1[i] > '9') 
			{
				msg = "The runway/sealane '" + name + "' has an illegal characters in its low-end name.";
				break;
			}
			if (i == n1.length())
			{
				num1 = atoi(n1.c_str());
			}
			if (num1 < 1 || num1 > 36)
			{
				msg = "The runway/sealane '" + name + "' has an illegal low-end number, which must be between 1 and 36.";
				num1 = -1;
			}
		}
		
		if (p != name.npos)
		{
			if (n2.empty())	msg = "The runway/sealane '" + name + "' has an empty high-end name.";
			else {
				int suffix = n2[n2.length()-1];
				if (suffix < '0' || suffix > '9')
				{
					if (suffix == 'L' || suffix == 'R' || suffix == 'C') suf2 = suffix;
					else msg = "The runway/sealane '" + name + "' has an illegal suffix for the high-end runway.";				
					n2.erase(n2.length()-1);				
				}
				
				for (i = 0; i < n2.length(); ++i)			
				if (n2[i] < '0' || n2[i] > '9') 
				{
					msg = "The runway/sealane '" + name + "' has an illegal characters in its high-end name.";
					break;
				}
				if (i == n2.length())
				{
					num2 = atoi(n2.c_str());
				}
				if (num2 < 19 || num2 > 36)
				{
					msg = "The runway/sealane '" + name + "' has an illegal high-end number, which must be between 19 and 36.";
					num2 = -1;
				}
			}
		}
		
		if (suf1 != 0 && suf2 != 0)
		{
			if ((suf1 == 'L' && suf2 != 'R') ||
				(suf1 == 'R' && suf2 != 'L') ||
				(suf1 == 'C' && suf2 != 'C'))
					msg = "The runway/sealane '" + name + "' has mismatched suffixes - check L vs R, etc.";		
		}
		if (num1 != -1 && num2 != -1)
		{
			if (num2 < num1)
				msg = "The runway/sealane '" + name + "' has mismatched runway numbers - the low number must be first.'";
			else if (num2 != num1 + 18)
				msg = "The runway/sealane '" + name + "' has mismatched runway numbers - high end is not the reciprocal of the low-end.";
		}
		
		if (msg.empty())
		{
			WED_GISLine_Width * lw = dynamic_cast<WED_GISLine_Width *>(who);
			if (lw->GetWidth() < 1.0) msg = "The runway/sealane '" + name + "' must be at least one meter wide.";
			
			WED_Runway * rwy = dynamic_cast<WED_Runway *>(who);
			if (rwy)
			{			
				if (rwy->GetDisp1() + rwy->GetDisp2() > rwy->GetLength()) msg = "The runway/sealane '" + name + "' has overlapping displaced threshholds.";
			}	
		}
	}
	if (who->GetClass() == WED_Helipad::sClass)
	{
		if (s_used_hel.count(name))	msg = "The helipad name '" + name + "' has already been used.";
		s_used_hel.insert(name);
	
		n1 = name;
		if (n1.empty())	msg = "The selected helipad has no name.";
		else {
			if (n1[0] != 'H')	msg = "The helipad '" + name + "' does not start with the letter H.";
			else {
				n1.erase(0,1);
				for (int i = 0; i < n1.length(); ++i)
				{
					if (n1[i] < '0' || n1[i] > '9')
					{
						msg = "The helipad '" + name + "' conntains illegal characters in its name.  It must be in the form H<number>.";
						break;
					}
				}
			}
		}
		if (msg.empty())
		{
			WED_Helipad * heli = dynamic_cast<WED_Helipad *>(who);
			if (heli->GetWidth() < 1.0) msg = "The helipad '" + name + "' is less than one meter wide.";
			if (heli->GetLength() < 1.0) msg = "The helipad '" + name + "' is less than one meter long.";
		}
	}
	if(who->GetClass() == WED_Airport::sClass)
	{
		s_used_hel.clear();
		s_used_rwy.clear();	
	}

	if (!msg.empty())
	{
		DoUserAlert(msg.c_str());
		return who;
	}

	int nn = who->CountChildren();
	for (int n = 0; n < nn; ++n)
	{
		WED_Thing * fail = ValidateRecursive(who->GetNthChild(n));
		if (fail) return fail;
	}
	return NULL;
}

bool	WED_ValidateApt(IResolver * resolver)
{	
	s_used_hel.clear();
	s_used_rwy.clear();
	WED_Thing * wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	WED_Thing * bad_guy = ValidateRecursive(wrl);
	if (bad_guy)
	{
		wrl->StartOperation("Select Invalid");
		sel->Select(bad_guy);
		wrl->CommitOperation();
		return false;
	}
	return true;	
}

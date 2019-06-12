/*
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_ResourceMgr_H
#define WED_ResourceMgr_H

/*
	WED_ResourceMgr - THEORY OF OPERATION

	This class maintains a lazy-create cache or art asset previews.  It currently supports two art asset classes:
	- OBJ
	- POL

	In the case of OBJ, we use the OBJ_ package for preview and data management, thus an OBJ preview is an XObj8 struct.
	For .pol since there is no package for .pol preview (since it is somewhat trivial) we define a struct.

	HERE'S THE HACK

	Traditionally the UI interface for WED is firewalled off from the document class/implementation using a purely virtual
	abstract interface.  (See ILibrarian.h for example.)  But...I have not had the time to do this here yet.  So
	WED_LibraryMgr is used directly as sort of its own interface.  This is definitely a hack, it's definitely "wrong", but
	it's also definitely not very dangerous at this point in the code's development - that is, WED is not so big that this
	represents a scalability issue.

*/

#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"
#include "IBase.h"
#include "XObjDefs.h"
#include "CompGeomDefs2.h"
#include <list>

class	WED_LibraryMgr;

struct	pol_info_t {
	string		base_tex; //Relative path
	bool		hasDecal;
	float		proj_s;
	float		proj_t;
	bool		kill_alpha;
	bool		wrap;
	string		group;
	int			group_offset;
	float		latitude;
	float		longitude;
	float		height_Meters;
	int			ddsHeight_Pxls;
	vector <Bbox2>	mSubBoxes;       // for subTexture selection in PreviewPanel
	Bbox2		mUVBox;              // set by PreviewPanel from selected subTexture
};

#include "WED_FacadePreview.h"

struct fac_info_t : public REN_FacadeLOD_t {

	fac_info_t() { is_new = false ; is_ring = true; doubled = two_sided = false;  min_floors = 1; max_floors  = 999; has_roof = false; 
						noroofmesh = nowallmesh = false; }

	bool			is_new;       // set if version 1000, aka type 2
	string		wall_tex;
	string		roof_tex;
	bool			is_ring; 	  // can be drawn as open polygon
	bool			two_sided;
	
	// Facade Scrapers
	vector<REN_facade_scraper_t>	scrapers;
	
	// V1 only
	// vector<FacadeLOD_t>		lods;  // WED does not recognize anything but the LOD that starts at 0
	
	// V2 only
	bool					noroofmesh;
	bool					nowallmesh;
	list<REN_facade_floor_t>	floors;
	vector<string>		objs;			// names of type 2 objects
	float					roof_scale_s;
	float					roof_scale_t;
	
	// WED only
	vector<string>	wallName;      // wall names, for property window etc
	vector<string>	wallUse;       // official width range supported by this wall
	string         h_range;       // official heights (or height range) of the facade
};

struct	lin_info_t {
	string		base_tex;
	float		scale_s;
	float		scale_t;
	float		eff_width;
	float		rgb[3];
	vector<float>	s1,sm,s2;
};

struct	str_info_t {
	float		offset;
	float		rotation;
	vector<string> objs;
};

struct	road_info_t {
	struct vroad_t {
		string 	description;      // text to display in menu's
		int		rd_type;				// index into road_types
	};
	struct road_t {
		int		tex_idx;				// index into textures[]
		float		width, length;    // texture scaling
		float 	s_left, s_right;	// st coordinates on texture (t is always 0 to 1)
	};
	
	map<int, vroad_t>	vroad_types;
	map<int, road_t>	road_types;
	vector<string>		textures;
};

struct agp_t {
	struct obj {
		float		x,y,r;			// annotation position
		int		show_lo,show_hi;
		string	name;
	};
	string			base_tex;
	string			mesh_tex;
	int				hide_tiles;
	vector<float>	tile;	// the base tile in x,y,s,t quads.
	vector<obj>		objs;
};


class WED_ResourceMgr : public GUI_Broadcaster, public GUI_Listener, public virtual IBase {
public:

					 WED_ResourceMgr(WED_LibraryMgr * in_library);
					~WED_ResourceMgr();

			void	Purge(void);

			bool	GetFac(const string& path, fac_info_t const *& info, int variant =0);
			bool	GetPol(const string& path, pol_info_t const *& info);
			bool 	SetPolUV(const string& path, Bbox2 box);
			bool	GetLin(const string& path, lin_info_t const *& info);
			bool	GetStr(const string& path, str_info_t const *& info);
			bool	GetFor(const string& path, XObj8 const *& obj);
			int		GetNumVariants(const string& path);

			//path is a RELATIVE PATH
			void	WritePol(const string& abspath, const pol_info_t& out_info); // side note: shouldn't this be in_info?
			bool	GetObj(const string& path, XObj8 const *& obj, int variant = 0);
			bool	GetObjRelative(const string& obj_path, const string& parent_path, XObj8 const *& obj);
			bool	GetAGP(const string& path, agp_t& out_info);
			bool	GetRoad(const string& path, road_info_t& out_info);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

			XObj8 * LoadObj(const string& abspath);
	
	unordered_map<string,vector<fac_info_t *> > mFac;
	unordered_map<string,pol_info_t>		mPol;
	unordered_map<string,lin_info_t>		mLin;
	unordered_map<string,str_info_t>		mStr;
	unordered_map<string,const XObj8 *>	mFor;
	unordered_map<string,vector<const XObj8 *> > mObj;
	unordered_map<string,agp_t>				mAGP;
#if ROAD_EDITING
	unordered_map<string,road_info_t>		mRoad;
#endif	
	WED_LibraryMgr *				mLibrary;
};	

#endif /* WED_ResourceMgr_H */

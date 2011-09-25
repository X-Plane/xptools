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
class	WED_LibraryMgr;

struct	XObj8;

struct	pol_info_t {
	string		base_tex;
	float		proj_s;
	float		proj_t;
	bool		kill_alpha;
	bool		wrap;
	string		group;
	int			group_offset;
};

struct	fac_info_t {
	bool			ring;
	bool			roof;
	bool			modern;
	vector<string>	walls;
};

#if AIRPORT_ROUTING
struct agp_t {
	struct obj {
		double  x,y,r;			// annotation position
		string	name;
	};
	string			base_tex;
	vector<double>	tile;	// the base tile in x,y,s,t quads.
	vector<obj>		objs;
};
#endif


class WED_ResourceMgr : public GUI_Broadcaster, public GUI_Listener, public virtual IBase {
public:

					 WED_ResourceMgr(WED_LibraryMgr * in_library);
					~WED_ResourceMgr();

			void	Purge(void);

			bool	GetFac(const string& path, fac_info_t& out_info);
			bool	GetPol(const string& path, pol_info_t& out_info);
			void	MakePol(const string& path, const pol_info_t& out_info);
			bool	GetObj(const string& path, XObj8 *& obj);
			bool	GetObjRelative(const string& obj_path, const string& parent_path, XObj8 *& obj);
#if AIRPORT_ROUTING
			bool	GetAGP(const string& path, agp_t& out_info);
#endif			

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

	map<string,fac_info_t>		mFac;
	map<string,pol_info_t>		mPol;
	map<string,XObj8 *>			mObj;
#if AIRPORT_ROUTING	
	map<string,agp_t>			mAGP;
#endif	
	WED_LibraryMgr *			mLibrary;
};

#endif /* WED_ResourceMgr_H */

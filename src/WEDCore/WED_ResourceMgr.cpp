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

#include "WED_ResourceMgr.h"
#include "WED_Messages.h"
#include "WED_LibraryMgr.h"
#include "MemFileUtils.h"
#include "XObjReadWrite.h"
#include "XObjDefs.h"
#include "ObjConvert.h"
#include "FileUtils.h"
#include "WED_PackageMgr.h"
#include "CompGeomDefs2.h"

static void process_texture_path(const string& path_of_obj, string& path_of_tex)
{
	string root(path_of_tex);
	string parent(path_of_obj);

	string::size_type pp = parent.find_last_of("\\:/");
	if(pp == parent.npos) parent.clear();
	else				  parent.erase(pp+1);

	pp = root.find_last_of(".");
	if(pp != root.npos) root.erase(pp);
											path_of_tex = parent + root + ".dds";
	if(!FILE_exists(path_of_tex.c_str()))	path_of_tex = parent + root + ".png";
	if(!FILE_exists(path_of_tex.c_str()))	path_of_tex = parent + root + ".bmp";
}

WED_ResourceMgr::WED_ResourceMgr(WED_LibraryMgr * in_library) : mLibrary(in_library)
{
}

WED_ResourceMgr::~WED_ResourceMgr()
{
	Purge();
	RemoveTmpFiles();
}

void	WED_ResourceMgr::Purge(void)
{
	for(map<string, XObj8 *>::iterator i = mObj.begin(); i != mObj.end(); ++i)
		delete i->second;
	mPol.clear();
	mObj.clear();
}

bool	WED_ResourceMgr::GetObjRelative(const string& obj_path, const string& parent_path, XObj8 *& obj)
{
	if(GetObj(obj_path,obj))
		return true;
	string lib_key = parent_path + string("\n") + obj_path;
	if(GetObj(lib_key,obj))
		return true;
	string full_parent = mLibrary->GetResourcePath(parent_path);
	string::size_type s = full_parent.find_last_of("\\/:");
	if(s == full_parent.npos) full_parent.clear(); else full_parent.erase(s+1);
	string p = full_parent + obj_path;

	obj = new XObj8;
	if(!XObj8Read(p.c_str(),*obj))
	{
		XObj obj7;
		if(XObjRead(p.c_str(),obj7))
		{
			Obj7ToObj8(obj7,*obj);
		}
		else
		{
			delete obj;
			obj = NULL;
			return false;
		}
	}

	process_texture_path(p,obj->texture);
	process_texture_path(p,obj->texture_draped);

	mObj[lib_key] = obj;
	return true;
	
	
}

bool	WED_ResourceMgr::GetObj(const string& path, XObj8 *& obj)
{
	map<string,XObj8 *>::iterator i = mObj.find(path);
	if(i != mObj.end())
	{
		obj = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	if (!p.size()) p = mLibrary->CreateLocalResourcePath(path);

	obj = new XObj8;
	if(!XObj8Read(p.c_str(),*obj))
	{
		XObj obj7;
		if(XObjRead(p.c_str(),obj7))
		{
			Obj7ToObj8(obj7,*obj);
		}
		else
		{
			delete obj;
			obj = NULL;
			return false;
		}
	}

	process_texture_path(p,obj->texture);
	process_texture_path(p,obj->texture_draped);

	mObj[path] = obj;
	return true;
}

bool	WED_ResourceMgr::GetPol(const string& path, pol_info_t& out_info)
{
	map<string,pol_info_t>::iterator i = mPol.find(path);
	if(i != mPol.end())
	{
		out_info = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * pol = MemFile_Open(p.c_str());
	if(!pol) return false;


	MFScanner	s;
	MFS_init(&s, pol);

	int versions[] = { 850, 0 };

	if(!MFS_xplane_header(&s,versions,"DRAPED_POLYGON",NULL))
	{
		MemFile_Close(pol);
		return false;
	}

	out_info.base_tex.clear();
	out_info.proj_s=1000;
	out_info.proj_t=1000;
	out_info.kill_alpha=false;
	out_info.wrap=false;

	while(!MFS_done(&s))
	{
		// TEXTURE <texname>
		if (MFS_string_match(&s,"TEXTURE", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=true;
		}
		// SCALE <ms> <mt>
		else if (MFS_string_match(&s,"SCALE", false))
		{
			out_info.proj_s = MFS_double(&s);
			out_info.proj_t = MFS_double(&s);
		}
		// TEXTURE_NOWRAP <texname>
		else if (MFS_string_match(&s,"TEXTURE_NOWRAP", false))
		{
			MFS_string(&s,&out_info.base_tex);
			out_info.wrap=false;
		}
		// NO_ALPHA
		else if (MFS_string_match(&s,"NO_ALPHA", true))
		{
			out_info.kill_alpha=true;
		}
		// LAYER_GROUP
		else if (MFS_string_match(&s,"LAYER_GROUP",false))
		{
			MFS_string(&s,&out_info.group);
			out_info.group_offset = MFS_int(&s);
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(pol);

	process_texture_path(p,out_info.base_tex);

	mPol[path] = out_info;
	return true;
}

void WED_ResourceMgr::MakePol(const string& path, const pol_info_t& out_info)
{
	map<string,pol_info_t>::iterator i = mPol.find(path);
	if(i != mPol.end())
	{
		return;
	}

	string p = mLibrary->CreateLocalResourcePath(path);
	//Makes sure that the file will end in .pol
	string pol = "pol";
	p.replace(p.length()-3,3,pol);
	FILE * fi = fopen(p.c_str(), "w");
	if(!fi)	return;
	fprintf(fi,"A\n850\nDRAPED_POLYGON\n\n");
	
	fprintf(fi,out_info.wrap ? "TEXTURE %s\n" : "TEXTURE_NOWRAP %s\n", out_info.base_tex.c_str());
	fprintf(fi,"SCALE %lf %lf\n",out_info.proj_s,out_info.proj_t);
		/*float		latitude;
	float		longitude;
	double		height_Meters;
	int			ddsHeight_Pxls;*/
	fprintf(fi,"LOAD_CENTER %lf %lf %f %d", out_info.latitude, out_info.longitude,out_info.height_Meters,out_info.ddsHeight_Pxls);
	if(out_info.kill_alpha)
		fprintf(fi,"NO_ALPHA\n");
	fclose(fi);	
	gPackageMgr->Rescan();
}



void	WED_ResourceMgr::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if (inMsg == msg_SystemFolderChanged ||
		inMsg == msg_SystemFolderUpdated)
	{
		Purge();
	}
}

bool	WED_ResourceMgr::GetFac(const string& path, fac_info_t& out_info)
{
	out_info.ring = true;
	out_info.roof = true;
	out_info.modern = true;
	
	map<string,fac_info_t>::iterator i = mFac.find(path);
	if(i != mFac.end())
	{
		out_info = i->second;
		return true;
	}

	string p = mLibrary->GetResourcePath(path);
	MFMemFile * fac = MemFile_Open(p.c_str());
	if(!fac) return false;


	MFScanner	s;
	MFS_init(&s, fac);

	int versions[] = { 800,900,1000, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"FACADE",NULL)) == 0)
	{
		MemFile_Close(fac);
		return false;
	}
	out_info.modern = v > 800;

	out_info.ring = true;
	out_info.roof = false;

	while(!MFS_done(&s))
	{
		// RING
		if (MFS_string_match(&s,"RING", false))
		{
			out_info.ring = MFS_int(&s) > 0;
		}
		// ROOF
		else if (MFS_string_match(&s,"ROOF", false))
		{
			out_info.roof = true;
		}
		else if (MFS_string_match(&s,"ROOF_SCALE", false))
		{
			out_info.roof = true;
		}
		// ROOF_HEIGHT
		else if (MFS_string_match(&s,"ROOF_HEIGHT", false))
		{
			out_info.roof = true;
		}
		// WALL min max min max name
		else if (MFS_string_match(&s,"WALL",false))
		{
			MFS_double(&s);
			MFS_double(&s);
			MFS_double(&s);
			MFS_double(&s);
			out_info.walls.push_back(string());
			MFS_string(&s,&out_info.walls.back());
		} 
		else if(MFS_string_match(&s,"FLOOR",false))
		{
			out_info.walls.clear();
		}

		MFS_string_eol(&s,NULL);
	}
	MemFile_Close(fac);

	mFac[path] = out_info;
	return true;
}

inline void	do_rotate(int n, double& io_x, double& io_y)
{
	Vector2 v(io_x,io_y);
	while(n > 0)
	{
		v = v.perpendicular_cw();
		--n;
	}
	io_x = v.dx;
	io_y = v.dy;
}

#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>

#define TMP_PREFIX "WED___"
#define TPR 6           // # of trees shown in a row

struct tree_t {
	float s,t,w,y; 		// texture coordinates of tree
	float o;            // offset of tree center line (where the quads inersect)
	float pct;          // relative occurence percentage for this tree
	float hmin,hmax;    // height range for this tree in meters
	int q;				// number of quads the tree is constructed of
};

// create a .obj that is "similar" than what a .for would expand to XP, for previewing forest

string WED_ResourceMgr::MakeTmpForestObj(const string& res_nam)
{
	string obj_fnam = TMP_PREFIX + res_nam;
	obj_fnam.replace(obj_fnam.end()-4,obj_fnam.end(),".obj");

	// sanitize resouce name to constitude a plain file name
	for (int i=0; i < obj_fnam.size()-4; ++i)
		if (obj_fnam.at(i) == '/' || obj_fnam.at(i) == '\\' || obj_fnam.at(i) == ':')
			obj_fnam.replace(i,1,"_");

	string res_path = mLibrary->GetResourcePath(res_nam);
	string obj_path = mLibrary->CreateLocalResourcePath(obj_fnam);

	ifstream for_fi (res_path.c_str());
	if (for_fi)
	{
		ofstream obj_fi (obj_path.c_str());
		if (obj_fi)
		{
			obj_fi << "A\n800\nOBJ\n\n";
			
			string line;
			vector <tree_t> tree;
			float scale_x=256, scale_y=256, space_x=30, space_y=30, rand_x=0, rand_y=0;
			int quads=0;

			while ( getline (for_fi,line) )
			{
				if (line.find("TEXTURE ") == 0) 
				{
					// get the path of the texture, relative to the original .for file
					line.erase(0,line.find_first_not_of(" \t",7));
		
					// now get the relative path from where we create the obj_fnam.obj to where the texture actually is
					process_texture_path(res_path, line);
					line = gPackageMgr->ReducePath("", line);

					obj_fi << "TEXTURE " << line << endl;
				}
				else if (line.find("SCALE_X") == 0)
					sscanf(line.c_str()+7,"%f", &scale_x);
				else if (line.find("SCALE_Y") == 0)
					sscanf(line.c_str()+7,"%f", &scale_y);
				else if (line.find("SPACING") == 0)
					sscanf(line.c_str()+7,"%f%f", &space_x, &space_y);
				else if (line.find("RANDOM") == 0)
					sscanf(line.c_str()+7,"%f%f", &rand_x, &rand_y);
				else if (line.find("TREE") == 0)
				{
					tree_t t;
					int i = sscanf(line.c_str()+4,"%f%f%f%f%i%f%f%f%i", 
					                 &t.s,&t.t,&t.w,&t.y,&t.o,&t.pct,&t.hmin,&t.hmax,&t.q);
					if (i==9 && t.w > 1.0 && t.y > 1.0 )         // yup, there are some .for with zero size tree's in XP10
					{
						tree.push_back(t);
						quads += t.q;
					}
				}
			}
			// now we have one of each tree. Like on the ark. Or maybe half that :)
			// expand that to full forest of TPS * TPS trees, populated with all the varieties there are
			int varieties =  tree.size();
			vector <tree_t> treev = tree;
			tree.clear();
			quads = 0;

#if 0		// truely random tree choice, taken into account each tree's relative percentage
			// it works, but not so perfect for a forest with a relatively small number of tree's
			// e.g. a 36 tree forest with one tree ocurring at 3.5% may have either 0, 1 or 2 of that kind
					
			for (int i=0; i<TPR*TPR; ++i)
			{
				int species = 0;
					float prob = (100.0*rand())/RAND_MAX;
				for (species=varieties-1; species>0; --species)
					if (prob < treev[species].pct)
						break;
					else
						prob-=treev[species].pct;
				// if the pct for all tree's don't add up too 100% - species #0 will make up for it. 
				// XP seems to do the same.
				
				tree.push_back(treev[species]);
				quads += treev[species].q;
			}
#else
			int species[TPR*TPR] = {};
			
			for (int i=varieties-1; i>0; --i)
				for (int j=0; j<round(treev[i].pct/100.0*TPR*TPR); ++j)
				{
					int cnt=10;     // needed in case the tree percentages add up to more than 100%
					do
					{
						int where = ((float) TPR*TPR*rand())/RAND_MAX;
						if(where >= 0 && where < TPR*TPR && !species[where]) 
						{ 
							species[where] = i;
							break;
						}
					} while (--cnt);
				}
			for (int i=0; i<TPR*TPR; ++i)
			{
				tree.push_back(treev[species[i]]);
				quads += treev[species[i]].q;
			}
#endif
				
			obj_fi << "POINT_COUNTS " << 4*quads << " 0 0 " << 6*quads << endl;
			
			char buf[100];
			for (int i = 0; i < tree.size(); ++i)
			{
				float t_h = (tree[i].hmin+tree[i].hmax);                       // height not randomized
				float t_w = t_h * tree[i].w/tree[i].y /2.0;                    // half width of tree
				float t_x = (i % TPR) * space_x + ((2.0*rand_x*rand())/RAND_MAX-0.5);  // tree position
				float t_y = (i / TPR) * space_y + ((2.0*rand_y*rand())/RAND_MAX-0.5);
				float rot_r = ((float) rand())/RAND_MAX;

				for (int j=0; j<tree[i].q; ++j)
				{
					float rot = M_PI*(rot_r+j/(float) tree[i].q);        // tree rotation
					float x = t_w * sin(rot);
					float z = t_w * cos(rot);
					
					sprintf(buf,"VT %6.2f %6.2f %6.2f  0.0 1.0 0.0  %6.4f %6.4f\n",
									t_x-x, 0.0, t_y-z, tree[i].s/scale_x, tree[i].t/scale_y);
					obj_fi << buf;
					sprintf(buf,"VT %6.2f %6.2f %6.2f  0.0 1.0 0.0  %6.4f %6.4f\n",
									t_x+x, 0.0, t_y+z, (tree[i].s+tree[i].w)/scale_x, tree[i].t/scale_y);
					obj_fi << buf;
					sprintf(buf,"VT %6.2f %6.2f %6.2f  0.0 1.0 0.0  %6.4f %6.4f\n",
									t_x+x, t_h, t_y+z, (tree[i].s+tree[i].w)/scale_x, (tree[i].t+tree[i].y)/scale_y);
					obj_fi << buf;
					sprintf(buf,"VT %6.2f %6.2f %6.2f  0.0 1.0 0.0  %6.4f %6.4f\n",
									t_x-x, t_h, t_y-z, tree[i].s/scale_x, (tree[i].t+tree[i].y)/scale_y);
					obj_fi << buf;
				}
			}
			int seq[6] = {0,1,2,0,2,3};
			for (int i = 0; i < 6*quads; ++i)
			{
				obj_fi << "IDX " << 4*(i/6)+seq[i%6] << endl;
			}
			obj_fi << "ATTR_LOD 0 1000\n";
			obj_fi << "ATTR_no_cull\n";
			obj_fi << "TRIS 0 " << 6*quads << endl;
			obj_fi.close();
		}
		for_fi.close();
		return obj_fnam;
	}
	else
		return "";
}

bool WED_ResourceMgr::RemoveTmpFileCB(const char * fn, bool isDir, void * ref)
{
	if (!isDir)
		if (strncmp(fn, TMP_PREFIX, strlen(TMP_PREFIX)) == 0)
			if (strcmp(fn+strlen(fn)-4, ".obj") == 0)
			{
				string fn_path = (char *) ref;
				fn_path += fn;
				remove (fn_path.c_str());
			}
	return false;
}


// create a .obj that is "similar" than what a .for would create in XP, for previewing forest
void WED_ResourceMgr::RemoveTmpFiles()
{
	string obj_path = mLibrary->CreateLocalResourcePath("");

	MF_IterateDirectory(obj_path.c_str(),RemoveTmpFileCB, (void *) obj_path.c_str());
}

#if AIRPORT_ROUTING
bool	WED_ResourceMgr::GetAGP(const string& path, agp_t& out_info)
{
	map<string,agp_t>::iterator i = mAGP.find(path);
	if(i != mAGP.end())
	{
		out_info = i->second;
		return true;
	}
	
	string p = mLibrary->GetResourcePath(path);
	MFMemFile * agp = MemFile_Open(p.c_str());
	if(!agp) return false;


	MFScanner	s;
	MFS_init(&s, agp);

	int versions[] = { 1000, 0 };
	int v;
	if((v=MFS_xplane_header(&s,versions,"AG_POINT",NULL)) == 0)
	{
		MemFile_Close(agp);
		return false;
	}
	
	double tex_s = 1.0, tex_t = 1.0;		// these scale from pixels to UV coords
	double tex_x = 1.0, tex_y = 1.0;		// meters for tex, x & y
	int	 rotation = 0;
	double anchor_x = 0.0, anchor_y = 0.0;
	out_info.hide_tiles = 0;
	vector<string>	obj_paths;

	bool is_mesh_shader = false;

	while(!MFS_done(&s))
	{
		if(MFS_string_match(&s,"TEXTURE",false))
		{
			string tex;
			MFS_string(&s,&tex);
			if(is_mesh_shader)
			{
				out_info.mesh_tex = tex;
				process_texture_path(p,out_info.mesh_tex);
			}
			else
			{
				out_info.base_tex = tex;
				process_texture_path(p,out_info.base_tex);
			}
		}
		else if(MFS_string_match(&s,"TEXTURE_SCALE",false))
		{
			tex_s = 1.0 / MFS_double(&s);
			tex_t = 1.0 / MFS_double(&s);
		}
		else if(MFS_string_match(&s,"TEXTURE_WIDTH",false))
		{
			tex_x = MFS_double(&s);
			tex_y = tex_x * tex_s / tex_t;
		}
		else if(MFS_string_match(&s,"OBJECT",false))
		{
			string p;
			MFS_string(&s,&p);
			obj_paths.push_back(p);
		}
		else if(MFS_string_match(&s,"TILE",false))
		{
			out_info.tile.resize(16);
			double s1 = MFS_double(&s);
			double t1 = MFS_double(&s);
			double s2 = MFS_double(&s);
			double t2 = MFS_double(&s);
			double x1 = s1 * tex_s * tex_x;
			double x2 = s2 * tex_s * tex_x;
			double y1 = t1 * tex_t * tex_y;
			double y2 = t2 * tex_t * tex_y;
			
			s1 *= tex_s;
			s2 *= tex_s;
			t1 *= tex_t;
			t2 *= tex_t;
			
			anchor_x = (x1 + x2) * 0.5;
			anchor_y = (y1 + y2) * 0.5;
			out_info.tile[ 0] = x1;
			out_info.tile[ 1] = y1;
			out_info.tile[ 2] = s1;
			out_info.tile[ 3] = t1;
			out_info.tile[ 4] = x2;
			out_info.tile[ 5] = y1;
			out_info.tile[ 6] = s2;
			out_info.tile[ 7] = t1;
			out_info.tile[ 8] = x2;
			out_info.tile[ 9] = y2;
			out_info.tile[10] = s2;
			out_info.tile[11] = t2;
			out_info.tile[12] = x1;
			out_info.tile[13] = y2;
			out_info.tile[14] = s1;
			out_info.tile[15] = t2;
		}
		else if(MFS_string_match(&s,"ROTATION",false))
		{
			rotation = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"CROP_POLY",false))
		{
			out_info.tile.clear();
			while(MFS_has_word(&s))
			{
				double ps = MFS_double(&s);
				double pt = MFS_double(&s);
				out_info.tile.push_back(ps * tex_s * tex_x);
				out_info.tile.push_back(pt * tex_t * tex_y);
				out_info.tile.push_back(ps * tex_s);
				out_info.tile.push_back(pt * tex_t);
			}
		}
		else if(MFS_string_match(&s,"OBJ_DRAPED",false) ||
				MFS_string_match(&s,"OBJ_GRADED",false))
		{
			out_info.objs.push_back(agp_t::obj());
			out_info.objs.back().x = MFS_double(&s) * tex_s * tex_x;
			out_info.objs.back().y = MFS_double(&s) * tex_t * tex_y;
			out_info.objs.back().r = MFS_double(&s);
			out_info.objs.back().name = obj_paths[MFS_int(&s)];
			out_info.objs.back().show_lo = MFS_int(&s);
			out_info.objs.back().show_hi = MFS_int(&s);
		}
		else if(MFS_string_match(&s,"ANCHOR_PT",false))
		{
			anchor_x = MFS_double(&s) * tex_s * tex_x;
			anchor_y = MFS_double(&s) * tex_t * tex_y;
		}
		else if (MFS_string_match(&s,"HIDE_TILES",true))
		{
			out_info.hide_tiles = 1;
		}
		else if (MFS_string_match(&s,"MESH_SHADER",true))
		{
			is_mesh_shader = true;
		}
		MFS_string_eol(&s,NULL);
	}
	
	for(int n = 0; n < out_info.tile.size(); n += 4)
	{
		out_info.tile[n  ] -= anchor_x;
		out_info.tile[n+1] -= anchor_y;
		do_rotate(rotation,out_info.tile[n  ],out_info.tile[n+1]);
	}
	for(vector<agp_t::obj>::iterator o = out_info.objs.begin(); o != out_info.objs.end(); ++o)
	{
		o->x -= anchor_x;
		o->y -= anchor_y;
		do_rotate(rotation,o->x,o->y);
		o->r += 90.0 * rotation;
	}
	
	MemFile_Close(agp);

	mAGP[path] = out_info;
	return true;
}
	
	
#endif
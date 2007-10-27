#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <vector>

using std::vector;

#define LOD_NEAR 0
#define LOD_FAR 20000

inline float extrap(float x1, float y1, float x2, float y2, float x)
{
	if (x1 == x2)	return (y1 + y2) * 0.5f;
	return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

struct	tex_info {
	const char *	tname;
	const char *	lname;
	int				layer;
	float			scale_v;	// meters for whole tex vert
	int				pix_h;		// pixels per meter (PPM)
	float			scale_h;	// size of tex horizonal in pixels
};

int		bridge_offset = 0;
int		cur_tex=0;
float	cur_scale = 0;
void do_tex(tex_info * t)
{
	assert(cur_tex == t->layer);
	printf("TEXTURE 1 %s\nTEXTURE_LIT %s\n",t->tname,t->lname);
	++cur_tex;
	++bridge_offset;
}

void do_tex_bridge(tex_info * t)
{
	assert(cur_tex == (t->layer + bridge_offset));
	printf("TEXTURE_BRIDGE 0 %s\nTEXTURE_LIT %s\n",t->tname,t->lname);
	++cur_tex;
}

inline float pixel_to_meter(tex_info * t, float s)
{
	return s / t->pix_h;
}

class	road_elem {
public:
	virtual float	get_width()=0;
	virtual void	emit(FILE * fi, float r1, float r2, float w, tex_info * t)=0;
};

class	road_container {
public:
	virtual void	accept(road_elem * e)=0;
};

class	road_deck : public road_elem {
	int				s1;
	int				s2;
	tex_info *		tex;
	const char *	surf;
public:
	road_deck(int is1, int is2, tex_info * itex, const char * isurf) :
		s1(is1),s2(is2), tex(itex), surf(isurf) { }
	virtual float	get_width() { return pixel_to_meter(tex,fabs(s2-s1)); }
	virtual void	emit(FILE * fi, float r1, float r2, float w, tex_info * t)
	{
		assert(t==tex);
		fprintf(fi,"SEGMENT_HARD % 5d % 5d   %f  0.00 % 4d    %f  0.00 % 4d    %s\n",
			LOD_NEAR,LOD_FAR, r1, s1, r2, s2, surf);
	}
};

class	road_underside : public road_elem, public road_container {
	road_elem *		parent;
	int				s1;
	int				s2;
	float 			y;
	tex_info *		tex;
public:
	virtual void accept(road_elem * e) { parent = e; }

	road_underside(int is1, int is2, float iy, tex_info * itex) :
		s1(is1),s2(is2), y(iy), tex(itex) { }
	virtual float	get_width() { return parent->get_width(); }
	virtual void	emit(FILE * fi, float r1, float r2, float w, tex_info * t)
	{
		assert(t==tex);
		parent->emit(fi,r1,r2,w,t);
		fprintf(fi,"SEGMENT      % 5d % 5d   %f %+.2f % 4d    %f %+.2f % 4d\n",
			LOD_NEAR,LOD_FAR, r1, y, s1, r2, y, s2);
	}
};

class	road_pylons : public road_elem, public road_container {
	road_elem * e;
	const char * obj;
	float		srat;
	float		soff;
	float		spacing;
	float		start_space;
	int 		on_ground;
public:
	virtual void accept(road_elem * ie) { e = ie; }

	road_pylons(const char * iobj, float israt, float isoff, float ispacing, float istart_space, int ion ) : obj(iobj), srat(israt),
		soff(isoff),spacing(ispacing),start_space(istart_space), on_ground(ion) { }
	virtual float get_width() { return e->get_width(); }
	virtual void	emit(FILE * fi, float r1, float r2, float w, tex_info * t)
	{
		e->emit(fi,r1,r2,w,t);
		fprintf(fi,"OBJECT   % 20s %+.2f   0.0   %d    % 4d % 4d\n",
			obj, extrap(0,r1,1,r2,srat) + soff / w, on_ground, (int) spacing, (int) start_space);
	}
};

class	road_blade : public road_elem {
	int				s1;
	int				s2;
	float			y1;
	float			y2;
	tex_info *		tex;
public:
	road_blade(int is1, int is2, float iy1, float iy2, tex_info * itex) :
		s1(is1),s2(is2), y1(iy1), y2(iy2), tex(itex) { }
	virtual float	get_width() { return 0; }
	virtual void	emit(FILE * fi, float r1, float r2, float w,tex_info * t)
	{
		assert(t==tex);
		fprintf(fi,"SEGMENT      % 5d % 5d   %f %+.2f % 4d    %f %+.2f % 4d\n",
			LOD_NEAR,LOD_FAR, r1, y1, s1, r2, y2, s2);
	}
};


class	road_spacer : public road_elem {
	float w;
public:
	road_spacer(float iw) : w(iw) { }
	virtual float	get_width() { return w; }
	virtual void	emit(FILE * fi, float r1, float r2, float w,tex_info * t) { }
};



class	road_composite : public road_elem, public road_container {
	vector<road_elem *>	parts;
public:
	void add(road_elem * e) { parts.push_back(e); }
	virtual void accept(road_elem * e) { add(e); }
	virtual float	get_width() 
	{
		float t = 0;
		for(vector<road_elem *>::iterator i = parts.begin(); i != parts.end(); ++i)
			t += (*i)->get_width();
		return t;		
	}
	virtual void	emit(FILE * fi, float r1, float r2,  float w, tex_info * tex)
	{
		float t = this->get_width();		
		float p = 0;
		
		for(vector<road_elem *>::iterator i = parts.begin(); i != parts.end(); ++i)
		{
			float sr1 = extrap(0,r1,t,r2,p);
			p += (*i)->get_width();
			float sr2 = extrap(0,r1,t,r2,p);
			(*i)->emit(fi,sr1,sr2,w,tex);
		}
	}
};	

class	road : public road_container {
	road_elem * 	e;
	tex_info * 		tex;
	const char * 	name;
	int 			num;
	int				bridge;
public:
	road(int br,tex_info * itex, const char * iname, int inum) :
		bridge(br),tex(itex), name(iname), num(inum) { }
	virtual void accept(road_elem * ie) { e = ie; }
	
	void emit()
	{
		if(cur_scale != tex->scale_h)
		{
			printf("SCALE %d\n\n", (int) tex->scale_h);
			cur_scale = tex->scale_h;
		}
		float w = e->get_width();
		printf("# %s\n", name);
		printf("ROAD_TYPE %d     %f %f       %d    1.0 1.0 1.0\n",num, w, tex->scale_v, tex->layer + (bridge ? bridge_offset : 0));
		e->emit(stdout,0.0, 1.0, w, tex);
		printf("\n");	
	}
};



tex_info	railroad = { "railroad.dds" , "//"				 , 0, 30, 8.5, 512  };
tex_info	local 	 = { "local.dds"    , "//"				 , 1, 15, 8.0, 512  };
tex_info	secondary= { "secondary.dds", "secondary_LIT.dds", 2, 30, 8.1, 1024 };
tex_info	highway  = { "highway.dds"  , "highway_LIT.dds"	 , 3, 30, 7.6, 1024 };


vector<road_container *>	road_stack;

road_container * get_top(void) { assert(!road_stack.empty()); return road_stack.back(); }

void	make_blade(int s1, int s2, float y1, float y2, tex_info * t) { get_top()->accept(new road_blade	(s1,s2,y1,y2,t)); }
void	make_deck(int s1, int s2, tex_info * t, const char * s) { get_top()->accept(new road_deck	(s1,s2,t,s)); }
void 	make_spacer(float f)	{	get_top()->accept(new road_spacer	(f)); }
void	underside_start(int is1, int is2, float iy, tex_info * itex) 
		{ 	road_underside * u = new road_underside(is1,is2,iy,itex); get_top()->accept(u); road_stack.push_back(u); }
void 	underside_end()	{	road_stack.pop_back(); }
void	pylons_start(const char * iobj, float israt, float isoff, float ispacing, float istart_space, int on_ground) 
		{	road_pylons * p = new road_pylons(iobj,israt,isoff,ispacing,istart_space, on_ground); get_top()->accept(p); road_stack.push_back(p); }				
void 	pylons_end()	{	road_stack.pop_back(); }


void	road_start(int br, int n, const char * na, tex_info * t) {  road * r = new road(br, t,na, n); road_stack.push_back(r); }
void 	road_end(void) { road * r = dynamic_cast<road *>(road_stack.back()); r->emit(); road_stack.pop_back(); assert(road_stack.empty()); }

void 	start_composite()	{	road_composite * c = new road_composite; road_stack.push_back(c); }
void 	end_composite()	{	road_composite * c = dynamic_cast<road_composite*>(road_stack.back()); road_stack.pop_back(); get_top()->accept(c); }

void hwy_underside_start()	{ underside_start(357,409,-1.5,&highway); }

void hwy_start(int b, int n, const char * na) { road_start(b, n,na,&highway); }
void hwy_pylon1_start() { pylons_start("highway_pylon.obj", 0.5, 0, 120, 60, 0); }
void hwy_pylon2_start() { 	pylons_start("highway_pylon.obj", 0.20, 0, 120, 60, 0); 	
							pylons_start("highway_pylon.obj", 0.80, 0, 120, 60, 0); }
void hwy_susp_start()   { pylons_start("highway_susp_twr.obj", 0.5, 0, 0, 0, 1);	} 
void hwy_arch1_start() { pylons_start("secondary_oldpylon.obj", 0.5, 0, 30, 0, 0); }
void hwy_arch2_start() { 	pylons_start("secondary_oldpylon.obj", 0.0, 6, 30, 0, 0); 	
							pylons_start("secondary_oldpylon.obj", 1.0, -6, 30, 0, 0); }

void hwy_guard_rail()	{	make_blade(177,189,0.5,0,&highway); }
void hwy_overpass_side(){	make_blade(330,357,0.5,-1.5,&highway); }
void hwy_suspension_side(){	make_blade(877,933,1.5,-1.5,&highway); }
void hwy_arch_side() 	{ 	make_blade(430,580,-8,1.2,&highway);  }
void hwy_median()		{	make_deck(199,223, &highway, "gravel"); }
void hwy_3lane_L()		{	make_deck(0, 103, &highway, "asphalt"); }
void hwy_3lane_R()		{	make_deck(103, 0, &highway, "asphalt"); }
void hwy_2lane_L()		{	make_deck(106, 177, &highway, "asphalt"); }
void hwy_2lane_R()		{	make_deck(177, 106, &highway, "asphalt"); }
void hwy_trains()		{	make_deck(235, 313, &highway, "gravel"); }


void hwy_6unsep(void) { hwy_3lane_L(); hwy_median(); hwy_3lane_R(); }
void hwy_6unseptr(void) { hwy_3lane_L(); hwy_trains(); hwy_3lane_R(); }
void hwy_6oneway(void) { hwy_3lane_L(); }
void hwy_6onewaytr(void) { hwy_3lane_L(); hwy_trains(); }
void hwy_4unsep(void) { hwy_2lane_L(); hwy_median(); hwy_2lane_R(); }
void hwy_4unseptr(void) { hwy_2lane_L(); hwy_trains(); hwy_2lane_R(); }
void hwy_4oneway(void) { hwy_2lane_L(); }
void hwy_4onewaytr(void) { hwy_2lane_L(); hwy_trains(); }


void hwy_reg_s(void) { start_composite(); hwy_guard_rail(); }
void hwy_reg_e(void) { hwy_guard_rail(); end_composite(); }

void hwy_ovr_s(int p) { if (p) hwy_pylon2_start(); else hwy_pylon1_start(); hwy_underside_start(); start_composite(); hwy_overpass_side(); }
void hwy_ovr_e(int p) { hwy_overpass_side(); end_composite(); underside_end(); if (p) pylons_end(); pylons_end(); }
void hwy_sus_s(int p) { hwy_susp_start(); hwy_underside_start(); start_composite(); hwy_suspension_side(); }
void hwy_sus_e(int p) { hwy_suspension_side(); end_composite(); underside_end(); pylons_end(); }
void hwy_arc_s(int p) { if (p) hwy_arch2_start(); else hwy_arch1_start(); hwy_underside_start(); start_composite(); hwy_arch_side(); }
void hwy_arc_e(int p) { hwy_arch_side(); end_composite(); underside_end(); if (p) pylons_end(); pylons_end(); }

/****/

void sec_underside_start(float y)	{ underside_start(481,528,y,&secondary); }

void sec_start(int b, int n, const char * na) { road_start(b, n,na,&secondary); }
void sec_pylon1_start() { pylons_start("highway_pylon.obj", 0.5, 0, 120, 60, 0); }
void sec_pylon2_start() { 	pylons_start("highway_pylon.obj", 0.20, 0, 120, 60, 0); 	
							pylons_start("highway_pylon.obj", 0.80, 0, 120, 60, 0); }
void sec_stone_start() { 	pylons_start("secondary_oldpylon.obj", 0.5, 0, 30, 0, 0); }
void sec_girdr_start() { 	pylons_start("secondary_pylon.obj", 0.0, -1.3, 60, 0, 0); 	
							pylons_start("secondary_pylon.obj", 1.0,  1.3, 60, 0, 0); }
void sec_girdr_space() { 	pylons_start("secondary_pylon.obj", 0.5, 0, 60, 0, 0); 	
							make_spacer(2.6);
							pylons_end(); }
void sec_space()		{	make_spacer(2.6); }

void sec_overpass_side(){	make_blade(556,545,0.5,-1.5,&secondary); }
void sec_stone_side()	{	make_blade(929,1021,-10.5,1.5,&secondary); }
void sec_girdr_side() 	{ 	make_blade(681,810,-2,9,&secondary);  }

void sec_prim_undiv()	{	make_deck(23,131, &secondary, "asphalt"); }
void sec_prim_undiv_s()	{	make_deck( 2,151, &secondary, "asphalt"); }
void sec_prim_L()		{	make_deck(175,229, &secondary, "asphalt"); }
void sec_prim_L_s()		{	make_deck(154,250, &secondary, "asphalt"); }
void sec_prim_tr()		{	make_deck(569,640, &secondary, "gravel"); }
void sec_prim_R()		{	make_deck(175,229, &secondary, "asphalt"); }
void sec_prim_R_s()		{	make_deck(154,250, &secondary, "asphalt"); }

void sec_sec()			{	make_deck(358,416, &secondary, "asphalt"); }
void sec_sec_s()		{	make_deck(253,356, &secondary, "asphalt"); }
void sec_sec_p()		{	make_deck(419,477, &secondary, "asphalt"); }

void sec_reg_s(void) { start_composite(); }
void sec_reg_e(void) { end_composite(); }
void sec_ovr_s(int p) { if (p) sec_pylon2_start(); else sec_pylon1_start(); sec_underside_start(-1.0); start_composite(); sec_overpass_side(); }
void sec_ovr_e(int p) { sec_overpass_side(); end_composite(); underside_end(); if (p) pylons_end(); pylons_end(); }
void sec_grd_s(int p) { if(p) { sec_girdr_start(); start_composite(); } sec_underside_start(-0.5); start_composite(); sec_girdr_side(); }
void sec_grd_e(int p) { sec_girdr_side(); end_composite(); underside_end(); if (p) {end_composite(); pylons_end(); pylons_end(); } }
void sec_grd_b(int p) { sec_grd_e(0); sec_girdr_space(); sec_grd_s(0); }
void sec_stn_s(int p) { sec_stone_start(); sec_underside_start(-1.5); start_composite(); sec_stone_side(); }
void sec_stn_e(int p) { sec_stone_side(); end_composite(); underside_end(); pylons_end(); }







int main(int, char *[])
{
	printf("A\n");
	printf("800\n");
	printf("ROADS\n\n");
	
	do_tex(&railroad);
	do_tex(&local);
	do_tex(&secondary);
	do_tex(&highway);
	do_tex_bridge(&railroad);
	do_tex_bridge(&local);
	do_tex_bridge(&secondary);
	do_tex_bridge(&highway);
	
	// six-lane US highways: 1-13 = normal,bridge then 76-88=suspension bridge, metal bridge	
	
	hwy_start(0, 1, "net_SixLaneUSHighway");
		hwy_reg_s();
		hwy_6unsep();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 2, "net_SixLaneUSHighwayOverpass");
		hwy_ovr_s(1);
		hwy_6unsep();
		hwy_ovr_e(1);
	road_end();

	hwy_start(1, 76, "net_SixLaneUSHighwayOverpass");
		hwy_sus_s(1);
		hwy_6unsep();
		hwy_sus_e(1);
	road_end();

	hwy_start(1, 77, "net_SixLaneUSHighwayOverpass");
		hwy_arc_s(1);
		hwy_6unsep();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 3, "net_SixLaneUSHighwaySeparated");
		hwy_reg_s();
		hwy_3lane_L(); make_spacer(3.0); hwy_3lane_R();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 4, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_ovr_s(0);
		hwy_3lane_L(); 
		hwy_ovr_e(0);
		make_spacer(3.0); 
		hwy_ovr_s(0);
		hwy_3lane_R(); 
		hwy_ovr_e(0);
		end_composite();
	road_end();

	hwy_start(1, 78, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_sus_s(0);
		hwy_3lane_L(); 
		hwy_sus_e(0);
		make_spacer(3.0); 
		hwy_sus_s(0);
		hwy_3lane_R(); 
		hwy_sus_e(0);
		end_composite();
	road_end();

	hwy_start(1, 79, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_arc_s(0);
		hwy_3lane_L(); 
		hwy_arc_e(0);
		make_spacer(3.0); 
		hwy_arc_s(0);
		hwy_3lane_R(); 
		hwy_arc_e(0);
		end_composite();
	road_end();

	hwy_start(0, 5, "net_SixLaneUSHighwayOneway");
		hwy_reg_s();
		hwy_6oneway();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,6, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_ovr_s(0);
		hwy_6oneway();
		hwy_ovr_e(0);
	road_end();

	hwy_start(1,80, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_sus_s(0);
		hwy_6oneway();
		hwy_sus_e(0);
	road_end();

	hwy_start(1,81, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_arc_s(0);
		hwy_6oneway();
		hwy_arc_e(0);
	road_end();
	
	hwy_start(0, 7, "net_SixLaneUSHighway");
		hwy_reg_s();
		hwy_6unseptr();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 8, "net_SixLaneUSHighwayOverpass");
		hwy_ovr_s(1);
		hwy_6unseptr();
		hwy_ovr_e(1);
	road_end();

	hwy_start(1, 82, "net_SixLaneUSHighwayOverpass");
		hwy_sus_s(1);
		hwy_6unseptr();
		hwy_sus_e(1);
	road_end();

	hwy_start(1, 83, "net_SixLaneUSHighwayOverpass");
		hwy_arc_s(1);
		hwy_6unseptr();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 9, "net_SixLaneUSHighwaySeparated");
		hwy_reg_s();
		hwy_3lane_L(); 	hwy_trains(); make_spacer(3.0); hwy_3lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 10, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_ovr_s(0);
		hwy_3lane_L(); 
		hwy_trains();
		hwy_ovr_e(0);
		make_spacer(3.0); 
		hwy_ovr_s(0);
		hwy_3lane_R(); 
		hwy_ovr_e(0);
		end_composite();
	road_end();

	hwy_start(1, 84, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_sus_s(0);
		hwy_3lane_L(); 
		hwy_trains();
		hwy_sus_e(0);
		make_spacer(3.0); 
		hwy_sus_s(0);
		hwy_3lane_R(); 
		hwy_sus_e(0);
		end_composite();
	road_end();

	hwy_start(1, 85, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_arc_s(0);
		hwy_3lane_L(); 
		hwy_trains();
		hwy_arc_e(0);
		make_spacer(3.0); 
		hwy_arc_s(0);
		hwy_3lane_R(); 
		hwy_arc_e(0);
		end_composite();
	road_end();

	hwy_start(0, 11, "net_SixLaneUSHighwayOneway");
		hwy_reg_s();
		hwy_6onewaytr();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,12, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_ovr_s(0);
		hwy_6onewaytr();
		hwy_ovr_e(0);
	road_end();

	hwy_start(1,86, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_sus_s(0);
		hwy_6onewaytr();
		hwy_sus_e(0);
	road_end();

	hwy_start(1,87, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_arc_s(1);
		hwy_6onewaytr();
		hwy_arc_e(1);
	road_end();

	// four-lane US highways: 13-26 = normal,bridge then 88-100=suspension bridge, metal bridge	

	hwy_start(0, 13, "net_SixLaneUSHighway");
		hwy_reg_s();
		hwy_4unsep();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 14, "net_SixLaneUSHighwayOverpass");
		hwy_ovr_s(1);
		hwy_4unsep();
		hwy_ovr_e(1);
	road_end();

	hwy_start(1, 88, "net_SixLaneUSHighwayOverpass");
		hwy_sus_s(1);
		hwy_4unsep();
		hwy_sus_e(1);
	road_end();

	hwy_start(1, 89, "net_SixLaneUSHighwayOverpass");
		hwy_arc_s(1);
		hwy_4unsep();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 15, "net_SixLaneUSHighwaySeparated");
		hwy_reg_s();
		hwy_2lane_L(); make_spacer(3.0); hwy_2lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 16, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_ovr_s(0);
		hwy_2lane_L(); 
		hwy_ovr_e(0);
		make_spacer(3.0); 
		hwy_ovr_s(0);
		hwy_2lane_R(); 
		hwy_ovr_e(0);
		end_composite();
	road_end();

	hwy_start(1, 90, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_sus_s(0);
		hwy_2lane_L(); 
		hwy_sus_e(0);
		make_spacer(3.0); 
		hwy_sus_s(0);
		hwy_2lane_R(); 
		hwy_sus_e(0);
		end_composite();
	road_end();

	hwy_start(1, 91, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_arc_s(0);
		hwy_2lane_L(); 
		hwy_arc_e(0);
		make_spacer(3.0); 
		hwy_arc_s(0);
		hwy_2lane_R(); 
		hwy_arc_e(0);
		end_composite();
	road_end();
	
	hwy_start(0, 17, "net_SixLaneUSHighwaySeparated");
		start_composite();
		hwy_2lane_L(); make_spacer(3.0); hwy_2lane_R();
		end_composite();
	road_end();
	
	hwy_start(0, 18, "net_SixLaneUSHighwayOneway");
		hwy_reg_s();
		hwy_4oneway();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,19, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_ovr_s(0);
		hwy_4oneway();
		hwy_ovr_e(0);
	road_end();

	hwy_start(1,92, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_sus_s(0);
		hwy_4oneway();
		hwy_sus_e(0);
	road_end();

	hwy_start(1,93, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_arc_s(1);
		hwy_4oneway();
		hwy_arc_e(1);
	road_end();
	
	hwy_start(0, 20, "net_SixLaneUSHighway");
		hwy_reg_s();
		hwy_4unseptr();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 21, "net_SixLaneUSHighwayOverpass");
		hwy_ovr_s(1);
		hwy_4unseptr();
		hwy_ovr_e(1);
	road_end();

	hwy_start(1, 94, "net_SixLaneUSHighwayOverpass");
		hwy_sus_s(1);
		hwy_4unseptr();
		hwy_sus_e(1);
	road_end();

	hwy_start(1, 95, "net_SixLaneUSHighwayOverpass");
		hwy_arc_s(1);
		hwy_4unseptr();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 22, "net_SixLaneUSHighwaySeparated");
		hwy_reg_s();
		hwy_2lane_L(); 	hwy_trains(); make_spacer(3.0); hwy_2lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 23, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_ovr_s(0);
		hwy_2lane_L(); 
		hwy_trains();
		hwy_ovr_e(0);
		make_spacer(3.0); 
		hwy_ovr_s(0);
		hwy_2lane_R(); 
		hwy_ovr_e(0);
		end_composite();
	road_end();

	hwy_start(1, 96, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_sus_s(0);
		hwy_2lane_L(); 
		hwy_trains();
		hwy_sus_e(0);
		make_spacer(3.0); 
		hwy_sus_s(0);
		hwy_2lane_R(); 
		hwy_sus_e(0);
		end_composite();
	road_end();

	hwy_start(1, 97, "net_SixLaneUSHighwaySeparatedOverpass");
		start_composite();
		hwy_arc_s(0);
		hwy_2lane_L(); 
		hwy_trains();
		hwy_arc_e(0);
		make_spacer(3.0); 
		hwy_arc_s(0);
		hwy_2lane_R(); 
		hwy_arc_e(0);
		end_composite();
	road_end();

	hwy_start(0, 24, "net_SixLaneUSHighwayOneway");
		hwy_reg_s();
		hwy_4onewaytr();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,25, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_ovr_s(0);
		hwy_4onewaytr();
		hwy_ovr_e(0);
	road_end();

	hwy_start(1,98, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_sus_s(0);
		hwy_4onewaytr();
		hwy_sus_e(0);
	road_end();

	hwy_start(1,99, "net_SixLaneUSHighwayOnewayOVerpass");
		hwy_arc_s(1);
		hwy_4onewaytr();
		hwy_arc_e(1);
	road_end();
		
	// PRIMARY roads: 26-42 = primary, primary overpass, 63-71 = primary girder

	sec_start(0, 26, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_undiv();
		sec_reg_e();
	road_end();
	
	sec_start(1,27, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(1);
		sec_prim_undiv();
		sec_ovr_e(1);
	road_end();

	sec_start(1,63, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_undiv();
		sec_grd_e(1);
	road_end();

	sec_start(0, 28, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_undiv_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,29, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(1);
		sec_prim_undiv_s();
		sec_ovr_e(1);
	road_end();

	sec_start(1,64, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_undiv_s();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 30, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,31, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_ovr_e(1);
	road_end();

	sec_start(1,65, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 32, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,33, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_ovr_e(1);
	road_end();

	sec_start(1,66, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 34, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L();
		sec_space();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,35, "net_SixLaneUSHighwayOnewayOVerpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_L();
		sec_ovr_e(0);
		sec_space();		
		sec_ovr_s(0);
		sec_prim_R();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(1,67, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L();
		sec_grd_b(1);
		sec_prim_R();
		sec_grd_e(1);
	road_end();

	sec_start(0, 36, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L_s();
		sec_space();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,37, "net_SixLaneUSHighwayOnewayOVerpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_L_s();
		sec_ovr_e(0);
		sec_space();		
		sec_ovr_s(0);
		sec_prim_R_s();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(1,68, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_grd_b(1);
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 38, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L();
		sec_prim_tr();
		sec_space();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,39, "net_SixLaneUSHighwayOnewayOVerpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_L();
		sec_prim_tr();
		sec_ovr_e(0);
		sec_space();		
		sec_ovr_s(0);
		sec_prim_R();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(1,69, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_grd_b(1);
		sec_prim_R();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 40, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_prim_L_s();
		sec_prim_tr();
		sec_space();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,41, "net_SixLaneUSHighwayOnewayOVerpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_L_s();
		sec_prim_tr();
		sec_ovr_e(0);
		sec_space();		
		sec_ovr_s(0);
		sec_prim_R_s();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(1,70, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_grd_b(1);
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
	
	// SECONDAARY roads: 42-47 = secondary, secondary overpass, 71-73 = stone bridge	

	sec_start(0, 42, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_sec_s();
		sec_reg_e();
	road_end();

	sec_start(1,43, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(0);
		sec_sec_s();
		sec_ovr_e(0);
	road_end();

	sec_start(1,71, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_stn_s(1);
		sec_sec_s();
		sec_stn_e(1);
	road_end();

	sec_start(0, 44, "net_SixLaneUSHighwayOneway");
		sec_reg_s();
		sec_sec();
		sec_reg_e();
	road_end();

	sec_start(1,45, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_ovr_s(0);
		sec_sec();
		sec_ovr_e(0);
	road_end();

	sec_start(1,46, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_reg_s();
		sec_sec_p();
		sec_reg_e();
	road_end();

	sec_start(1,72, "net_SixLaneUSHighwayOnewayOVerpass");
		sec_stn_s(1);
		sec_sec();
		sec_stn_e(1);
	road_end();

	
		
	
	return 0;
}



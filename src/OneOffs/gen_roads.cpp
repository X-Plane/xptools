#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <vector>

using std::vector;

#define LOD_NEAR 0
#define LOD_FAR 20000

static int gBackward = 0;

inline float extrap(float x1, float y1, float x2, float y2, float x)
{
	if (x1 == x2)	return (y1 + y2) * 0.5f;
	return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

struct	tex_info {
	const char *	tname;
	const char *	lname;
	int				layer;
	int				show_level;
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
	virtual void	emit(FILE * fi, float r1, float r2, float w, float d, tex_info * t)=0;
};

class	road_container {
public:
	virtual void	accept(road_elem * e)=0;
};

struct	car_lane {
	int		reversed;
	int		pixel;
	int		speed;
	float	density;
	int		traffic_type;
};

class	road_deck : public road_elem {
	int				s1;
	int				s2;
	tex_info *		tex;
	const char *	surf;
	vector<car_lane>	cars;
public:
	road_deck(int is1, int is2, tex_info * itex, const char * isurf) :
		s1(is1),s2(is2), tex(itex), surf(isurf) { }
	void add_car(int r, int p, int s, float d, int t) {
		cars.push_back(car_lane());
		cars.back().reversed = r;
		cars.back().pixel = p;
		cars.back().speed = s;
		cars.back().density = d;
		cars.back().traffic_type = t;
	}
	virtual float	get_width() { return pixel_to_meter(tex,fabs(s2-s1)); }
	virtual void	emit(FILE * fi, float r1, float r2, float w, float d, tex_info * t)
	{
		assert(t==tex);
		fprintf(fi,"SEGMENT_HARD % 5d % 5d   %f  0.00 % 4d    %f  0.00 % 4d    %s\n",
			LOD_NEAR,LOD_FAR, r1, s1, r2, s2, surf);
		
		for(vector<car_lane>::iterator i = cars.begin(); i != cars.end(); ++i)
		{
			float cp = extrap(s1,r1,s2,r2,i->pixel);
			if(gBackward)
			{
				if(!i->reversed) cp = 1.0 - cp;
			}
			else
			{
				if(i->reversed) cp = 1.0 - cp;
			}
			fprintf(fi,"CAR % 2d %.2f % 2d %.2f % 2d\n", gBackward ? (1 - i->reversed) : i->reversed, cp, i->speed, i->density * d, i->traffic_type);
		}
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
	virtual void	emit(FILE * fi, float r1, float r2, float w, float d, tex_info * t)
	{
		assert(t==tex);
		parent->emit(fi,r1,r2,w,d,t);
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
	virtual void	emit(FILE * fi, float r1, float r2, float w, float d, tex_info * t)
	{
		e->emit(fi,r1,r2,w,d, t);
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
	virtual void	emit(FILE * fi, float r1, float r2, float w,float d,tex_info * t)
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
	virtual void	emit(FILE * fi, float r1, float r2, float w,float d,tex_info * t) { }
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
	virtual void	emit(FILE * fi, float r1, float r2,  float w, float d, tex_info * tex)
	{
		float t = this->get_width();		
		float p = 0;
		
		for(vector<road_elem *>::iterator i = parts.begin(); i != parts.end(); ++i)
		{
			float sr1 = extrap(0,r1,t,r2,p);
			p += (*i)->get_width();
			float sr2 = extrap(0,r1,t,r2,p);
			(*i)->emit(fi,sr1,sr2,w,d,tex);
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
		if(bridge==2)
		{
			printf("REQUIRE_EVEN\n");
			printf("SHOW_LEVEL 1\n");
		} else
			printf("SHOW_LEVEL %d\n", tex->show_level);
		e->emit(stdout,0.0, 1.0, w, bridge==2 ? 0.0 : 1.0, tex);
		printf("\n");	
	}
};



tex_info	railroad = { "railroad.dds" , "//"				 , 0, 1, 30, 8.5, 512  };
tex_info	local 	 = { "local.dds"    , "//"				 , 1, 3, 15, 8.5, 512  };
tex_info	secondary= { "secondary.dds", "secondary_LIT.dds", 2, 2, 30, 8.5, 1024 };
tex_info	highway  = { "highway.dds"  , "highway_LIT.dds"	 , 3, 1, 30, 8.5, 1024 };


vector<road_container *>	road_stack;

road_container * get_top(void) { assert(!road_stack.empty()); return road_stack.back(); }

road_deck * last = NULL;

void	make_blade(int s1, int s2, float y1, float y2, tex_info * t) { get_top()->accept(new road_blade	(s1,s2,y1,y2,t)); }
void	make_deck(int s1, int s2, tex_info * t, const char * s) { get_top()->accept(last = new road_deck	(s1,s2,t,s)); }
void	make_car(int r, int p, int s, float d, int t) { last->add_car(r,p,s,d,t); }
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
void hwy_3lane_L()		{	make_deck(0, 103, &highway, "asphalt"); 
							make_car(1, 24, 25, 0.06, 1);
							make_car(1, 50, 30, 0.06, 1);
							make_car(1, 77, 35, 0.06, 0);		}																	
void hwy_3lane_R()		{	make_deck(103, 0, &highway, "asphalt"); 
							make_car(0, 24, 25, 0.06, 1);
							make_car(0, 50, 30, 0.06, 1);
							make_car(0, 77, 35, 0.06, 0);		}
void hwy_2lane_L()		{	make_deck(106, 177, &highway, "asphalt");
							make_car(1, 128, 25, 0.06, 1);
							make_car(1, 154, 35, 0.06, 0);		}
void hwy_2lane_R()		{	make_deck(177, 106, &highway, "asphalt"); 
							make_car(0, 128, 25, 0.06, 1);
							make_car(0, 154, 35, 0.06, 0);		}
void hwy_trains()		{	make_deck(235, 313, &highway, "gravel"); 
							make_car(1, 259, 30, 0.01, 3);
							make_car(0, 290, 30, 0.01, 3);		}


void hwy_6unsep(void) { hwy_3lane_L(); hwy_median(); hwy_3lane_R(); }
void hwy_6unseptr(void) { hwy_3lane_L(); hwy_trains(); hwy_3lane_R(); }
void hwy_6oneway(void) { hwy_3lane_R(); }
void hwy_6onewaytr(void) { hwy_3lane_L(); hwy_trains(); }
void hwy_4unsep(void) { hwy_2lane_L(); hwy_median(); hwy_2lane_R(); }
void hwy_4unseptr(void) { hwy_2lane_L(); hwy_trains(); hwy_2lane_R(); }
void hwy_4oneway(void) { hwy_2lane_R(); }
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

void sec_prim_undiv()	{	make_deck(23,131, &secondary, "asphalt"); 
							make_car(1, 38 , 20, 0.02, 1);
							make_car(1, 63 , 25, 0.02, 1);
							make_car(0, 90 , 25, 0.02, 1);
							make_car(0, 115, 20, 0.02, 1); }
void sec_prim_undiv_s()	{	make_deck( 2,151, &secondary, "asphalt");
							make_car(1, 38 , 20, 0.02, 1);
							make_car(1, 63 , 25, 0.02, 1);
							make_car(0, 90 , 25, 0.02, 1);
							make_car(0, 115, 20, 0.02, 1); }
void sec_prim_L()		{	make_deck(175,229, &secondary, "asphalt"); 
							make_car(1, 190 , 20, 0.02, 1);
							make_car(1, 215 , 25, 0.02, 1);		}
void sec_prim_L_s()		{	make_deck(154,250, &secondary, "asphalt"); 
							make_car(1, 190 , 20, 0.02, 1);
							make_car(1, 215 , 25, 0.02, 1);		}
void sec_prim_tr()		{	make_deck(569,640, &secondary, "gravel"); 
							make_car(1, 588, 30, 0.01, 3);
							make_car(0, 620, 30, 0.01, 3);		}
void sec_prim_R()		{	make_deck(175,229, &secondary, "asphalt"); 
							make_car(0, 190 , 25, 0.02, 1);
							make_car(0, 215 , 20, 0.02, 1);		}
void sec_prim_R_s()		{	make_deck(154,250, &secondary, "asphalt"); 
							make_car(0, 190 , 25, 0.02, 1);
							make_car(0, 215 , 20, 0.02, 1);		}
void sec_sec()			{	make_deck(358,416, &secondary, "asphalt"); 
							make_car(1, 374 , 20, 0.01, 1);
							make_car(0, 401 , 20, 0.01, 1);		}
void sec_sec_s()		{	make_deck(253,356, &secondary, "asphalt"); 
							make_car(1, 291 , 20, 0.01, 1);
							make_car(0, 318 , 20, 0.01, 1);		}
void sec_sec_p()		{	make_deck(419,477, &secondary, "asphalt"); 
							make_car(1, 434 , 20, 0.01, 1);
							make_car(0, 461 , 20, 0.01, 1);		}

void sec_reg_s(void) { start_composite(); }
void sec_reg_e(void) { end_composite(); }
void sec_ovr_s(int p) { if (p) sec_pylon2_start(); else sec_pylon1_start(); sec_underside_start(-1.0); start_composite(); sec_overpass_side(); }
void sec_ovr_e(int p) { sec_overpass_side(); end_composite(); underside_end(); if (p) pylons_end(); pylons_end(); }
void sec_grd_s(int p) { if(p) { sec_girdr_start(); start_composite(); } sec_underside_start(-0.5); start_composite(); sec_girdr_side(); }
void sec_grd_e(int p) { sec_girdr_side(); end_composite(); underside_end(); if (p) {end_composite(); pylons_end(); pylons_end(); } }
void sec_grd_b(int p) { sec_grd_e(0); sec_girdr_space(); sec_grd_s(0); }
void sec_stn_s(int p) { sec_stone_start(); sec_underside_start(-1.5); start_composite(); sec_stone_side(); }
void sec_stn_e(int p) { sec_stone_side(); end_composite(); underside_end(); pylons_end(); }

/*****/

void lcl_underside_start()	{ underside_start(178,226,-1.5,&local); }
void lcl_underwalk()	{ underside_start(183,221,-0.25,&local); }
void lcl_underarch()	{ underside_start(309,359,-0.1,&local); }

void lcl_start(int b, int n, const char * na) { road_start(b, n,na,&local); }
void lcl_pylon_start() { pylons_start("ramp_pylon.obj", 0.5, 0, 120, 60, 0); }

void lcl_overpass_side(){	make_blade(277,266,0.5,-1.5,&local); }
void lcl_arch_side() 	{ 	make_blade(362,435,-8,1,&local);  }
void lcl_walk_side() 	{ 	make_blade(277,265,0.25,-0.25,&local);  }

void lcl_marked2() { make_deck(0,48, &local, "asphalt"); 
						make_car(1, 15 , 15, 0.005, 1);
						make_car(0, 34 , 15, 0.005, 1);		}					
void lcl_marked1() { make_deck(0,48, &local, "asphalt"); 
						make_car(0, 34 , 15, 0.005, 1);		}
void lcl_unmarked1() { make_deck(5,44, &local, "asphalt"); 
						make_car(0, 34 , 15, 0.005, 1);		}
void lcl_4wd() { make_deck(51,108, &local, "gravel"); }

void lcl_walk     () { make_deck(107,146, &local, "gravel"); }
void lcl_walk_over() { make_deck(120,137, &local, "gravel"); }

/*******/

void tra_underside_start()	{ underside_start(391,432,-1.0,&railroad); }
void tra_underbridge_start()	{ underside_start(391,432,-0.5,&railroad); }
void tra_roof_start()	{ underside_start(265,298,6.5,&railroad); }

void tra_start(int b, int n, const char * na) { road_start(b, n,na,&railroad); }

void tra_pylon_start() { pylons_start("railroad_pylon.obj", 0.5, 0, 30, 0, 0); }


void tra_guard_rail()	{	make_blade(96,123,0,2,&railroad); }
void tra_over_side()	{	make_blade(300,347,-1,0,&railroad); }
void tra_bridge_side()	{	make_blade(300,347,-0.5,6.5,&railroad); }

void tra_one()			{	make_deck(44, 95, &railroad, "gravel"); 
							make_car(0, 69 , 30, 0.01, 2);		}
void tra_two()			{	make_deck(148, 251, &railroad, "gravel"); 
							make_car(1, 173 , 30, 0.01, 2);
							make_car(0, 225 , 30, 0.01, 2);		}
void tra_one_ovr_f()	{	make_deck(349, 391, &railroad, "gravel"); 
							make_car(0, 369 , 30, 0.01, 2);		}
void tra_one_ovr_r()	{	make_deck(349, 391, &railroad, "gravel"); 
							make_car(1, 369 , 30, 0.01, 2);		}


int main(int argc, char * argv[])
{
	if(argc >= 2 && strcmp(argv[1], "-backward") == 0)
		gBackward = 1;
	

	printf("A\n");
	printf("800\n");
	printf("ROADS\n\n");
	
printf("#############################################################################################################################\n");	
printf("#This file is automatically generated from a script program that is part of the scenery tools tree - source can be\n");
printf("#ound in the XPTools tree at src/OneOffs/gen_roads.cpp.  Compile with g++ gen_roads.cpp -o gen_roads and then pipe\n");
printf("#the output to roads.net.  Use the -backward flag to generate left-side roads for the UK and friends.\n");


printf("#############################################################################################################################\n");	
printf("# TEXTURE DEFINITIONS\n");
printf("#############################################################################################################################\n");	
	
	do_tex(&railroad);
	do_tex(&local);
	do_tex(&secondary);
	do_tex(&highway);
	do_tex_bridge(&railroad);
	do_tex_bridge(&local);
	do_tex_bridge(&secondary);
	do_tex_bridge(&highway);

printf("#############################################################################################################################\n");	
printf("# TRAFFIC DEFINITIONS\n");
printf("#############################################################################################################################\n");	

printf("# 0: car-only traffic\n");	
printf("CAR_MODEL	lib/cars/car.obj\n");
printf("# 1: car+truck traffic traffic\n");	
printf("CAR_MODEL	lib/cars/car_or_truck.obj\n");

printf("# 2: freight trains\n");	
printf("TRAIN\n");
printf("TRAIN_VARIANT\n");
printf("TRAIN_CAR		11.46	11.46		lib/trains/F_head.obj\n");
printf("TRAIN_CAR		6.665	6.665		lib/trains/F_13_33.obj\n");
printf("TRAIN_CAR		10.03	10.03		lib/trains/F_20.06.obj\n");
printf("TRAIN_CAR		11.94	11.94		lib/trains/F_23.88.obj\n");
printf("TRAIN_CAR		8.665	8.665		lib/trains/F_17.33.obj\n");
printf("TRAIN_CAR		9.155	9.155		lib/trains/F_18.31.obj\n");
printf("TRAIN_CAR		11.46	11.46		lib/trains/F_tail.obj\n");

printf("# 2: passenger-only trains\n");	
printf("# HS TRAIN\n");
printf("TRAIN\n");
printf("TRAIN_VARIANT\n");
printf("TRAIN_CAR		11.0	9.64	lib/trains/HS_head.obj\n");
printf("TRAIN_CAR		13.88	13.72	lib/trains/HS_body.obj\n");
printf("TRAIN_CAR		13.88	13.72	lib/trains/HS_body.obj\n");
printf("TRAIN_CAR		13.88	13.72	lib/trains/HS_body.obj\n");
printf("TRAIN_CAR		9.84	10.9	lib/trains/HS_tail.obj\n");
printf("TRAIN_VARIANT\n");
printf("TRAIN_CAR		10		6.37	lib/trains/R_head.obj\n");
printf("TRAIN_CAR		6.6		6.2		lib/trains/R_body.obj\n");
printf("TRAIN_CAR		6.84	9.7		lib/trains/R_tail.obj\n");
printf("TRAIN_VARIANT\n");
printf("TRAIN_CAR		15.8	13.924	lib/trains/IC_head.obj\n");
printf("TRAIN_CAR		13.8	13.68	lib/trains/IC_body.obj\n");
printf("TRAIN_CAR		13.8	13.68	lib/trains/IC_powr.obj\n");
printf("TRAIN_CAR		13.8	13.68	lib/trains/IC_body.obj\n");
printf("TRAIN_CAR		14.26	15.47	lib/trains/IC_tail.obj\n");

printf("#############################################################################################################################\n");	
printf("# HIGHWAYS\n");
printf("#############################################################################################################################\n");	
printf("#### Six-lane highways.  1-13 = normal/overpass, 76-88 = suspension/metal arch bridges.\n");
	
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

	hwy_start(2, 76, "net_SixLaneUSHighwaySuspensionBridge");
		hwy_sus_s(1);
		hwy_6unsep();
		hwy_sus_e(1);
	road_end();

	hwy_start(2, 77, "net_SixLaneUSHighwayArchBridge");
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

	hwy_start(2, 78, "net_SixLaneUSHighwaySeparatedSuspensionBridge");
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

	hwy_start(2, 79, "net_SixLaneUSHighwaySeparatedArchBridge");
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
	
	hwy_start(1,6, "net_SixLaneUSHighwayOnewayOverpass");
		hwy_ovr_s(0);
		hwy_6oneway();
		hwy_ovr_e(0);
	road_end();

	hwy_start(2,80, "net_SixLaneUSHighwayOnewaySuspensionBridge");
		hwy_sus_s(0);
		hwy_6oneway();
		hwy_sus_e(0);
	road_end();

	hwy_start(2,81, "net_SixLaneUSHighwayOnewayArchBridge");
		hwy_arc_s(0);
		hwy_6oneway();
		hwy_arc_e(0);
	road_end();
	
	hwy_start(0, 7, "net_SixLaneUSHighwayWithTrain");
		hwy_reg_s();
		hwy_6unseptr();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 8, "net_SixLaneUSHighwayWithTrainOverpass");
		hwy_ovr_s(1);
		hwy_6unseptr();
		hwy_ovr_e(1);
	road_end();

	hwy_start(2, 82, "net_SixLaneUSHighwayWithTrainSuspensionBridge");
		hwy_sus_s(1);
		hwy_6unseptr();
		hwy_sus_e(1);
	road_end();

	hwy_start(2, 83, "net_SixLaneUSHighwayWithTrainArchBridge");
		hwy_arc_s(1);
		hwy_6unseptr();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 9, "net_SixLaneUSHighwaySeparatedWithTrain");
		hwy_reg_s();
		hwy_3lane_L(); 	hwy_trains(); make_spacer(3.0); hwy_3lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 10, "net_SixLaneUSHighwaySeparatedWithTrainOverpass");
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

	hwy_start(2, 84, "net_SixLaneUSHighwaySeparatedWithTrainSuspensionBridge");
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

	hwy_start(2, 85, "net_SixLaneUSHighwaySeparatedWithTrainArchBridge");
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

	hwy_start(0, 11, "net_SixLaneUSHighwayOnewayWithTrain");
		hwy_reg_s();
		hwy_6onewaytr();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,12, "net_SixLaneUSHighwayOnewayWithTrainOverpass");
		hwy_ovr_s(0);
		hwy_6onewaytr();
		hwy_ovr_e(0);
	road_end();

	hwy_start(2,86, "net_SixLaneUSHighwayOnewayWithTrainSuspensionBridge");
		hwy_sus_s(0);
		hwy_6onewaytr();
		hwy_sus_e(0);
	road_end();

	hwy_start(2,87, "net_SixLaneUSHighwayOnewayWithTrainArchBridge");
		hwy_arc_s(1);
		hwy_6onewaytr();
		hwy_arc_e(1);
	road_end();

	printf("#### Four-lane highways.  13-26 = normal,bridge then 88-100=suspension bridge, metal arch bridge.\n");

	hwy_start(0, 13, "net_FourLaneUSHighway");
		hwy_reg_s();
		hwy_4unsep();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 14, "net_FourLaneUSHighwayOverpass");
		hwy_ovr_s(1);
		hwy_4unsep();
		hwy_ovr_e(1);
	road_end();

	hwy_start(2, 88, "net_FourLaneUSHighwaySuspensionBridge");
		hwy_sus_s(1);
		hwy_4unsep();
		hwy_sus_e(1);
	road_end();

	hwy_start(2, 89, "net_FourLaneUSHighwayArchBridge");
		hwy_arc_s(1);
		hwy_4unsep();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 15, "net_FourLaneUSHighwaySeparated");
		hwy_reg_s();
		hwy_2lane_L(); make_spacer(3.0); hwy_2lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 16, "net_FourLaneUSHighwaySeparatedOverpass");
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

	hwy_start(0, 17, "net_FourLaneUSHighwaySeparatedNoGuardRails");
		start_composite();
		hwy_2lane_L(); make_spacer(3.0); hwy_2lane_R();
		end_composite();
	road_end();
	
	hwy_start(2, 90, "net_FourLaneUSHighwaySeparatedSuspensionBridge");
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

	hwy_start(2, 91, "net_FourLaneUSHighwaySeparatedArchBridge");
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
	
	hwy_start(0, 18, "net_FourLaneUSHighwayOneway");
		hwy_reg_s();
		hwy_4oneway();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,19, "net_FourLaneUSHighwayOnewayOverpass");
		hwy_ovr_s(0);
		hwy_4oneway();
		hwy_ovr_e(0);
	road_end();

	hwy_start(2,92, "net_SixLaneUSHighwayOnewaySuspensionBridge");
		hwy_sus_s(0);
		hwy_4oneway();
		hwy_sus_e(0);
	road_end();

	hwy_start(2,93, "net_SixLaneUSHighwayOnewayArchBridge");
		hwy_arc_s(1);
		hwy_4oneway();
		hwy_arc_e(1);
	road_end();
	
	hwy_start(0, 20, "net_FourLaneUSHighwayWithTrain");
		hwy_reg_s();
		hwy_4unseptr();
		hwy_reg_e();
	road_end();
	
	hwy_start(1, 21, "net_FourLaneUSHighwayWithTrainOverpass");
		hwy_ovr_s(1);
		hwy_4unseptr();
		hwy_ovr_e(1);
	road_end();

	hwy_start(2, 94, "net_FourLaneUSHighwayWithTrainSuspensionBridge");
		hwy_sus_s(1);
		hwy_4unseptr();
		hwy_sus_e(1);
	road_end();

	hwy_start(2, 95, "net_FourLaneUSHighwayWithTrainArchBridge");
		hwy_arc_s(1);
		hwy_4unseptr();
		hwy_arc_e(1);
	road_end();

	hwy_start(0, 22, "net_FourLaneUSHighwaySeparatedWithTrain");
		hwy_reg_s();
		hwy_2lane_L(); 	hwy_trains(); make_spacer(3.0); hwy_2lane_R();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1, 23, "net_FourLaneUSHighwaySeparatedWithTrainOverpass");
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

	hwy_start(2, 96, "net_FourLaneUSHighwaySeparatedWithTrainSuspensionBridge");
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

	hwy_start(2, 97, "net_FourLaneUSHighwaySeparatedWithTrainArchBridge");
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

	hwy_start(0, 24, "net_FourLaneUSHighwayOnewayWithTrain");
		hwy_reg_s();
		hwy_4onewaytr();
		hwy_guard_rail(); end_composite();
	road_end();
	
	hwy_start(1,25, "net_FourLaneUSHighwayOnewayWithTrainOverpass");
		hwy_ovr_s(0);
		hwy_4onewaytr();
		hwy_ovr_e(0);
	road_end();

	hwy_start(2,98, "net_FourLaneUSHighwayOnewayWithTrainSuspensionBridge");
		hwy_sus_s(0);
		hwy_4onewaytr();
		hwy_sus_e(0);
	road_end();

	hwy_start(2,99, "net_FourLaneUSHighwayOnewayWithTrainArchBridge");
		hwy_arc_s(1);
		hwy_4onewaytr();
		hwy_arc_e(1);
	road_end();

	printf("#### PRIMARY roads: 26-42 = primary, primary overpass, 63-71 = primary girder.\n");

	sec_start(0, 26, "net_PrimaryUndivided");
		sec_reg_s();
		sec_prim_undiv();
		sec_reg_e();
	road_end();
	
	sec_start(1,27, "net_PrimaryUndividedOverpass");
		sec_ovr_s(1);
		sec_prim_undiv();
		sec_ovr_e(1);
	road_end();

	sec_start(2,63, "net_PrimaryUndividedBridge");
		sec_grd_s(1);
		sec_prim_undiv();
		sec_grd_e(1);
	road_end();

	sec_start(0, 28, "net_PrimaryUndividedWithSidewalks");
		sec_reg_s();
		sec_prim_undiv_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,29, "net_PrimaryUndividedWithSidewalksOverpass");
		sec_ovr_s(1);
		sec_prim_undiv_s();
		sec_ovr_e(1);
	road_end();

	sec_start(2,64, "net_PrimaryUndividedWithSidewalksBridge");
		sec_grd_s(1);
		sec_prim_undiv_s();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 30, "net_PrimaryUndividedWithTrains");
		sec_reg_s();
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,31, "net_PrimaryUndividedWithTrainsOverpass");
		sec_ovr_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_ovr_e(1);
	road_end();

	sec_start(2,65, "net_PrimaryUndividedWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_prim_R();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 32, "net_PrimaryUndividedWithSidewalksWithTrains");
		sec_reg_s();
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,33, "net_PrimaryUndividedWithSidewalksWithTrainsOverpass");
		sec_ovr_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_ovr_e(1);
	road_end();

	sec_start(2,66, "net_PrimaryUndividedWithSidewalksWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
	
	sec_start(0, 34, "net_PrimaryDivided");
		sec_reg_s();
		sec_prim_L();
		sec_space();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,35, "net_PrimaryDividedOverpass");
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

	sec_start(2,67, "net_PrimaryDividedBridge");
		sec_grd_s(1);
		sec_prim_L();
		sec_grd_b(1);
		sec_prim_R();
		sec_grd_e(1);
	road_end();

	sec_start(0, 36, "net_PrimaryDividedWithSidewalks");
		sec_reg_s();
		sec_prim_L_s();
		sec_space();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,37, "net_PrimaryDividedWithSidewalksOverpass");
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

	sec_start(2,68, "net_PrimaryDividedWithSidewalksBridge");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_grd_b(1);
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 38, "net_PrimaryDividedWithTrains");
		sec_reg_s();
		sec_prim_L();
		sec_prim_tr();
		sec_space();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,39, "net_PrimaryDividedWithTrainsOverpass");
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

	sec_start(2,69, "net_PrimaryDividedWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_L();
		sec_prim_tr();
		sec_grd_b(1);
		sec_prim_R();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 40, "net_PrimaryDividedWithSidewalksWithTrains");
		sec_reg_s();
		sec_prim_L_s();
		sec_prim_tr();
		sec_space();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,41, "net_PrimaryDividedWithSidewalksWithTrainsOverpass");
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

	sec_start(2,70, "net_PrimaryDividedWithSidewalksWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_L_s();
		sec_prim_tr();
		sec_grd_b(1);
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();

	sec_start(0, 100, "net_PrimaryOneway");
		sec_reg_s();
		sec_prim_R();
		sec_reg_e();
	road_end();
	
	sec_start(1,101, "net_PrimaryOnewayOverpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_R();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(2,108, "net_PrimaryOnewayBridge");
		sec_grd_s(1);
		sec_prim_R();
		sec_grd_e(1);
	road_end();

	sec_start(0, 102, "net_PrimaryOnewayWithSidewalks");
		sec_reg_s();
		sec_prim_R_s();
		sec_reg_e();
	road_end();
	
	sec_start(1,103, "net_PrimaryOnewayWithSidewalksOverpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_R_s();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(2,109, "net_PrimaryOnewayWithSidewalksBridge");
		sec_grd_s(1);
		sec_prim_R_s();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 104, "net_PrimaryOnewayWithTrains");
		sec_reg_s();
		sec_prim_R();
		sec_prim_tr();
		sec_reg_e();
	road_end();
	
	sec_start(1,105, "net_PrimaryOnewayWithTrainsOverpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_R();
		sec_prim_tr();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(2,110, "net_PrimaryOnewayWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_R();
		sec_prim_tr();
		sec_grd_e(1);
	road_end();
		
	sec_start(0, 106, "net_PrimaryOnewayWithSidewalksWithTrains");
		sec_reg_s();
		sec_prim_R_s();
		sec_prim_tr();
		sec_reg_e();
	road_end();
	
	sec_start(1,107, "net_PrimaryOnewayWithSidewalksWithTrainsOverpass");
		start_composite();
		sec_ovr_s(0);
		sec_prim_R_s();
		sec_prim_tr();
		sec_ovr_e(0);
		end_composite();
	road_end();

	sec_start(2,111, "net_PrimaryOnewayWithSidewalksWithTrainsBridge");
		sec_grd_s(1);
		sec_prim_R_s();
		sec_prim_tr();
		sec_grd_e(1);
	road_end();

	printf("#### SECONDAARY roads: 42-47 = secondary, secondary overpass, 71-73 = stone bridge.\n");

	sec_start(0, 42, "net_SecondaryRoadWithSidewalks");
		sec_reg_s();
		sec_sec_s();
		sec_reg_e();
	road_end();

	sec_start(1,43, "net_SecondaryRoadWithSidewalksOverpass");
		sec_ovr_s(0);
		sec_sec_s();
		sec_ovr_e(0);
	road_end();

	sec_start(2,71, "net_SecondaryRoadWithSidewalksBridge");
		sec_stn_s(1);
		sec_sec_s();
		sec_stn_e(1);
	road_end();

	sec_start(0, 44, "net_SecondaryRoad");
		sec_reg_s();
		sec_sec();
		sec_reg_e();
	road_end();

	sec_start(1,45, "net_SecondaryRoadOverpass");
		sec_ovr_s(0);
		sec_sec();
		sec_ovr_e(0);
	road_end();

	sec_start(1,46, "net_SecondaryRoadPassing");
		sec_reg_s();
		sec_sec_p();
		sec_reg_e();
	road_end();

	sec_start(2,72, "net_SecondaryRoadBridge");
		sec_stn_s(1);
		sec_sec();
		sec_stn_e(1);
	road_end();

	printf("#### LOCALS 47-53, 58-61, 73.\n");
	
	lcl_start(0, 47, "net_LocalRoad");
		lcl_marked2();
	road_end();
	
	lcl_start(1, 48, "net_LocalRoadOverpass");
		lcl_pylon_start();
		lcl_underside_start();
		start_composite();
		lcl_overpass_side();
		lcl_marked2();
		lcl_overpass_side();
		end_composite();
		underside_end();
		pylons_end();
	road_end();
	
	lcl_start(2, 73, "net_LocalRoadBridge");
		lcl_underarch();
		start_composite();
		lcl_arch_side();
		make_spacer(0.6);
		lcl_marked2();
		make_spacer(0.6);
		lcl_arch_side();
		end_composite();
		underside_end();
	road_end();
	
	lcl_start(0, 49, "net_CulDeSac");
		lcl_marked1();
	road_end();
	
	lcl_start(1, 50, "net_AccessRamp");
		lcl_pylon_start();
		lcl_underside_start();
		start_composite();
		lcl_overpass_side();
		lcl_marked1();
		lcl_overpass_side();
		end_composite();
		underside_end();
		pylons_end();
	road_end();	

	lcl_start(0, 51, "net_4WDRoad");
		lcl_4wd();
	road_end();
	
	lcl_start(0, 52, "net_Alley");
		lcl_unmarked1();
	road_end();
	
	lcl_start(0, 53, "net_Driveway");
		lcl_unmarked1();
	road_end();
	
	lcl_start(0, 58, "net_Walking");
		lcl_walk();
	road_end();
	
	lcl_start(0, 59, "net_WalkingCity");
		lcl_walk();
	road_end();
	
	lcl_start(1, 60, "net_WalkingCityOverpass");
		lcl_pylon_start();
		lcl_underwalk();
		start_composite();
		lcl_walk_side();
		lcl_walk_over();
		lcl_walk_side();
		end_composite();
		underside_end();
		pylons_end();
	road_end();		

	
	lcl_start(0, 61, "net_WalkingCitySteps");
		lcl_walk();
	road_end();
	
	printf("#### Trains 54-57, 74-75\n");

	tra_start(0, 54, "net_TrainsTwoWay");
		tra_two();
	road_end();

	tra_start(1, 55, "net_TrainsTwoWayOverpass");
		tra_pylon_start();
		start_composite();
		tra_underside_start();
		start_composite();
		tra_over_side();
		tra_one_ovr_r();
		end_composite();
		underside_end();
		tra_underside_start();
		start_composite();
		tra_one_ovr_f();
		tra_over_side();
		end_composite();
		underside_end();
		end_composite();
		pylons_end();
	road_end();

	tra_start(2, 74, "net_TrainsTwoWayBridge");
		tra_pylon_start();
		start_composite();
		tra_underbridge_start();
		tra_roof_start();
		start_composite();
		tra_bridge_side();
		tra_one_ovr_r();
		end_composite();
		underside_end();
		underside_end();
		tra_bridge_side();
		tra_underbridge_start();
		tra_roof_start();
		start_composite();
		tra_one_ovr_f();
		tra_bridge_side();
		end_composite();
		underside_end();
		underside_end();
		end_composite();
		pylons_end();
	road_end();

	tra_start(0, 56, "net_TrainsOneWay");
		tra_one();
	road_end();

	tra_start(1, 57, "net_TrainsOneWayOverpass");
		tra_pylon_start();
		tra_underside_start();
		start_composite();
		tra_over_side();
		tra_one_ovr_f();
		tra_over_side();
		end_composite();
		underside_end();
		pylons_end();
	road_end();
	
	tra_start(2, 75, "net_TrainsOneWayBridge");
		tra_pylon_start();
		tra_underbridge_start();
		tra_roof_start();
		start_composite();
		tra_bridge_side();
		tra_one_ovr_f();
		tra_bridge_side();
		end_composite();
		underside_end();
		underside_end();
		pylons_end();
	road_end();

printf("####################################################################\n");
printf("# POWER LINES\n");
printf("####################################################################\n");
	
printf("# net_Powerlines\n");
printf("ROAD_TYPE 62   25.000000 25.000000 3   1.0 1.0 0.0 \n");
printf("SHOW_LEVEL 1\n");
printf("WIRE        0 30000   0.15 22.8 0.4\n");
printf("WIRE        0 30000   0.85 22.8 0.4\n");
printf("OBJECT      powerline_tower.obj 0.5 0.0 0 0.0 0.0\n");
	

	return 0;
}



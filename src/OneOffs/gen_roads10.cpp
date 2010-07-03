// To compile from the terminal: in the dir where these files live, type
// g++ gen_roads10.cpp -o gen_roads10
// This will make a program called gen_roads10
// to RUN, type
// ./gen_roads10 > file.net
// where file.net is the output file to make.  Note that this can be in some dir like
// ./gen_roads10 > /Volumes/RAID/code/design/Resources/default\ scenery/1000\ roads/road.net
// (Note use of \ before spaces.)

// This includes the C++ types that make a somewhat sane file possible.
#include "gen_roads10.h"
#include <map>

int main(int argc, const char ** argv)
{
	//----------------------------------------------------------------------------------------------------
	// LOD and length declarations
	//----------------------------------------------------------------------------------------------------
	// To avoid having a million different LODs (or typos in those LODs) we declare a small number of
	// "standard" LODs right here that we will use.
	
	lod_range	lod_standard = { 0, 20000 };
	lod_range	lod_detail = { 0, 2000 };
	
	// These are length declarations.  The first number is the length of the whole texture in meters, and
	// the second is the "granularity" for cutting it with require_even.  If we don't care where it is
	// cut, specify a second number of 0.0 and require_even won't be used.
		
	vert_props	len_normal = { 100.0, 25.0 };
	vert_props	len_fence = { 25.0, 25.0 };

	//----------------------------------------------------------------------------------------------------
	// SHADER DECLARATIONS
	//----------------------------------------------------------------------------------------------------
	// In this section we declare a "shader" object for every shader we want in the file...in the order
	// that they will appear in the file.  
	// Tiling params are: x tiles, y tiles, x bias, y bias, x wavelength, y wavelength.
	
	shader	base_offset("roads_1000_provisional1.dds",1, 2048);
			base_offset.set_tile(1,4,0.25, 1.0, 100.0, 200.0);

	shader	base_normal("roads_1000_provisional1.dds",0, 2048);
			base_normal.set_tile(1,4,0.25, 1.0, 100.0, 200.0);

	//----------------------------------------------------------------------------------------------------
	// VEHICLES
	//----------------------------------------------------------------------------------------------------

	// Trivial vehicles are declared into a traffic object as follows:
	traffic	car("lib/cars/car.obj");
	traffic truck("lib/cars/car_or_truck.obj");

	// For trains, things are slightly more tricky.   Each train car is individually
	// declared (here we have a virtual path into the lib and then the two half-lengths of the train, just
	// as in the old format).
	train_car		HS_head("lib/trains/HS_head.obj",11.0,9.64);
	train_car		HS_body("lib/trains/HS_body.obj",13.88,13.72);
	train_car		HS_tail("lib/trains/HS_tail.obj",9.84,10.9);
		
	train_car		R_head("lib/trains/R_head.obj",		10	,	6.37);
	train_car		R_body("lib/trains/R_body.obj",		6.6	,	6.2	);
	train_car		R_tail("lib/trains/R_tail.obj",		6.84,	9.7	);

	train_car		IC_head("lib/trains/IC_head.obj",	15.8,	13.924);
	train_car		IC_powr("lib/trains/IC_powr.obj",	13.8,	13.68);
	train_car		IC_body("lib/trains/IC_body.obj",	13.8,	13.68);
	train_car		IC_tail("lib/trains/IC_tail.obj",	14.26,	15.47);

	train_car		F_head("lib/trains/F_head.obj",		11.46,	11.46);
	train_car		F_13  ("lib/trains/F_13_33.obj",	6.665,	6.665);
	train_car		F_20  ("lib/trains/F_20.06.obj",	10.03,	10.03);
	train_car		F_23  ("lib/trains/F_23.88.obj",	11.94,	11.94);
	train_car		F_17  ("lib/trains/F_17.33.obj",	8.665,	8.665);
	train_car		F_18  ("lib/trains/F_18.31.obj",	9.155,	9.155);
	train_car		F_tail("lib/trains/F_tail.obj",		11.46,	11.46);

	// We can declare a "traffic" object that takes the SUM of train cars to spell
	// a train.
	traffic train_freight(F_head + F_13 + F_20 + F_23 + F_17 + F_18 + F_tail);	
	traffic	train_passenger(HS_head + HS_body + HS_body + HS_body + HS_tail);
	
	// If we then add a new "spelling" it makes VARIANTS.  So the passenger train
	// has THREE variants...a 5-car high speed train, a 3 car regional train,
	// and a 5 car inter-city train.
	train_passenger += (R_head + R_body + R_tail);
	train_passenger += (IC_head + IC_body + IC_powr + IC_body + IC_tail);

	//----------------------------------------------------------------------------------------------------
	// ROAD CORES
	//----------------------------------------------------------------------------------------------------
	// Here we declare our road cores.  We will build DRAPED versions of every road core, then 
	// automatically derive the graded versions and center-alignment markings.
	//
	// In some cases, we are simply copying atoms inappropriately to have a complete set of information.

	map<int, road>	road_cores_graded, road_cores_draped;	

	// This declares a simple asphalt standard LOD deck based on the normal shader.  Numbers are a pair of x,y,s coordinates.
	road	primary_left = make_deck_draped(base_offset, lod_standard, len_normal, 0,334,8.5625,402.5,asphalt);
	road	primary_right = make_deck_draped(base_offset, lod_standard,len_normal,  0,447.5,8.5625,516,asphalt);
	road	residential	= make_deck_draped(base_offset, lod_standard, len_normal, 0.0,589,9.0,661,asphalt);
	
	road_cores_draped[15] = make_deck_draped(base_offset, lod_standard, len_normal, 0,1,16.25,131,asphalt);	// 6 lane city
	road_cores_draped[16] = road_cores_draped[15];	// Clone 6 lane city for rural
	
	road_cores_draped[17] = make_deck_draped(base_offset, lod_standard, len_normal, 0,242, 7,298,asphalt);

	road_cores_draped[20] = make_deck_draped(base_offset, lod_standard, len_normal, 0,1455, 5,1490, none);
	
	road_cores_draped[1] = primary_left + primary_right;	
	road_cores_draped[3] = primary_right;
	road_cores_draped[5] = primary_left + primary_right;			// copy primary to secondary for now
	road_cores_draped[7] = primary_right;
	road_cores_draped[9] = residential;
	road_cores_draped[11] = primary_right;
	
	// This goes through and marks the center of every road core as the centerline point.  (The center
	// is computed for us.)  Only one road atom can have a center marker when we merge road atoms, so
	// we mark the cores and nothing else.
	for(map<int,road>::iterator r = road_cores_draped.begin(); r != road_cores_draped.end(); ++r)
	{
		// Cars: ptr to car, pixel ofset, velocity in meters, density, and then optionally: 0 for same dir, 1 for reverse direction
		r->second.add_traffic(&car  , 34, 35, 0.06);
		r->second.add_traffic(&car  , 62, 30, 0.06);
		r->second.add_traffic(&truck, 90, 25, 0.06);

		r->second.add_traffic(&car  , 170, 30, 0.06);
		r->second.add_traffic(&truck, 199, 25, 0.06);

		r->second.add_traffic(&truck, 265, 25, 0.06);


		r->second.add_traffic(&truck, 363, 25, 0.06, 1);
		r->second.add_traffic(&car  , 386, 20, 0.06, 1);
		r->second.add_traffic(&car  , 461, 20, 0.06, 0);
		r->second.add_traffic(&truck, 486, 25, 0.06, 0);

		r->second.add_traffic(&train_freight, 1473, 25, 0.01);

		r->second.set_center_at_center();
		road_cores_graded[r->first] = graded_from_draped(base_normal, r->second);
	}
	
	// This is just idiotic test code to show how to make objs...
	
	// This defines 2-band perlin noise.  Wavelengths are 500 and 50 m (bands 3 and 4 ignored, amps are 90%, 1%).
	perlin_params stringie = { 500, 50, 0, 0, 0.9, 0.1, 0, 0 };	
	
	// This defines a set of obj alternatives.  NULL at end is necessary!!
	const char * cars[] = { "local_pylon.obj", "cart.obj", "cafe.obj", NULL };
	
	// Example 1: add a set of obj choices 1 meter in from the left side of our primary core a random object with perlin frequency.  This obj exists 30% of the time.  (See last two params)
	road_cores_draped[1].add_obj_left(cars,graded, 1, 1, 0, 0, 10, 12, 15, 15, 0, 0.3, &stringie);
	// Example 2: add a specific obj 1 meter to the left of the right edge (-1 = left of center and this obj adds to the right).
	// This obj has no frequency info.
	road_cores_draped[1].add_obj_right("ramp_pylon.obj",graded, -1, -1, 0, 0, 10, 12, 15, 15, 0, 0, NULL);
	
	//----------------------------------------------------------------------------------------------------
	// OUTER ATTACHMENTS AND CRUD
	//----------------------------------------------------------------------------------------------------
	// Here we build our attachment atoms.  IN most cases, atoms are "filed" by a 2 digit code, the first
	// being the attachment type and the seocnd being the draping suffix.

	map<int, road>	left_embankment_rural;
	map<int, road>	right_embankment_rural;
	map<int, road>	left_embankment_city;
	map<int, road>	right_embankment_city;

	// Using | to "or" together roads simply merges all elements in place.  These are embankments, stored in the format
	// climate * 10 + grading.  So this is cilmate = 0, grading = 2, which is the 30 degree grading of a wet climate.
	left_embankment_rural[ 2] =	make_deck_graded(base_normal, lod_standard, len_normal, 0.0,-3.0,776,		6.0,0.0,830,grass) |
								make_deck_graded(base_normal, lod_standard, len_normal, 6.0,0.0,830,		7.25,0.0,840,gravel) |
								make_deck_graded(base_normal, lod_standard, len_fence, 6.125,0.0,842.5,		6.125,0.75,848,none) |
								make_deck_graded(base_normal, lod_standard, len_fence, 6.125,0.75,848,		6.125,0.0,842.5,none);

	right_embankment_rural[ 2] =make_deck_graded(base_normal, lod_standard, len_normal, 0,0,840,			1.25,0,830,grass) | 
								make_deck_graded(base_normal, lod_standard, len_normal, 1.25,0,830,			7.25,-3,776,grass);

	map<int, road>	left_sidewalks;
	map<int, road>	right_sidewalks;

	// Sidewalks, stored in sidewalk code * 10 + grading.  So these are the draped sidewalks, normal and wide.
	left_sidewalks [11] = make_deck_draped(base_offset,lod_standard,len_normal, 0,569,2.5,589,none);
	right_sidewalks[11] = make_deck_draped(base_offset,lod_standard,len_normal, 0,661,2.5,681,none);	
	left_sidewalks [21] = make_deck_draped(base_offset,lod_standard,len_normal, 0,298,4.5,334,none);
	right_sidewalks[21] = make_deck_draped(base_offset,lod_standard,len_normal, 0,516,4.5,551,none);


	map<int, road>	left_rail;
	map<int, road>	right_rail;
	left_rail [12] = make_deck_graded(base_normal, lod_standard, len_normal, 0  ,  -12, 1629, 0  , 1.25, 1718, none) |
					 make_deck_graded(base_normal, lod_standard, len_normal, 0  , 1.25, 1718, 0.5, 1.25, 1721, none) |
					 make_deck_graded(base_normal, lod_standard, len_normal, 0.5, 1.25, 1721, 0.5, 0   , 1731, none);

	right_rail[12] = make_deck_graded(base_normal, lod_standard, len_normal, 0  , 0   , 1731, 0  , 1.25, 1721, none) |
					 make_deck_graded(base_normal, lod_standard, len_normal, 0  , 1.25, 1721, 0.5, 1.25, 1718, none) |
					 make_deck_graded(base_normal, lod_standard, len_normal, 0.5, 1.25, 1718, 0.5,-12.0, 1629, none);

	//----------------------------------------------------------------------------------------------------
	// OUTPUT ALL COMBINATIONS OF REAL ROAD TYPES
	//----------------------------------------------------------------------------------------------------
	// These sets of nested for loops actually combine every type with every other type to maek the file.

	int * core;
	const char ** name;

	{
		int highway_cores[] = { 15, 16, 17, 0 };
		const char * highway_names[] = { "6-lane", "4-lane", "ramp" };
		float	schedule_city[] = { 0.0, 10.0, 20.0, 0.0 };	// drape, 90 deg, bridge
		float	schedule_rural[] = { 0.0, 3.0, 6.0, 10.0, 20.0, 0.0 };	// drape, 30 deg, 60 deg, 90 deg, bridge
		const char * climate_names[] = { " wet ", " dry " };
		const char * rural_grading_names[] = { "", "(draped)", "(30 deg)", "(60 deg)", "(90 deg)", "(bridged)" };
		const char * city_grading_names[] = { "", "(draped)", "(90 deg)", "(bridged)" };
		
		for(core = highway_cores, name = highway_names; *core; ++core, ++name)
		for(int climate = 0; climate < 2; ++climate)
		{
			int need_drape = *core == 17;
		
			for(int grading = (need_drape ? 1 : 2); grading < 4; ++grading) 
				publish_road(
					*core * 1000 + climate * 100 + grading,
					string(*name) + " city "+ climate_names[climate] + city_grading_names[grading],
					1, 0.6, 0.6, 0.6,
					left_embankment_city[climate * 10 + grading] + (grading == 1 ? road_cores_draped[*core] : road_cores_graded[*core]) + right_embankment_city[climate * 10 + grading]);

			if(need_drape)
				make_draped(string(*name) + " city "+ climate_names[climate], *core * 10 + climate, 0.1, 0.2, schedule_city);
			else
				make_graded(string(*name) + " city "+ climate_names[climate], *core * 10 + climate, 0.1, schedule_city);



			for(int grading = (need_drape ? 1 : 2); grading < 6; ++grading) 
				publish_road(
					*core * 1000 + 3000 + climate * 100 + grading,
					string(*name) + " rural " + climate_names[climate] + rural_grading_names[grading],
					1, 0.6, 0.6, 0.6,
					left_embankment_rural[climate * 10 + grading] + (grading == 1 ? road_cores_draped[*core] : road_cores_graded[*core]) + right_embankment_rural[climate * 10 + grading]);

			if(need_drape)
				make_draped(string(*name) + " rural " + climate_names[climate], *core * 10 + 30 + climate, 0.1, 0.2, schedule_rural);
			else
				make_graded(string(*name) + " rural " + climate_names[climate], *core * 10 + 30 + climate, 0.1, schedule_rural);

		}
	}


	float	schedule_railway[] = { 0.0, -12.0, 0.0 };					// drape, 90 bridge


	publish_road(21102, "rail", 1, 0.5, 0.2, 0.2, left_rail[12] + road_cores_graded[20] + right_rail[12]);
	make_graded("rail", 211, 0.05, schedule_railway);

	// This crosses our 4 city cores with our sidewalk combinations...
	{
		int city_cores[] = { 1,3,5,7,9,11,0 };
		const char * city_names[] = { "Primary", "Primary Oneway", "Secondary", "Secondary Oneway", "Local", "Local Oneway" };
		const char * climate_names[] = { " Wet ", " Dry " };
		const char * sidewalk_names[] = { "None", "Sidewalk", "Widewalk" };
		const char * grading_names[] = { "", " (Draped)", " (Embanked)", " (Bridged)" };
		float	schedule_city[] = { 0.0, 10.0, 15.0, 0.0 };					// drape, 90 deg, bridge
		
		for(core = city_cores, name = city_names; *core; ++core, ++name)
		for(int climate = 0; climate < 2; ++climate)
		for(int sidewalkR = 0; sidewalkR < 3; ++sidewalkR)
		for(int sidewalkL = 0; sidewalkL < 3; ++sidewalkL)
		{
			for(int grading = 1; grading < 4; ++grading)
				publish_road(
					*core * 1000 + climate * 1000 + sidewalkL * 100 + sidewalkR * 300 + grading,
					string(*name) + climate_names[climate] + sidewalk_names[sidewalkL] + "/" + sidewalk_names[sidewalkR] + grading_names[grading],
					2, 0.5, 0.5, 0.5, 
					left_sidewalks[sidewalkL*10+grading] + 
					(grading == 1 ? road_cores_draped[*core] : road_cores_graded[*core]) + 
					right_sidewalks[sidewalkR*10+grading]);
			make_draped(string(*name) + climate_names[climate] + sidewalk_names[sidewalkL] + "/" + sidewalk_names[sidewalkR], *core * 10 + climate * 10 + sidewalkL + sidewalkR * 3, 0.1, 1.0, schedule_city);
		}
	}


	// This make a single real-type powerline.

	road	power_lines = make_wire(lod_standard, 0 , 22.8, 0.4) |
						  make_wire(lod_standard, 25, 22.8, 0.4);
	power_lines.add_obj_left("powerline_tower.obj", graded, 12.5, 12.5, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
	power_lines.set_center_at_center();
	publish_road(
					220,
					"power lines",
					1, 1,1,0,
					power_lines);
				


	// Finally, we call this to cause the program to output all of this stuff to a single road.net file.
	output(stdout);
	return 0;
}

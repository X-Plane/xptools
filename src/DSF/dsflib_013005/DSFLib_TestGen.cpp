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
#include "DSFLib.h"
#include "DSFDefs.h"
#if LIN
#include <stdlib.h> /* for rand() */
#endif

// +34-118

static char * kFacs[] = {
"ind/construction.fac",
"ind/warehouse.fac",
"ind/storage.fac",
"res/lowapt.fac",
"res/midapt.fac",
"res/motel.fac",
"res/hotel.fac",
"res/rowhouses.fac",
"res/walkups.fac",
"com/skyscraper.fac",
"com/lowoffice.fac",
"com/medoffice.fac",
"com/convertrow.fac",
"com/rowshops.fac",
"ins/cityschool.fac",
"ins/university.fac",
"ins/jail.fac",
"ins/capitalbuilding.fac",
0
};

static char *	kObjs[] = {
"ind/refinery.obj",
"ind/crane.obj",
"ind/construction.obj",

"ind/gastank.obj",
"ind/smokestacks.obj",

"ind/warehouse.obj",

"ind/self_storage.obj",

"ind/power_coal.obj",
"ind/power_nuke.obj",
"ind/power_hydro.obj",
"ind/power_gas.obj",
"ind/truckstop.obj",
"res/trailerpark.obj",
"res/trailerparkden.obj",
"res/lowapt.obj",

"res/midapt.obj",

"res/aptcomp.obj",
"res/duplex.obj",
"res/starterhome.obj",
"res/singlehouse.obj",
"res/bighouse.obj",
"res/model.obj",

"res/hotel.obj",

"res/luxuryhotel.obj",
"res/rowhouses.obj",

"res/walkups.obj",

"farm/farmhouse.obj",
"farm/grainsilo.obj",
"farm/firetower.obj",
"com/offtrailers.obj",

"com/skyscraper.obj",

"com/lowoffice.obj",

"com/medoffice.obj",

"com/smallstrip.obj",
"com/largestrip.obj",
"com/smallmall.obj",
"com/largevmall.obj",
"com/largehmall.obj",
"com/bankbrach.obj",
"com/bankcenter.obj",
"com/fastfood.obj",
"com/convertsingle.obj",
"com/convertrow.obj",

"com/rowshops.obj",

"com/repairshop.obj",
"trans/marina.obj",
"trans/seaterminal.obj",
"trans/busstation.obj",
"trans/trainstation.obj",
"trans/cargoloader.obj",
"trans/coolingtower.obj",
"trans/gasstation.obj",
"trans/truckstop.obj",
"trans/radiotower.obj",
"rec/tent.obj",
"rec/tents.obj",
"rec/cabin.obj",
"rec/cabins.obj",
"rec/golfhole.obj",
"rec/amusement.obj",
"rec/arcade.obj",
"rec/cassino.obj",
"rec/football.obj",
"rec/baseball.obj",
"rec/arena.obj",
"rec/playground.obj",
"rec/pool.obj",
"rec/fountain.obj",
"ins/barracks.obj",
"ins/supplydepot.obj",
"ins/ruralhospital.obj",
"ins/cityhospital.obj",
"ins/cityhospitalhelo.obj",
"ins/medicalcomplex.obj",
"ins/ruralschool.obj",
"ins/cityschool.obj",

"ins/campus_school.obj",
"ins/university.obj",

"ins/postoffice.obj",
"ins/jail.obj",

"ins/policestation.obj",
"ins/firestation.obj",
"ins/capitalbuilding.obj",

"ins/courthouse.obj",
"ins/church.obj",
"ins/synagogue.obj",
"ins/mosque.obj",
"ins/temple.obj",
"misc/flagpole.obj",
"misc/elevator.obj",
"misc/windmill.obj",
"misc/arch.obj",
"misc/monument.obj",
"misc/statue.obj",
"misc/obelisk.obj",
"misc/dome.obj",
"misc/sign.obj",
0
};

// 20 km cutover (a little over 10 miles)
#define	LOD_CUTOVER		20000

int	RRI(int a, int b)
{
	return (rand() % (b - a)) + a;
}

float RRF(float a, float b)
{
	return (rand() % (int)(1000 * (b - a))) / 1000.0 + a;
}


//************************************************************************************************************************
// DSF TEST NUMBER 1 - BASIC FEATURE TEST
//************************************************************************************************************************
#if 0
void	GenFakeDSFFile(const char * path)
{
	void * f = DSFCreateWriter(-118.0, 34.0, -117.0, 35.0, 8);
	DSFCallbacks_t	cbs;
	DSFGetWriterCallbacks(&cbs);

	cbs.AcceptProperty_f("sim/west", "-118", f);
	cbs.AcceptProperty_f("sim/east", "-117", f);
	cbs.AcceptProperty_f("sim/south", "34", f);
	cbs.AcceptProperty_f("sim/north", "35", f);
	cbs.AcceptTerrainDef_f("water", f);
	cbs.AcceptTerrainDef_f("test_terrain.ter", f);
	cbs.AcceptTerrainDef_f("test_terrain2.ter", f);
	cbs.AcceptTerrainDef_f("test_terrain3.bmp", f);
	cbs.AcceptTerrainDef_f("grass/Grass0_000.bmp", f);
	cbs.AcceptTerrainDef_f("test_terrain4.ter", f);
	cbs.AcceptObjectDef_f("KSBD_example.obj", f);
	cbs.AcceptPolygonDef_f("terminal.fac", f);
	cbs.AcceptPolygonDef_f("testforest.for", f);
	cbs.AcceptNetworkDef_f("test.net", f);

double	OFFSET= 0.0078125;
//	double OFFSET =	0.125;

	double	oc[2];
	for (float i = OFFSET+OFFSET/2; i < (1.0-OFFSET); i += OFFSET)
	for (float j = OFFSET+OFFSET/2; j < (1.0-OFFSET); j += OFFSET)
	{
		oc[0] = -118.0 + i + RRF(-OFFSET, OFFSET);
		oc[1] = 34.0 + j + RRF(-OFFSET, OFFSET);
		cbs.AddObject_f(0, oc, 0.0, f);
	}

	double	pc[7];


		OFFSET =	0.0625;
//#define OFFSET 0.0078125

	int	prim = 0;
	for (double i = 0.0; i < 1.0; i += OFFSET)
	{
		++prim;

		for (double j = 0.0; j < 1.0; j += OFFSET)
		{
			++prim;
			if (prim > 5) prim = 0;
			if (prim == 1 || prim == 2)
			{
				cbs.BeginPatch_f(0, 0.0, LOD_CUTOVER, dsf_Flag_Physical, NULL, NULL, 5, f);
				cbs.BeginPrimitive_f(dsf_Tri, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				cbs.EndPrimitive_f(f);
				cbs.EndPatch_f(f);
			}
			cbs.BeginPatch_f(prim, 0.0, LOD_CUTOVER, dsf_Flag_Physical, NULL, NULL, (prim == 1) ? 7 : 5, f);
			cbs.BeginPrimitive_f(dsf_Tri, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			cbs.EndPrimitive_f(f);
			cbs.EndPatch_f(f);
		}
	}

//	SECOND LOD

		OFFSET *= 2;
//#define OFFSET 0.0078125

	prim = 0;
	for (double i = 0.0; i < 1.0; i += OFFSET)
	{
		++prim;

		for (double j = 0.0; j < 1.0; j += OFFSET)
		{
			++prim;
			if (prim > 5) prim = 0;
			if (prim == 1 || prim == 2)
			{
				cbs.BeginPatch_f(0, LOD_CUTOVER, -1.0, 0, NULL, NULL, 5, f);
				cbs.BeginPrimitive_f(dsf_Tri, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 0.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
				pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[3] = 1.0; pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
				cbs.EndPrimitive_f(f);
				cbs.EndPatch_f(f);
			}
			cbs.BeginPatch_f(prim, LOD_CUTOVER, -1.0, 0, NULL, NULL, (prim == 1) ? 7 : 5, f);
			cbs.BeginPrimitive_f(dsf_Tri, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0  		+ i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 0.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0 + OFFSET + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 1.0;		cbs.AddPatchVertex_f(pc, f);
			pc[0] = -118.0 + OFFSET + i;	pc[1] = 34.0  		  + j;	pc[2] = 0.0;	pc[6] = pc[3] = 1.0; pc[5] = pc[4] = 0.0;		cbs.AddPatchVertex_f(pc, f);
			cbs.EndPrimitive_f(f);
			cbs.EndPatch_f(f);
		}
	}

	OFFSET =	0.0625;

	double	build_size = 0.0001645819618169849;
	for (double i = OFFSET*2+OFFSET/3; i < (1.0-OFFSET*2); i += (OFFSET * 3))
	{
		++prim;
		for (double j = OFFSET*2+OFFSET/3; j < (1.0-OFFSET*2); j += (OFFSET * 3))
		{
			cbs.BeginPolygon_f(0,3, f);
			cbs.BeginPolygonWinding_f(f);
			pc[0] = -118.0 + i - build_size;	pc[1] = 34.0 + j + build_size ;	cbs.AddPolygonPoint_f(pc,f);
			pc[0] = -118.0 + i + build_size;	pc[1] = 34.0 + j + build_size ;	cbs.AddPolygonPoint_f(pc,f);
			pc[0] = -118.0 + i + build_size;	pc[1] = 34.0 + j - build_size ;	cbs.AddPolygonPoint_f(pc,f);
			pc[0] = -118.0 + i - build_size;	pc[1] = 34.0 + j - build_size ;	cbs.AddPolygonPoint_f(pc,f);
			cbs.EndPolygon_f(f);

		}
	}

	cbs.BeginPolygon_f(1, 0xFFFF, f);
	cbs.BeginPolygonWinding_f(f);
	pc[0] = -117.50;	pc[1] = 34.25;	cbs.AddPolygonPoint_f(pc, f);
	pc[0] = -117.25;	pc[1] = 34.50;	cbs.AddPolygonPoint_f(pc, f);
	pc[0] = -117.50;	pc[1] = 34.75;	cbs.AddPolygonPoint_f(pc, f);
	pc[0] = -117.75;	pc[1] = 34.50;	cbs.AddPolygonPoint_f(pc, f);
	cbs.EndPolygonWinding_f(f);
//	cbs.BeginPolygonWinding_f(f);
//	pc[0] = -117.50;	pc[1] = 34.40;	cbs.AddPolygonPoint_f(pc, f);
//	pc[0] = -117.60;	pc[1] = 34.50;	cbs.AddPolygonPoint_f(pc, f);
//	pc[0] = -117.50;	pc[1] = 34.60;	cbs.AddPolygonPoint_f(pc, f);
//	pc[0] = -117.40;	pc[1] = 34.50;	cbs.AddPolygonPoint_f(pc, f);
//	cbs.EndPolygonWinding_f(f);
	cbs.EndPolygon_f(f);


	double	nc[3];
	nc[1] = 34.5 ; nc[0] = -117.5 ; nc[2] = 0.0;		cbs.BeginSegment_f(0,0, 5, nc, false, f);
	nc[1] = 34.75; nc[0] = -117.75; nc[2] = 0.0;		cbs.AddSegmentShapePoint_f(nc, false, f);
	nc[1] = 34.85; nc[0] = -117.85; nc[2] = 0.0;		cbs.EndSegment_f(1, nc, false, f);

	nc[1] = 34.5 ; nc[0] = -117.5 ; nc[2] = 0.0;		cbs.BeginSegment_f(0,1, 5, nc, false, f);
	nc[1] = 34.75; nc[0] = -117.25; nc[2] = 0.0;		cbs.AddSegmentShapePoint_f(nc, false, f);
	nc[1] = 34.85; nc[0] = -117.15; nc[2] = 0.0;		cbs.EndSegment_f(2, nc, false, f);

	nc[1] = 34.5 ; nc[0] = -117.5 ; nc[2] = 0.0;		cbs.BeginSegment_f(0,1, 5, nc, false, f);
	nc[1] = 34.25; nc[0] = -117.75; nc[2] = 0.0;		cbs.AddSegmentShapePoint_f(nc, false, f);
	nc[1] = 34.15; nc[0] = -117.85; nc[2] = 0.0;		cbs.EndSegment_f(3, nc, false, f);

	nc[1] = 34.5 ; nc[0] = -117.5 ; nc[2] = 0.0;		cbs.BeginSegment_f(0,0, 5, nc, false, f);
	nc[1] = 34.25; nc[0] = -117.25; nc[2] = 0.0;		cbs.AddSegmentShapePoint_f(nc, false, f);
	nc[1] = 34.15; nc[0] = -117.15; nc[2] = 0.0;		cbs.EndSegment_f(4, nc, false, f);

	DSFWriteToFile(path, f);
	DSFDestroyWriter(f);
}
#endif

//************************************************************************************************************************
// DSF TEST NUMBER 2 - DIRECT BITMAP TERRAIN
//************************************************************************************************************************
// Four corners:
// Left side - no lights, right side, lights
// Bottom - on alpha, top - alpha
#if 0
void	GenFakeDSFFile(const char * path)
{
	void * f = DSFCreateWriter(-118.0, 34.0, -117.0, 35.0, 8);
	DSFCallbacks_t	cbs;
	DSFGetWriterCallbacks(&cbs);

	cbs.AcceptProperty_f("sim/west", "-118", f);
	cbs.AcceptProperty_f("sim/east", "-117", f);
	cbs.AcceptProperty_f("sim/south", "34", f);
	cbs.AcceptProperty_f("sim/north", "35", f);
	cbs.AcceptTerrainDef_f("water", f);
	cbs.AcceptTerrainDef_f("no_alpha.png", f);
	cbs.AcceptTerrainDef_f("no_alpha_with_lights.png", f);
	cbs.AcceptTerrainDef_f("alpha.png", f);
	cbs.AcceptTerrainDef_f("alpha_with_lights.png", f);

	double	pc[5];

	//-LOWER LEFT------------------------------------------------------------------------------------------------------------------/
	cbs.BeginPatch_f(0, 0.0, -1.0, dsf_Flag_Physical, NULL, NULL, 3, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -118.0;	pc[1] = 34.0;	pc[2] = 100.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -118.0; pc[1] = 34.5;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.0;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);
	cbs.BeginPatch_f(1, 0.0, -1.0, dsf_Flag_Overlay, NULL, NULL, 5, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -118.0;	pc[1] = 34.0;	pc[2] = 100.0;	pc[3] = 0.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -118.0; pc[1] = 34.5;	pc[2] = 150.0;	pc[3] = 0.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.0;	pc[2] = 150.0;	pc[3] = 1.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	pc[3] = 1.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);

	//-LOWER RIGHT-----------------------------------------------------------------------------------------------------------------/
	cbs.BeginPatch_f(0, 0.0, -1.0, dsf_Flag_Physical, NULL, NULL, 3, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -117.5;	pc[1] = 34.0;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.0;	pc[2] = 100.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.5;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);
	cbs.BeginPatch_f(2, 0.0, -1.0, dsf_Flag_Overlay, NULL, NULL, 5, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -117.5;	pc[1] = 34.0;	pc[2] = 150.0;	pc[3] = 0.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	pc[3] = 0.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.0;	pc[2] = 100.0;	pc[3] = 1.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.5;	pc[2] = 150.0;	pc[3] = 1.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);

	//-UPPER LEFT------------------------------------------------------------------------------------------------------------------/
	cbs.BeginPatch_f(0, 0.0, -1.0, dsf_Flag_Physical, NULL, NULL, 3, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -118.0;	pc[1] = 34.5;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -118.0; pc[1] = 35.0;	pc[2] = 100.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 35.0;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);
	cbs.BeginPatch_f(3, 0.0, -1.0, dsf_Flag_Overlay, NULL, NULL, 5, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -118.0;	pc[1] = 34.5;	pc[2] = 150.0;	pc[3] = 0.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -118.0; pc[1] = 35.0;	pc[2] = 100.0;	pc[3] = 0.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 34.5;	pc[2] = 200.0;	pc[3] = 1.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 35.0;	pc[2] = 150.0;	pc[3] = 1.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);

	//-UPPER LEFT------------------------------------------------------------------------------------------------------------------/
	cbs.BeginPatch_f(0, 0.0, -1.0, dsf_Flag_Physical, NULL, NULL, 3, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -117.5;	pc[1] = 34.5;	pc[2] = 200.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 35.0;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.5;	pc[2] = 150.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 35.0;	pc[2] = 100.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);
	cbs.BeginPatch_f(4, 0.0, -1.0, dsf_Flag_Overlay, NULL, NULL, 5, f);
	cbs.BeginPrimitive_f(dsf_TriStrip, f);
	pc[0] = -117.5;	pc[1] = 34.5;	pc[2] = 200.0;	pc[3] = 0.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.5; pc[1] = 35.0;	pc[2] = 150.0;	pc[3] = 0.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 34.5;	pc[2] = 150.0;	pc[3] = 1.0;	pc[4] = 0.0;	cbs.AddPatchVertex_f(pc, f);
	pc[0] = -117.0; pc[1] = 35.0;	pc[2] = 100.0;	pc[3] = 1.0;	pc[4] = 1.0;	cbs.AddPatchVertex_f(pc, f);
	cbs.EndPrimitive_f(f);
	cbs.EndPatch_f(f);

	DSFWriteToFile(path, f);
	DSFDestroyWriter(f);
}
#endif


//************************************************************************************************************************
// DSF TEST NUMBER 2 - .TER FILES
//************************************************************************************************************************
// Left side no borders    	rigth side borders
// Left side no projection 	right side projection

int	depths[16] = {
	2,							//	S
	2,							//	SL
	4,							//	SB
	4,							//	SBL
	0,							//	SP
	0,							//	SPL
	2,							//	SPB
	2,							//	SPBL
	3,							//	C
	3,							//	CL
	5,							//	CB
	5,							//	CBL
	1,							//	CP
	1,							//	CPL
	3,							//	CPB
	3							//	CPBL
};



void	BuildOnePatch(DSFCallbacks_t * cbs, void * f, int x, int y)
{
	double	pc[10];
	float	s1 = (float) (x  ) / 4.0;
	float	s2 = (float) (x+1) / 4.0;
	float	t1 = (float) (y  ) / 4.0;
	float	t2 = (float) (y+1) / 4.0;
	s1 += -118.0;	t1 += 34.0;
	s2 += -118.0;	t2 += 34.0;

	int	layer = x + y * 4 + 1;

	pc[3] = 0.0; pc[4] = 0.0;
	cbs->BeginPatch_f(0, 0.0, -1.0, dsf_Flag_Physical, 5, f);
	cbs->BeginPrimitive_f(dsf_TriFan, f);
	pc[0] = s1;	pc[1] = t1;	pc[2] = 100.0; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s1;	pc[1] = t2;	pc[2] = 100.0; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s2;	pc[1] = t2;	pc[2] = 100.0; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s2;	pc[1] = t1;	pc[2] = 100.0; cbs->AddPatchVertex_f(pc, f);
	cbs->EndPrimitive_f(f);
	cbs->EndPatch_f(f);

	int d = depths[layer-1];
	int n = (d % 2) ? (d-1) : d;
	cbs->BeginPatch_f(layer, 0.0, -1.0, dsf_Flag_Overlay, d+5, f);
	cbs->BeginPrimitive_f(dsf_TriFan, f);
	pc[0] = s1;	pc[1] = t1;	pc[2] = 100.0; pc[5] = 0.0; pc[6] = 0.0; pc[7] = 0.0; pc[8] = 0.0; pc[n+5] = 0.0; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s1;	pc[1] = t2;	pc[2] = 100.0; pc[5] = 0.0; pc[6] = 1.0; pc[7] = 0.0; pc[8] = 1.0; pc[n+5] = 0.6; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s2;	pc[1] = t2;	pc[2] = 100.0; pc[5] = 1.0; pc[6] = 1.0; pc[7] = 1.0; pc[8] = 1.0; pc[n+5] = 1.0; cbs->AddPatchVertex_f(pc, f);
	pc[0] = s2;	pc[1] = t1;	pc[2] = 100.0; pc[5] = 1.0; pc[6] = 0.0; pc[7] = 1.0; pc[8] = 0.0; pc[n+5] = 0.2; cbs->AddPatchVertex_f(pc, f);
	cbs->EndPrimitive_f(f);
	cbs->EndPatch_f(f);
}

void	GenFakeDSFFile(const char * path)
{
	void * f = DSFCreateWriter(-118.0, 34.0, -117.0, 35.0, 8);
	DSFCallbacks_t	cbs;
	DSFGetWriterCallbacks(&cbs);

	cbs.AcceptProperty_f("sim/west", "-118", f);
	cbs.AcceptProperty_f("sim/east", "-117", f);
	cbs.AcceptProperty_f("sim/south", "34", f);
	cbs.AcceptProperty_f("sim/north", "35", f);
	cbs.AcceptTerrainDef_f("water", f);
	for (int q = 0; q < 16; ++q)
		cbs.AcceptTerrainDef_f("water", f);

//	cbs.AcceptTerrainDef_f("ter_S.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SB.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SBL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SP.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SPL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SPB.ter",f);
//	cbs.AcceptTerrainDef_f("ter_SPBL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_C.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CB.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CBL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CP.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CPL.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CPB.ter",f);
//	cbs.AcceptTerrainDef_f("ter_CPBL.ter",f);

	int n = 0;
	while (kFacs[n])
	{
		cbs.AcceptPolygonDef_f(kFacs[n], f);
		++n;
	}

	n = 0;
	while (kObjs[n])
	{
		cbs.AcceptObjectDef_f(kObjs[n], f);
		++n;
	}

	for (int y = 0; y < 4; ++y)
	for (int x = 0; x < 4; ++x)
		BuildOnePatch(&cbs, f, x, y);



	DSFWriteToFile(path, f);
	DSFDestroyWriter(f);
}

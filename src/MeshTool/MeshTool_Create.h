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

#ifndef MeshTool_Create_H
#define MeshTool_Create_H

struct	DEMGeo;

typedef	void (* MT_Error_f)(const char * fmt,va_list args);

void MT_StartCreate(const char * xes_path, const DEMGeo& in_dem, MT_Error_f err_handler);
void MT_FinishCreate(void);
void MT_MakeDSF(const char * dump_dir, const char * file_name);
void MT_Cleanup(void);

int MT_CreateCustomTerrain(
					const char * terrain_name,
					double		proj_lon[4],
					double		proj_lat[4],
					double		proj_s[4],
					double		proj_t[4],
					int			back_with_water);
					
void MT_LimitZ(int limit);
					
void MT_LayerStart(int in_terrain_type);
void MT_LayerEnd(void);
void MT_LayerShapefile(const char * fi, const char * in_terrain_type);

void MT_PolygonStart(void);
void MT_PolygonPoint(double lon, double lat);
bool MT_PolygonEnd(void);						// Returns false if z-limit rejected the polygon.

void MT_HoleStart(void);
void MT_HolePoint(double lon, double lat);
void MT_HoleEnd(void);

void MT_NetStart(const char * road_type);
void MT_NetSegment(double lon1, double lat1, double lon2, double lat2);
void MT_NetEnd(void);



#endif /* MeshTool_Create_H */

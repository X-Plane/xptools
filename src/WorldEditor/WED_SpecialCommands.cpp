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
#include "WED_SpecialCommands.h"
#include "BitmapUtils.h"
#include <gl.h>
#include "XPLMMenus.h"
#include "PlatformUtils.h"
#include "SceneryPackages.h"
#include "WED_Document.h"
#include "WED_Assert.h"
#include <ShapeFil.h>
#include "MapAlgs.h"
#include "ObjTables.h"
#include "NetTables.h"
#include "DEMTables.h"
#include "WED_Selection.h"
#include "WED_Progress.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "WED_Notify.h"
#include "WED_Msgs.h"
#include "PlatformUtils.h"
#include "WED_Globals.h"
#include "WED_SpreadsheetWizard.h"

#define	kMaxDegChangePerSample	0.125

enum {
	specCmd_Screenshot,
	specCmd_CreateTerrainPackage,
	specCmd_UpdateTerrainPackage,
	specCmd_Div1,
	specCmd_Wizard,
	specCmd_CountBorders,
	specCmd_ClimateRange,
	specCmd_ReloadConfigFiles,
	specCmd_TempMSL,
	specCmd_FixMSL,
	specCmd_FixRain,
	specCmd_SplatClimate,
	specCmd_Div2,
	specCmd_FaceHeight,
	specCmd_ObjHeight,
	specCmd_MeshErr,
	specCmd_PreviewSHP,
	specCmd_KillObjs,
	specCmd_CheckEnums,
	specCmd_Count
};

const char *	kSpecCmdNames [] = {
	"Take Screenshot...",
	"Create Terrain Package...",
	"Update Terrain Package...",
	"-",
	"Spreadsheet Wizard...",
	"Count Mesh and Border Triangles...",
	"Show Climate Ranges...",
	"Reload Configuration Files",
	"Create Sea Level Temperatures...",
	"Filter Sea Level Temperatures...",
	"Filter Rain Fall...",
	"Spread Climate Data...",
	"-",
	"Show Height of Selected Faces...",
	"Show Height of Objs in Selected Faces...",
	"Measure Error in Triangulation...",
	"Preview Shape File",
	"Kill Features Without Heights in Selected Faces",
	"Self-Check Enums",
	0
};

static	const char	kCmdKeys [] = {
	'.',	xplm_ControlFlag,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
//	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0
};

static	XPLMMenuID	sSpecMenu = NULL;

static	void	WED_HandleSpecMenuCmd(void *, void * i);
static	void 	DoScreenshot(void);

struct FeatureHasNoHeight {
	bool operator()(const GISPointFeature_t& x) const {
		return x.mParams.find(pf_Height) == x.mParams.end();
	}
};


void	RegisterSpecialCommands(void)
{
	int n;
	sSpecMenu = XPLMCreateMenu("Package", NULL, 0, WED_HandleSpecMenuCmd, NULL);
	n = 0;
	while (kSpecCmdNames[n])
	{
		XPLMAppendMenuItem(sSpecMenu, kSpecCmdNames[n], (void *) n, 1);
		if (kCmdKeys[n*2])
			XPLMSetMenuItemKey(sSpecMenu,n,kCmdKeys[n*2],kCmdKeys[n*2+1]);
		++n;
	}
}


//static hash_map<int, int> 	gTransTable;

#if 0
static bool HandleTranslate(const vector<string>& inTokenLine, void * inRef)
{
	int e1, e2;
	if (TokenizeLine(inTokenLine, " ee", &e1, &e2) == 3)
		gTransTable[e1] = e2;
	else
		return false;
	return true;
}
#endif


static	void	WED_HandleSpecMenuCmd(void *, void * i)
{
	try {
		int cmd = (int) i;
		switch(cmd) {
		case specCmd_Screenshot:
			{
				DoScreenshot();
			}
			break;

		case specCmd_CreateTerrainPackage:
		case specCmd_UpdateTerrainPackage:
			{
				char	buf[1024];
				strcpy(buf, "New Scenery Package");
				if (!GetFilePathFromUser((cmd == specCmd_UpdateTerrainPackage) ? getFile_PickFolder : getFile_Save, 
										(cmd == specCmd_UpdateTerrainPackage) ? "Please pick your scenery package" : "Please name your scenery package", 
										(cmd == specCmd_UpdateTerrainPackage) ? "Update" : "Create", 5, buf)) return;
				if (cmd != specCmd_UpdateTerrainPackage) strcat(buf, DIR_STR);
				CreateTerrainPackage(buf, true);
			}
			break;
		case specCmd_ObjHeight:
			{
				map<int, map<float, int> >	histo;
				for (set<GISFace *>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
				for (GISPointFeatureVector::iterator p = (*f)->mPointFeatures.begin(); p != (*f)->mPointFeatures.end(); ++p)
				if (p->mParams.count(pf_Height))
				{
					int h = p->mParams[pf_Height];
					h /= 10;
					h *= 10;
					histo[p->mFeatType][h]++;
				}
				for (map<int, map<float, int> >::iterator feat = histo.begin(); feat != histo.end(); ++feat)
				{
					printf("Feature: %s\n", FetchTokenString(feat->first));
					for (map<float, int>::iterator h = feat->second.begin(); h != feat->second.end(); ++h)
						printf("   h=%5lf count = %d\n", h->first, h->second);
				}
			}
			break;
		case specCmd_CheckEnums:
			{
				EnumSystemSelfCheck();
			}
			break;
		case specCmd_FaceHeight:
			{
				int n = 0;
				char buf[1024];
				map<float, int>	hist;
				map<float, int>::iterator iter;
				for (set<Pmwx::Face_handle>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
				if (!(*f)->is_unbounded())
				{
					n += GetParamHistogram(*f, gDocument->gDem[dem_Elevation], hist);
				}
			
				if (n == 0) 
				{
					sprintf(buf, "Lake contains no points.");
				} 
				else if (n == 1) 
				{
					sprintf(buf, "Lake contains one point, h = %f", hist.begin()->first);
				} 
				else 
				{
					// BASICS - mean, min, max, mode
					float minv = hist.begin()->first;
					float maxv = hist.begin()->first;
					float mean = 0.0;
					float mode = NO_DATA;
					int mode_count = 0;
					for (iter = hist.begin(); iter != hist.end(); ++iter)
					{
						if (iter->second > mode_count)
						{
							mode = iter->first;
							mode_count = iter->second;
						}
						mean += (iter->first * (float) iter->second);
						minv = min(minv, iter->first);
						maxv = max(maxv, iter->first);
					}
					mean /= float (n);
					
					// STANDARD DEVIATION
					float	devsq = 0.0;
					for (iter = hist.begin(); iter != hist.end(); ++iter)
					{
						devsq += ( ((float) iter->second) * (iter->first - mean) * (iter->first - mean) );
					}
					devsq /= (float) (n-1);
					devsq = sqrt(devsq);
					
					// CONFIDENCE LEVEL
					multimap<int, float, greater<int> >	reverser;
					for (iter = hist.begin(); iter != hist.end(); ++iter)
						reverser.insert(multimap<int,float>::value_type(iter->second, iter->first));
					
					float minv_lim = 9.9e9;
					float maxv_lim =-9.9e9;
					float sum = 0.0;
					int ctr = 0;
					int needed = 0.7 * (float) n;
					for (multimap<int, float, greater<int> >::iterator i = reverser.begin(); i != reverser.end(); ++i)
					{
						minv_lim = min(minv_lim, i->second);
						maxv_lim = max(maxv_lim, i->second);
						sum += (i->second * (float) i->first);
						ctr += i->first;
						if (ctr > needed)
							break;
					}
					sum /= (float) ctr;

					sprintf(buf, "mean=%f min=%f max=%f mode=%f (%d) std dev = %f, 70%% range = [%f..%f], rangemean=%f", mean, minv, maxv, mode, mode_count, devsq, minv_lim, maxv_lim, sum);
				}
				DoUserAlert(buf);
//				for (iter = hist.begin(); iter != hist.end(); ++iter)
//				{
//					printf("   %10f %d %f\n", iter->first, iter->second, (float) iter->second / (float) n);
//				}
				
			}
			break;
		case specCmd_MeshErr:
			{
				char buf[1024];
				map<float, int>	hist;
				map<float, int>::iterator iter;
				int n = CalcMeshError(gDocument->gTriangulationHi, gDocument->gDem[dem_Elevation], hist, WED_ProgressFunc);

				float minv = hist.begin()->first;
				float maxv = hist.begin()->first;
				float mean = 0.0;
				for (iter = hist.begin(); iter != hist.end(); ++iter)
				{
					mean += (iter->first * (float) iter->second);
					minv = min(minv, iter->first);
					maxv = max(maxv, iter->first);
				}
				mean /= float (n);
				
				float	devsq = 0.0;
				for (iter = hist.begin(); iter != hist.end(); ++iter)
				{
					devsq += ( ((float) iter->second) * (iter->first - mean) * (iter->first - mean) );
				}
				devsq /= (float) (n-1);
				devsq = sqrt(devsq);
				
				sprintf(buf, "mean=%f min=%f max=%f std dev = %f", mean, minv, maxv, devsq);
				DoUserAlert(buf);				
			}
			break;
		case specCmd_PreviewSHP:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a shape file", "Preview", 16, buf)) return;
				SHPHandle file = SHPOpen(buf, "rb");
				if (file == NULL)
					return;
				
				int	entityCount, shapeType;
				double	bounds_lo[4], bounds_hi[4];
				
				gMeshPoints.clear();
				gMeshLines.clear();
				SHPGetInfo(file, &entityCount, &shapeType, bounds_lo, bounds_hi);
				
				for (int n = 0; n < entityCount; ++n)
				{
					SHPObject * obj = SHPReadObject(file, n);

					if (obj->nSHPType == SHPT_POLYGONZ || obj->nSHPType == SHPT_POLYGON || obj->nSHPType == SHPT_POLYGONM)
					{
						for (int part = 0; part < obj->nParts; ++part)
						{
							Polygon2 pts;
							int start_idx = obj->panPartStart[part];
							int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
							for (int index = start_idx; index < stop_idx; ++index)
							{
								if (part == 0)
									pts.insert(pts.begin(), Point2(obj->padfX[index],obj->padfY[index]));
								else
									pts.insert(pts.end(), Point2(obj->padfX[index],obj->padfY[index]));
							}
							for (int m = 0; m < pts.size(); ++m)
							{
								gMeshLines.push_back(pair<Point2,Point3>(pts.side(m).p1, Point3(0.8, 0.3, 0.1)));
								gMeshLines.push_back(pair<Point2,Point3>(pts.side(m).p2, Point3(0.8, 0.3, 0.1)));
							}
						}
					}					
					SHPDestroyObject(obj);	
				}	
				SHPClose(file);
			}
			break;
		case specCmd_KillObjs:
			{
				for (set<GISFace*>::iterator face = gFaceSelection.begin(); face != gFaceSelection.end(); ++face)
				{
					(*face)->mPointFeatures.erase(remove_if((*face)->mPointFeatures.begin(), (*face)->mPointFeatures.end(), FeatureHasNoHeight()), (*face)->mPointFeatures.end());
				}				
			}
			break;
		case specCmd_TempMSL:
			{
				gDocument->gDem[dem_TemperatureSeaLevel] = gDocument->gDem[dem_Temperature];
				DEMGeo& temp(gDocument->gDem[dem_TemperatureSeaLevel]);
				const DEMGeo& elev(gDocument->gDem[dem_Elevation]);
				
				if (elev.mWidth == temp.mWidth && elev.mHeight == temp.mHeight)
				{
					for (int y = 0; y < temp.mHeight; ++y)
					for (int x = 0; x < temp.mWidth ; ++x)
					{
						float e = elev(x,y);
						if (temp(x,y) != NO_DATA && e != NO_DATA)
							temp(x,y) -= e * kStdLapseRate;
					}
				} else {
				
					for (int y = 0; y < temp.mHeight; ++y)
					for (int x = 0; x < temp.mWidth ; ++x)
					{
						float e = elev.value_linear(temp.x_to_lon(x), temp.y_to_lat(y));
						if (temp(x,y) != NO_DATA && e != NO_DATA)
							temp(x,y) -= e * kStdLapseRate;
					}
				}
				WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			}
			break;
		case specCmd_FixMSL:
			{
				DEMGeo& msl(gDocument->gDem[dem_TemperatureSeaLevel]);
				
				float k[25];

				CalculateFilter(5, k, demFilter_Spread, false);
				DEMGeo	msl2(msl);
				msl.filter_self_normalize(5, k);
				for (int y = 0; y < msl.mHeight; ++y)
				for (int x = 0; x < msl.mWidth ; ++x)
				if (msl2.get(x,y) == NO_DATA)
					msl(x,y) = NO_DATA;				
			}
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;
		case specCmd_FixRain:
			{
				DEMGeo& rain(gDocument->gDem[dem_Rainfall]);
				
				float k[25];
				//Sergio sez: not too much rain smoothing - for reasons that only the master can understand! ;-)
				CalculateFilter(3, k, demFilter_Spread, false);
				DEMGeo	rain2(rain);
				rain.filter_self_normalize(3, k);
				for (int y = 0; y < rain.mHeight; ++y)
				for (int x = 0; x < rain.mWidth ; ++x)
				if (rain2.get(x,y) == NO_DATA)
					rain(x,y) = NO_DATA;				
			}
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;
		case specCmd_SplatClimate:
			{
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.0);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_Temperature	  	 ])) { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.15);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_TemperatureRange	 ]))	 { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.3);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_Rainfall			 ])) { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.45);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_Biomass			 ])) { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.6);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_TemperatureSeaLevel ])) { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 0.75);
				while (SpreadDEMValuesIterate(gDocument->gDem[dem_Climate			 ])) { }
				WED_ProgressFunc(0, 1, "Spreading climate.", 1.0);

				WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			}
			break;
		case specCmd_ReloadConfigFiles:
			WED_ProgressFunc(0, 1, "Reloading config files...", 0.0);
			LoadDEMTables();
			LoadObjTables();
			LoadNetFeatureTables();
			WED_ProgressFunc(0, 1, "Reloading config files...", 1.0);
			CheckObjTable();
			break;
		case specCmd_Wizard:
			WED_ShowSpreadsheetWizard();
			break;
		case specCmd_CountBorders:
			{
				int b = 0, c = 0;
				for (CDT::Face_iterator f = gDocument->gTriangulationHi.finite_faces_begin(); f != gDocument->gTriangulationHi.finite_faces_end(); ++f)
				{
					c++;
					b += f->info().terrain_border.size();
				}
				
				if (c > 0)
				{					
					char buf[512];
					int t= b + c;
					sprintf(buf, "Mesh contains %d base triangles, %d border triangles, %d total triangles.  Triangle-to-mesh ratio: %f",
						c, b, t, (float) t / (float) c);
					DoUserAlert(buf);
				} else 
					DoUserAlert("The mesh has not yet been built.");
			}
			break;
		case specCmd_ClimateRange:
			{
				DEMGeo& temp(gDocument->gDem[dem_Temperature]);
				DEMGeo& temps(gDocument->gDem[dem_TemperatureSeaLevel]);
				DEMGeo& rain(gDocument->gDem[dem_Rainfall]);
				DEMGeo& elev(gDocument->gDem[dem_Elevation]);
				DEMGeo& lu(gDocument->gDem[dem_LandUse]);
				DEMGeo&	old_lu(gDocument->gDem[dem_OrigLandUse]);
				DEMGeo	rain_diff, temp_diff;
				DEMMakeDifferential(temps, temp_diff);
				DEMMakeDifferential(rain, rain_diff);
				
				set<int>	lus, olus;
				float tmin, tmax, rmin, rmax, emin, emax, tsmin, tsmax;
				int x, y;
				tmin = tmax = temp.get(0,0);
				tsmin = tsmax = temps.get(0,0);
				rmin = rmax = rain.get(0,0);
				emin = emax = elev.get(0,0);
				
				float rain_max_dif = 0.0, temp_max_dif = 0.0;
				
				for (y = 0; y < temp.mHeight; ++y)
				for (x = 0; x < temp.mWidth; ++x)
				{
					tmin = MIN_NODATA(tmin, temp.get(x,y));
					tmax = MAX_NODATA(tmax, temp.get(x,y));
				}

				for (y = 0; y < temps.mHeight; ++y)
				for (x = 0; x < temps.mWidth; ++x)
				{
					tsmin = MIN_NODATA(tsmin, temps.get(x,y));
					tsmax = MAX_NODATA(tsmax, temps.get(x,y));
					
					temp_max_dif = MAX_NODATA(temp_max_dif, temp_diff.get(x,y));
				}

				for (y = 0; y < rain.mHeight; ++y)
				for (x = 0; x < rain.mWidth; ++x)
				{
					rmin = MIN_NODATA(rmin, rain.get(x,y));
					rmax = MAX_NODATA(rmax, rain.get(x,y));

					rain_max_dif = MAX_NODATA(rain_max_dif, rain_diff.get(x,y));
				}
				
				for (y = 0; y < elev.mHeight; ++y)
				for (x = 0; x < elev.mWidth; ++x)
				{
					emin = MIN_NODATA(emin, elev.get(x,y));
					emax = MAX_NODATA(emax, elev.get(x,y));
				}

				for (y = 0; y < lu.mHeight; ++y)
				for (x = 0; x < lu.mWidth; ++x)
					lus.insert(lu.get(x,y));

				for (y = 0; y < old_lu.mHeight; ++y)
				for (x = 0; x < old_lu.mWidth; ++x)
					olus.insert(old_lu.get(x,y));
		
				char buf[1024];
				sprintf(buf,"Temp: %.1fC..%.1fC SeaLevelTemp: %.1fC..%.1fC Rain: %.1fmm..%.1fmm Elevation: %.1fm..%.1fm.  %d old landuses, %d new landuses.  Max temp change = %f, Max rain dif = %f",
						tmin, tmax, tsmin,tsmax, rmin, rmax, emin, emax, olus.size(), lus.size(), temp_max_dif, rain_max_dif);
				DoUserAlert(buf);
				set<int>::iterator iter;
				printf("--------OLD LANDUSES---------\n");
				for (iter = olus.begin(); iter != olus.end(); ++iter)
					printf("%4d %s\n", *iter, FetchTokenString(*iter));
				printf("--------NEW LANDUSES---------\n");
				for (iter = lus.begin(); iter != lus.end(); ++iter)
					printf("%4d %s\n", *iter, FetchTokenString(*iter));
				rain.swap(rain_diff);
				temp.swap(temp_diff);
			}
			break;
		}
	} catch (wed_assert_fail_exception& e) {
	} catch (exception& e) {
		DoUserAlert(e.what());
	} catch (...) {
		DoUserAlert("The operation was aborted due to an unexpected internal error.");
	}
	
}

void DoScreenshot(void)
{
	static	int	rev = 1;
	
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	ImageInfo	cap;
	int err = CreateNewBitmap(viewport[2], viewport[3], 3, &cap);
	if (err == 0)
	{	
#if APL	
		glReadPixels(0, 0, viewport[2], viewport[3], GL_BGR, GL_UNSIGNED_BYTE, cap.data);
#else
		glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, cap.data);
#endif		
		char	buf[1024];
		do {
			strcpy(buf, GetApplicationPath());
			char * p = buf + strlen(buf);
			while (p > buf && *(p-1) != DIR_CHAR)	--p;
			sprintf(p, "screenshot %d.png", rev);
			FILE * fi = fopen(buf, "rb");
			if (fi)
			{
				fclose(fi);
				++rev;
			} else
				break;
		} while (1);
		++rev;		
		WriteBitmapToPNG(&cap, buf, NULL, 0);
		DestroyBitmap(&cap);
	}	
}

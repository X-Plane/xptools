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
#include "RF_SpecialCommands.h"
#include "BitmapUtils.h"
#include "GISTool_Globals.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

#include "GUI_Application.h"
#include "PlatformUtils.h"
#include "SceneryPackages.h"
#include "RF_Assert.h"
#include <ShapeFil.h>
#include "MapAlgs.h"
#include "ObjTables.h"
#include "NetTables.h"
#include "DEMTables.h"
#include "RF_Selection.h"
#include "RF_Progress.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "PlatformUtils.h"
#include "RF_Globals.h"
//#include "RF_SpreadsheetWizard.h"

#define	kMaxDegChangePerSample	0.125



GUI_MenuItem_t	kSpecialItems[] = {
{	"Take Screenshot...",										'.',	gui_ControlFlag,	0,		specCmd_Screenshot				},
{	"Create Terrain Package...",								0,		0,					0,		specCmd_CreateTerrainPackage	},
{	"Update Terrain Package...",								0,		0,					0,		specCmd_UpdateTerrainPackage	},
{	"-",														0,		0,					0,		0								},
//{	"Spreadsheet Wizard...",									0,		0,					0,		specCmd_Wizard					},
{	"Count Mesh and Border Triangles...",						0,		0,					0,		specCmd_CountBorders			},
{	"Show Climate Ranges...",									0,		0,					0,		specCmd_ClimateRange			},
{	"Reload Configuration Files",								0,		0,					0,		specCmd_ReloadConfigFiles		},
{	"Create Sea Level Temperatures...",							0,		0,					0,		specCmd_TempMSL					},
{	"Filter Sea Level Temperatures...",							0,		0,					0,		specCmd_FixMSL					},
{	"Filter Rain Fall...",										0,		0,					0,		specCmd_FixRain					},
{	"Spread Climate Data...",									0,		0,					0,		specCmd_SplatClimate			},
{	"-",														0,		0,					0,		0								},
{	"Show Height of Selected Faces...",							0,		0,					0,		specCmd_FaceHeight				},
{	"Show Height of Objs in Selected Faces...",					0,		0,					0,		specCmd_ObjHeight				},
{	"Measure Error in Triangulation...",						0,		0,					0,		specCmd_MeshErr					},
{	"Print Terrain Histogram...",								0,		0,					0,		specCmd_MeshLU					},
{	"Preview Shape File",										0,		0,					0,		specCmd_PreviewSHP				},
{	"Kill Features Without Heights in Selected Faces",			0,		0,					0,		specCmd_KillObjs				},
{	"Self-Check Enums",											0,		0,					0,		specCmd_CheckEnums				},
{	0,															0,		0,					0,		0 } };



//static	XPLMMenuID	sSpecMenu = NULL;

static	void	RF_HandleSpecMenuCmd(void *, void * i);
static	void 	DoScreenshot(void);

struct FeatureHasNoHeight {
	bool operator()(const GISPointFeature_t& x) const {
		return x.mParams.find(pf_Height) == x.mParams.end();
	}
};


void	RegisterSpecialCommands(void)
{
	GUI_Menu special_menu = gApplication->CreateMenu("Special", kSpecialItems,gApplication->GetMenuBar(), 0);
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


void	HandleSpecialCommand(int cmd)
{
	try {
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
										(cmd == specCmd_UpdateTerrainPackage) ? "Update" : "Create", 5, buf, sizeof(buf))) return;
//				if (cmd != specCmd_UpdateTerrainPackage)
					strcat(buf, DIR_STR);
				CreateTerrainPackage(buf, true);
			}
			break;
		case specCmd_ObjHeight:
			{
				map<int, map<float, int> >	histo;
				for (set<Face_handle>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
				for (GISPointFeatureVector::iterator p = (*f)->data().mPointFeatures.begin(); p != (*f)->data().mPointFeatures.end(); ++p)
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
					n += GetParamHistogram(*f, gDem[dem_Elevation], hist);
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
					float mode = DEM_NO_DATA;
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
				map<float, int>::iterator iter;
				float minv, maxv, mean, devsq;
				int n = CalcMeshError(gTriangulationHi, gDem[dem_Elevation], minv, maxv, mean, devsq, RF_ProgressFunc);

				sprintf(buf, "mean=%f min=%f max=%f std dev = %f", mean, minv, maxv, devsq);
				DoUserAlert(buf);
			}
			break;
		case specCmd_MeshLU:
			{
				map<int, int> lus;
				int t = CalcMeshTextures(gTriangulationHi,lus);
				multimap<int,int> sorted;
				for(map<int,int>::iterator l = lus.begin(); l != lus.end(); ++l)
					sorted.insert(multimap<int,int>::value_type(l->second,l->first));
				for(multimap<int,int>::iterator s = sorted.begin(); s != sorted.end(); ++s)
					printf("%f (%d): %s\n", (float) s->first / (float) t, s->first, FetchTokenString(s->second));
			}
			break;
		case specCmd_PreviewSHP:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a shape file", "Preview", 16, buf, sizeof(buf))) return;
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
				for (set<Face_handle>::iterator face = gFaceSelection.begin(); face != gFaceSelection.end(); ++face)
				{
					(*face)->data().mPointFeatures.erase(remove_if((*face)->data().mPointFeatures.begin(), (*face)->data().mPointFeatures.end(), FeatureHasNoHeight()), (*face)->data().mPointFeatures.end());
				}
			}
			break;
		case specCmd_TempMSL:
			{
				gDem[dem_TemperatureSeaLevel] = gDem[dem_Temperature];
				DEMGeo& temp(gDem[dem_TemperatureSeaLevel]);
				const DEMGeo& elev(gDem[dem_Elevation]);

				if (elev.mWidth == temp.mWidth && elev.mHeight == temp.mHeight)
				{
					for (int y = 0; y < temp.mHeight; ++y)
					for (int x = 0; x < temp.mWidth ; ++x)
					{
						float e = elev(x,y);
						if (temp(x,y) != DEM_NO_DATA && e != DEM_NO_DATA)
							temp(x,y) -= e * kStdLapseRate;
					}
				} else {

					for (int y = 0; y < temp.mHeight; ++y)
					for (int x = 0; x < temp.mWidth ; ++x)
					{
						float e = elev.value_linear(temp.x_to_lon(x), temp.y_to_lat(y));
						if (temp(x,y) != DEM_NO_DATA && e != DEM_NO_DATA)
							temp(x,y) -= e * kStdLapseRate;
					}
				}
				RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			}
			break;
		case specCmd_FixMSL:
			{
				DEMGeo& msl(gDem[dem_TemperatureSeaLevel]);

				float k[25];

				CalculateFilter(5, k, demFilter_Spread, false);
				DEMGeo	msl2(msl);
				msl.filter_self_normalize(5, k);
				for (int y = 0; y < msl.mHeight; ++y)
				for (int x = 0; x < msl.mWidth ; ++x)
				if (msl2.get(x,y) == DEM_NO_DATA)
					msl(x,y) = DEM_NO_DATA;
			}
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case specCmd_FixRain:
			{
				DEMGeo& rain(gDem[dem_Rainfall]);

				float k[25];
				//Sergio sez: not too much rain smoothing - for reasons that only the master can understand! ;-)
				CalculateFilter(3, k, demFilter_Spread, false);
				DEMGeo	rain2(rain);
				rain.filter_self_normalize(3, k);
				for (int y = 0; y < rain.mHeight; ++y)
				for (int x = 0; x < rain.mWidth ; ++x)
				if (rain2.get(x,y) == DEM_NO_DATA)
					rain(x,y) = DEM_NO_DATA;
			}
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case specCmd_SplatClimate:
			{
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.0);
				while (SpreadDEMValuesIterate(gDem[dem_Temperature	  	 ])) { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.15);
				while (SpreadDEMValuesIterate(gDem[dem_TemperatureRange	 ]))	 { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.3);
				while (SpreadDEMValuesIterate(gDem[dem_Rainfall			 ])) { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.45);
				while (SpreadDEMValuesIterate(gDem[dem_Biomass			 ])) { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.6);
				while (SpreadDEMValuesIterate(gDem[dem_TemperatureSeaLevel ])) { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 0.75);
				while (SpreadDEMValuesIterate(gDem[dem_Climate			 ])) { }
				RF_ProgressFunc(0, 1, "Spreading climate.", 1.0);

				RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			}
			break;
		case specCmd_ReloadConfigFiles:
			RF_ProgressFunc(0, 1, "Reloading config files...", 0.0);
			LoadDEMTables();
			LoadObjTables();
			LoadNetFeatureTables();
			RF_ProgressFunc(0, 1, "Reloading config files...", 1.0);
			CheckObjTable();
			break;
		case specCmd_Wizard:
//			RF_ShowSpreadsheetWizard();
			break;
		case specCmd_CountBorders:
			{
				int b = 0, c = 0;
				for (CDT::Face_iterator f = gTriangulationHi.finite_faces_begin(); f != gTriangulationHi.finite_faces_end(); ++f)
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
				DEMGeo& temp(gDem[dem_Temperature]);
				DEMGeo& temps(gDem[dem_TemperatureSeaLevel]);
				DEMGeo& rain(gDem[dem_Rainfall]);
				DEMGeo& elev(gDem[dem_Elevation]);
				DEMGeo& lu(gDem[dem_LandUse]);
//				DEMGeo&	old_lu(gDem[dem_OrigLandUse]);
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

//				for (y = 0; y < old_lu.mHeight; ++y)
//				for (x = 0; x < old_lu.mWidth; ++x)
//					olus.insert(old_lu.get(x,y));

				char buf[1024];
				sprintf(buf,"Temp: %.1fC..%.1fC SeaLevelTemp: %.1fC..%.1fC Rain: %.1fmm..%.1fmm Elevation: %.1fm..%.1fm.  %d new landuses.  Max temp change = %f, Max rain dif = %f",
						tmin, tmax, tsmin,tsmax, rmin, rmax, emin, emax, lus.size(), temp_max_dif, rain_max_dif);
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
	} catch (rf_assert_fail_exception& e) {
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
//			strcpy(buf, GetApplicationPath());
//			char * p = buf + strlen(buf);
//			while (p > buf && *(p-1) != DIR_CHAR)	--p;
//			sprintf(p, "screenshot %d.png", rev);
			sprintf(buf, "screenshot %d.png", rev);
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

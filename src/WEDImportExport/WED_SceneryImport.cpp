/*
 * Copyright (c) 2024, Laminar Research.
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
 */

#include "WED_SceneryImport.h"

#include "CompGeomDefs2.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "WED_UIDefs.h"
#include "WED_ToolUtils.h"

#include "WED_AptIE.h"
#include "AptIO.h"
#include "WED_DSFImport.h"
#include "WED_MetadataUpdate.h"

#include "WED_Airport.h"
#include "WED_Group.h"
#include "WED_Thing.h"

#include <sstream>
#include <iostream>

bool WED_SceneryImport(string scn_path, WED_Thing* wrl, bool limited)
{
    vector<WED_Airport*> apts;

    WED_ImportOneAptFile(scn_path + DIR_STR + "apt.dat", wrl, &apts);

    if (apts.size() == 0 || (limited && apts.size() > 10))   // prevent folks from opening the global airports ...
        return 0;

    set<string> dsf = FILE_find_dsfs(scn_path);
    if (limited && dsf.size() > 10)                    // prevent really likey too big sceneries from getting auto-opened ...
        return 0;

    for (auto& a : apts)
    {
        string id;
        a->GetICAO(id);
        for (int i = 0; i < id.length(); i++)
            id[i] = toupper(id[i]);
        for (auto& d : dsf)
            DSF_Import_Partial(d.c_str(), a, id);
    }
    for (auto& d : dsf)
        DSF_Import_Partial(d.c_str(), wrl, "");

    return 1;
}

void WED_DoImportScenery(IResolver * resolver)
{
	char tmp[200];
	bool success = GetFilePathFromUser(getFile_PickFolder, "Import all scenery files from ...", "Import", FILE_DIALOG_IMPORT_DSF, tmp, sizeof(tmp));
    string dir(tmp);

	if (dir.find("Earth nav data") != dir.size() - sizeof("Earth nav data"))
	{
        dir += DIR_STR "Earth nav data";
        if (!FILE_exists(dir.c_str()))
            success = false;
	}

	if(success)
    {
        WED_Thing * wrl = WED_GetWorld(resolver);
        wrl->StartOperation("Import Scenery");
        if (WED_SceneryImport(dir, wrl, false))
            wrl->CommitOperation();
        else
            wrl->AbortOperation();
    }
}

#if GATEWAY_IMPORT_FEATURES

static Point2 apt_centroid(const AptInfo_t& apt)
{
    Vector2 loc;
    for( auto& r : apt.runways)
        loc += Vector2(r.ends.midpoint());
    for (auto& h : apt.helipads)
        loc += Vector2(h.location);
    for (auto& s : apt.sealanes)
        loc += Vector2(s.ends.midpoint());

    int cnt = apt.runways.size() + apt.helipads.size() + apt.sealanes.size();
    if (cnt)
        loc /= cnt;

    return Point2(0,0) + loc;
}

void	WED_DoImportExtracts(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);

	char dir_path[200];
	bool success = GetFilePathFromUser(getFile_PickFolder, "Import all files in directory...", "Import", FILE_DIALOG_IMPORT_DSF, dir_path, sizeof(dir_path));
	const string dir = string(dir_path) + '/';
	if(success)
	{
		wrl->StartOperation("Import DSF");

		vector<string> all_files;
		FILE_get_directory(dir, &all_files, NULL);

		unordered_map<string, pair<int, string> > scn_ids;
		if(find(all_files.begin(), all_files.end(), "scenery_ids.txt") != all_files.end())
			if (auto fi = fopen((dir + "scenery_ids.txt").c_str(), "r"))
			{
				char buf[128];
				while(fgets(buf, sizeof(buf), fi))
				{
					stringstream in(buf);
					string tok[3];
					for (int i = 0; i < 3; i++)
						getline(in, tok[i], ';');
					tok[2].erase(0, 1);
					tok[2].pop_back();
					scn_ids[tok[0]] = make_pair(atoi(tok[1].c_str()), tok[2]);
				}
				LOG_MSG("Got list of %d scenery ids\n", (int) scn_ids.size());
				fclose(fi);
			}

        // Why adding an extra layer of hierachy ?
        // At exoport (and many draw or select operations) the full archive is traversed hirechically and culled 
        // by geograhic bounds.With a flat hierachy, all ~40,000 airport are tested sequentially.
        // Exporting the global airports tiles causes repeated testing per DSF tile, 360*160*40,000 = 2G tests.
        // This consumed originally some 80+ %of he total 25 cpu-min export time. Dividing the archive into 
        // just a dozen moderately well balanced buckets cuts export time by 5x to under 5 min.
        //
        // The areas are, for now, not split by longitude. This is rather archieved by exporting the
        // whole world in 2 separate and concurrent WED sessions for the western vs eastern hemisphere,
        // which happens to split the overall database very evenly w/o any geographics overlap.
			
        vector<double> lats = {-90, -40, -20, 0, 20, 30, 35, 40, 45, 50, 60};
        vector<WED_Thing*> lat_grp;

        char tmp[16];
        for(auto l : lats)
        {
            auto grp = WED_Group::CreateTyped(wrl->GetArchive());
            sprintf(tmp, "Lat%+02d", (int) l);
            grp->SetParent(wrl,0);
            grp->SetName(tmp);
            lat_grp.push_back(dynamic_cast<WED_Thing*>(grp));
        }

		int hemisphere = ConfirmMessage("Which Hemisphere to import airports for ?", "West", "All", "East");
		for(const auto& nam_apt : all_files)
		{
			if(nam_apt.compare(nam_apt.length() - 4, 4, ".dat") == 0)
			{
				AptVector		apts;
				ReadAptFile((dir + nam_apt).c_str(), apts);
				Assert(apts.size() == 1);

                string icao = apts.back().icao;
                auto lonlat = apt_centroid(apts.back());

                WED_Thing* grp = wrl;
                for(int i = lats.size()-1; i >= 0; i--)
                    if(lonlat.y() >= lats[i])
                    {
                        grp = lat_grp[i];
                        break;
                    }

                if(icao != "TEST" && icao != "LEGO")
				if(hemisphere == 0 ||
				  (hemisphere == 1 && lonlat.x() < -32.2) ||
                  (hemisphere == 2 && lonlat.x() > -31.8) )
				{
                    vector<WED_Airport*> this_apt;
                    WED_AptImport(wrl->GetArchive(), grp, nam_apt.c_str(), apts, &this_apt);

                    if (scn_ids.count(icao))
                    {
                        this_apt.front()->SetSceneryID(scn_ids[icao].first);
                        if(!scn_ids[icao].second.empty())
                            this_apt.front()->AddMetaDataKey("gw_credits", scn_ids[icao].second);
                    }

                    WED_DoInvisibleUpdateMetadata(this_apt.front());

                    for (const auto& nam_dsf : all_files)
                    {
                        if (nam_dsf.compare(nam_dsf.length() - 4, 4, ".txt") == 0 &&
                            nam_dsf.compare(0, nam_dsf.length() - 4, nam_apt, 0, nam_apt.length() - 4) == 0)
                            {
                                WED_ImportText((dir + nam_dsf).c_str(), this_apt.front());
                                break;
                            }
                    }
				}
				else if((hemisphere == 1 && lonlat.x() < -31.8) ||
                        (hemisphere == 2 && lonlat.x() > -32.2) )
                DoUserAlert((string("Airport ") + icao + " near hemisphere boundary").c_str());
			}
		}

		int tot = 0;
        for(auto l : lat_grp)
            tot += l->CountChildren();
        if(tot==0) tot=1;

        for(auto l : lat_grp)
        {
            string nam;
            l->GetName(nam);
            int cnt = l->CountChildren();
            LOG_MSG("%s %5d %5.1lf\%\n", nam.c_str(), cnt, 100.0 * cnt / tot);
        }
		wrl->CommitOperation();
		gExportTarget = wet_gateway;
	}
}

#endif /* GATEWAY_IMPORT_FEATURES */

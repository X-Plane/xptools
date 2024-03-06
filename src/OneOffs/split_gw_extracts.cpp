
// compile with:   g++ -DLIN=1 -include ../Obj/XDefs.h split_gw_extracts.cpp ../Utils/FileUtils.cpp

#include "../Utils/PlatformUtils.h"
#include "../Utils/FileUtils.h"
#include <math.h>

// kill the time consuming case-desensitizing from XDefs.h
#if LIN
#undef fopen
#endif

/* Theory of operation:
   Split up the gateway extracts to separate all airports into a couple of equal size batches. These get then
   separately im & exported in WED, so RAM requirements (about 20GB for the whole world as of 2021) are manageable.
   Or if sufficient RAM is available, processing can be done in parallel.

   Once all batches are done, all .DSF get merged into a single directory and the apt.dat concatenated (sans the 
   header and 99 rowcodes at the end), with the shell script "merge_gw_exports.sh"

   BUT

   This only works, if no airports overlap the boundaries between the geographic area for each batch.
   As that would possibly result in two batches both including the same DSF tile. And the simple shellscript
   can not merge those DSF.

   So we choose the geographic dividing lines carefully, through areas with low airport densities. And this
   code here also warns if an airport is close to the boundaries. Which itself isn't nessesarily a problem,
   but it helps in finding good dividing lines to cut batches.

   The shell script that merges the DSF does do full DSF collision checking - so if that script does NOT 
   stop with a clear error message, we did cut the world into suitable batches here.
*/

// split the world into E & W hemispheres, in the middle of the atlantic. A near perfect 50/50 split wrt. 
// data amounts. Its also a no-brainer wrt airports overlapping this boundary - there are none out there.
#define lonEW     -32.0

// this can be set to divide into even more & smaller batches
#define split4way 0

// split north vs. south america - not a good split wrt data amounts. So add AK and the west coast to SA.
#define latNA_SA   24.0
#define lonPNW   -103.0
// split europe from asia - a fair 60/40 split in terms of data amounts
#define lonEU_AS   34.0

const char* decodeLatLon(double lat, double lon, const string& fn)
{
    if(fabs(lon - lonEW) < 0.2)
      printf("Close call at %.8lf %.8lf by %s\n", lat, lon, FILE_get_file_name_wo_extensions(fn).c_str());

    if (lon < lonEW) 
    {
      #if split4way
	if(fabs(lat - latNA_SA) < 0.1 || (lat > latNA_SA && fabs(lon - lonPNW) < 0.1))
	  printf("Close call at %.8lf %.8lf by %s\n", lat, lon, FILE_get_file_name_wo_extensions(fn).c_str());
	return (lat < latNA_SA || lon < lonPNW) ? "SA"  : "NA";
      #else
	return "W";
      #endif
    }
    else
    {
	#if split4way
	  if(fabs(lon - lonEU_AS) < 0.1)
	    printf("Close call at %.8lf %.8lf by %s\n", lat, lon, FILE_get_file_name_wo_extensions(fn).c_str());
	  return lon > lonEU_AS ? "AS" : "EU";
	#else
	  return "E";
	#endif
    }
}

const char* where_in_world(const string& fn)
{
  FILE* inf = fopen(fn.c_str(), "r");
  const char* res = nullptr;
  if(inf)
  {
    char ln[100];
    fgets(ln, sizeof(ln), inf);
    fgets(ln, sizeof(ln), inf);
    while(fgets(ln, sizeof(ln), inf) && !res)
    {
      int row = 0, col = 0, a[10];
      double d[4];
      char c[20];
      sscanf(ln,"%d%n", &row, &col);
      switch(row)                        // we decide by location of a single item - fast, crude, good enough
      {
        case 100:
          if(sscanf(ln+col,"%lf%d%d%lf%d%d%d%19s%lf%lf", d, a, a+1, d+1, a+2, a+3, a+4, c, d+2, d+3) == 10)
	    res = decodeLatLon(d[2], d[3], fn);
          break;
        case 101:
          if(sscanf(ln+col,"%lf%d%19s%lf%lf", d, a, c, d+1, d+2) == 5)
	    res = decodeLatLon(d[1], d[2], fn);
          break;
        case 102:
          if(sscanf(ln+col,"%19s%lf%lf", c, d, d+1) == 3)
	    res = decodeLatLon(d[0], d[1], fn);
         break;
      }
    }
    fclose(inf);
  }
  if(!res) printf("Can't locate %s\n", FILE_get_file_name_wo_extensions(fn).c_str());
  return res;
}


int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("Usage:\n%s <from dir>\n\n"
	"Prepares gateway extracts to be imported in multiple batches or in parallel\n"
	"Moves all .dat .txt files in <from dir> to subdirectories <from_dir>"
#if split4way
	"/NA, /SA, /EU, /AS\n"
#else
	"/W, /E\n"
#endif
	"according to each airports location on earth\n", argv[0]);
   return false;
  }

  string dir = argv[1];
  if(dir.back() != DIR_CHAR) dir += DIR_STR;

#if split4way
  FILE_make_dir_exist((dir + "NA").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"NA/").c_str());
  FILE_make_dir_exist((dir + "SA").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"SA/").c_str());
  FILE_make_dir_exist((dir + "EU").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"EU/").c_str());
  FILE_make_dir_exist((dir + "AS").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"AS/").c_str());
#else
  FILE_make_dir_exist((dir + "W").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"W/").c_str());
  FILE_make_dir_exist((dir + "E").c_str());
  link((dir+"scenery_ids.txt").c_str(), (dir+"E/").c_str());
#endif

  vector<string> all_files;
  FILE_get_directory(dir, &all_files, NULL);

  for(auto& it : all_files)
    if(it.find(".dat") != string::npos)
    {
      auto w = where_in_world(dir + it);
      if(w && it != "TEST.dat" && it != "LEGO.dat")
      {
        string ndir = dir + w + DIR_STR;
        printf("Rename %s to %s\n", (dir + it).c_str(), (ndir + it).c_str());
        FILE_rename_file((dir + it).c_str(), (ndir + it).c_str());

        string dsf_nam = FILE_get_file_name_wo_extensions(it) + ".txt";
        if(FILE_exists((dir + dsf_nam).c_str()))
        {
          printf("Rename %s to %s\n",(dir + dsf_nam).c_str(), (ndir + dsf_nam).c_str());
          FILE_rename_file((dir + dsf_nam).c_str(), (ndir + dsf_nam).c_str());
        }
      }
    }
    
  return true;
}

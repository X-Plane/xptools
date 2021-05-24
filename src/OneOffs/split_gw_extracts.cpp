
// compile with:   g++ -DLIN=1 -include ../Obj/XDefs.h split_gw_extracts.cpp ../Utils/FileUtils.cpp

#include "../Utils/PlatformUtils.h"
#include "../Utils/FileUtils.h"
#include <math.h>

#if LIN
#undef fopen
#endif

#define lonW -30.0
#define lonE 60.0

char where_in_world(const string& fn)
{
  FILE* inf = fopen(fn.c_str(), "r");
  char res = 0;
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
      switch(row)
      {
        case 100:
          if(sscanf(ln+col,"%lf%d%d%lf%d%d%d%19s%lf%lf", d, a, a+1, d+1, a+2, a+3, a+4, c, d+2, d+3) == 10)
          {
            if     (d[3] < lonW) res = 'W';
            else if(d[3] > lonE) res = 'E';
            else                 res = 'C';
            if(fabs(d[3] - lonW) < 0.2 || fabs(d[3] - lonE) < 0.2)
              printf("Close call at %.8lf %.8lf by %s\n", d[2], d[3], FILE_get_file_name_wo_extensions(fn).c_str());
	  }
          break;
        case 101:
          if(sscanf(ln+col,"%lf%d%19s%lf%lf", d, a, c, d+1, d+2) == 5)
          {
            if     (d[2] < lonW) res = 'W';
            else if(d[2] > lonE) res = 'E';
            else                 res = 'C';
            if(fabs(d[2] - lonW) < 0.2 || fabs(d[2] - lonE) < 0.2)
              printf("Close call at %.8lf %.8lf by %s\n", d[1], d[2], FILE_get_file_name_wo_extensions(fn).c_str());
          }
          break;
        case 102:
          if(sscanf(ln+col,"%19s%lf%lf", c, d, d+1) == 3)
          {
            if     (d[1] < lonW) res = 'W';
            else if(d[1] > lonE) res = 'E';
            else                 res = 'C';
            if(fabs(d[1] - lonW) < 0.2 || fabs(d[1] - lonE) < 0.2)
              printf("Close call at %.8lf %.8lf by %s\n", d[0], d[1], FILE_get_file_name_wo_extensions(fn).c_str());
          }
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
    printf("%s <from dir>\n\n"
           "for preparing gateway extract to be processed as 3 separate exports\n"
           "moves  all .dat and .txt files  <from dir> into subdirectories <from_dir>/W , /C, /E\n"
           "accoring to the longitude on earth\n", argv[0]);
   return false;
  }

  string dir = argv[1];
  if(dir.back() != DIR_CHAR) dir += DIR_STR;

  FILE_make_dir_exist((dir + "W").c_str());
  FILE_make_dir_exist((dir + "C").c_str());
  FILE_make_dir_exist((dir + "E").c_str());

  vector<string> all_files;
  FILE_get_directory(dir, &all_files, NULL);

  for(auto& it : all_files)
    if(it.find(".dat") != string::npos)
    {
      char w = where_in_world(dir + it);
      if(w)
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

#include <stdio.h>

int	latlon_bucket(int p)
{
	if (p > 0) return (p / 10) * 10;
	else return ((-p + 9) / 10) * -10;
}


int main(int argc, char * argv[])
{
  int lat, lon;
  if (argc < 4)
  {
   fprintf(stderr, "Usage: genpath [folder|file] lon lat\n");
  return 1;
  }
  if (!strcmp(argv[1], "folder"))
  {
    lon = latlon_bucket(atoi(argv[2]));
    lat = latlon_bucket(atoi(argv[3]));
    printf("%+03d%+04d\n", lat, lon);
  } else if (!strcmp(argv[1], "file"))
  {
   lon = atoi(argv[2]);
    lat = atoi(argv[3]);
   printf("%+03d%+04d\n", lat, lon);
 } else {
   fprintf(stderr, "First arg must be 'folder' or 'file'\n");
  return 1;
  }
  return 0;
}

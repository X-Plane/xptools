/* 
 * Copyright (c) 2007, Laminar Research.
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

/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: 3ds2m.c,v 1.1 2001/07/18 12:17:49 jeh Exp $
 */
#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/*!
\example 3ds2m.c

Converts meshes of a <i>3DS</i> file into a m-file.

\code
Syntax: 3ds2m [options] filename [options]

Options:
  -h/--help              This help
  -o/--output filename   Write output to file instead of stdout
  -m/--mesh name         Write only specific mesh
\endcode

A m-file is a simple data format storing vertex and face information of a polygonal mesh.
It has the following structure.

\code
#
# <comment>
#
Vertex <vertex-id> <x> <y> <z>
...
Face <face-id> <vertex-1> <vertex-2> ... <vertex-n>
\endcode

\author J.E. Hoffmann <je-h@gmx.net>
*/


static void
help()
{
  fprintf(stderr,
"The 3D Studio File Format Library - 3ds2m Version " VERSION "\n"
"Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>\n"
"All rights reserved.\n"
""
"Syntax: 3ds2m [options] filename [options]\n"
"\n"
"Options:\n"
"  -h/--help              This help\n"
"  -o/--output filename   Write output to file instead of stdout\n"
"  -m/--mesh name         Write only specific mesh\n"
"\n"
);
  exit(1);
}


static const char* filename=0;
static const char* output=0;
static const char* mesh=0;


static void
parse_args(int argc, char **argv)
{
  int i;
  
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      if ((strcmp(argv[i],"-h")==0) || (strcmp(argv[i],"--help")==0)) {
        help();
      }
      else
      if ((strcmp(argv[i],"-o")==0) || (strcmp(argv[i],"--output")==0)) {
        ++i;
        if (output || (i>=argc)) {
          help();
        }
        output=argv[i];
      }
      else
      if ((strcmp(argv[i],"-m")==0) || (strcmp(argv[i],"--mesh")==0)) {
        ++i;
        if (mesh || (i>=argc)) {
          help();
        }
        mesh=argv[i];
      }
      else {
        help();
      }
    }
    else {
      if (filename) {
        help();
      }
      filename=argv[i];
    }
  }
  if (!filename) {
    help();
  }
}


static void
dump_m_file(Lib3dsFile *f, FILE *o)
{
  Lib3dsMesh *m;
  int points=0;
  int faces=0;
  unsigned i;
  Lib3dsVector pos;

  fprintf(o, "#\n");
  fprintf(o, "# Created by lib3ds2m (http://lib3ds.sourceforge.net)\n");
  fprintf(o, "#\n");
  for (m=f->meshes; m; m=m->next) {
    if (mesh) {
      if (strcmp(mesh, m->name)!=0) {
        continue;
      }
    }
    
    fprintf(o, "#\n");
    fprintf(o, "# %s vertices=%ld faces=%ld\n",
      m->name,
      m->points,
      m->faces
    );
    fprintf(o, "#\n");
    
    for (i=0; i<m->points; ++i) {
      lib3ds_vector_copy(pos, m->pointL[i].pos);
      fprintf(o, "Vertex %d %f %f %f\n", points+i, pos[0], pos[1],pos[2]);
    }

    for (i=0; i<m->faces; ++i) {
      fprintf(o, "Face %d %d %d %d\n",
        faces+i,
        points+m->faceL[i].points[0],
        points+m->faceL[i].points[1],
        points+m->faceL[i].points[2]
      );
    }

    points+=m->points;
    faces+=m->faces;
  }
}


int
main(int argc, char **argv)
{
  Lib3dsFile *f=0;

  parse_args(argc, argv);
  f=lib3ds_file_load(filename);
  if (!f) {
    fprintf(stderr, "***ERROR***\nLoading file %s failed\n", filename);
    exit(1);
  }
  if (output) {
    FILE *o=fopen(output, "w+");
    if (!o) {
      fprintf(stderr, "***ERROR***\nCan't open %s for writing\n", output);
      exit(1);
    }
    dump_m_file(f,o);
    fclose(o);
  }
  else {
    dump_m_file(f,stdout);
  }

  lib3ds_file_free(f);
  return(0);
}








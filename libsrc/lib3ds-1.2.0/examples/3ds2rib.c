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
 * $Id: 3ds2rib.c,v 1.3 2001/07/07 19:05:30 jeh Exp $
 */
#include <lib3ds/file.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/camera.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif
#include <math.h>


/*!
\example 3ds2rib.c

Simple <i>3DS</i> to RIB (RenderMan Interface Bytestream) converter.

\code
Syntax: 3ds2rib [options] filename [options]

Options:\n"
  -h/--help               This help
  -o/--output <filename>  Write output to <filename> instead of stdout
  -c/--camera <name>      Use camera <name> for rendering
  -a/--all                Render all frames
  -f/--frame #            Render frame #
  -d/--downcuts #         Render # frames of animation
\endcode

\author J.E. Hoffmann <je-h@gmx.net>
*/


static void
help()
{
  fprintf(stderr,
"The 3D Studio File Format Library - 3ds2rib Version " VERSION "\n"
"Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>\n"
"All rights reserved.\n"
"\n"
"Syntax: 3ds2rib [options] filename [options]\n"
"\n"
"Options:\n"
"  -h/--help               This help\n"
"  -o/--output <filename>  Write output to <filename> instead of stdout\n"
"  -c/--camera <name>      Use camera <name> for rendering\n"
"  -a/--all                Render all frames\n"
"  -f/--frame #            Render frame #\n"
"  -d/--downcuts #         Render # frames of animation\n"
"\n"
);
  exit(1);
}


typedef enum _Flags {
  LIB3DS2RIB_ALL  =0x0001
} Flags;

static const char* filename=0;
static const char* output=0;
static Lib3dsDword flags=0;
static float frame=0.0f;
static const char* camera=0;
static int downcuts=0;


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
      if ((strcmp(argv[i],"-a")==0) || (strcmp(argv[i],"--all")==0)) {
        flags|=LIB3DS2RIB_ALL;
      }
      else
      if ((strcmp(argv[i],"-f")==0) || (strcmp(argv[i],"--frame")==0)) {
        ++i;
        if (i>=argc) {
          help();
        }
        frame=(Lib3dsFloat)atof(argv[i]);
      }
      else
      if ((strcmp(argv[i],"-c")==0) || (strcmp(argv[i],"--camera")==0)) {
        ++i;
        if (i>=argc) {
          help();
        }
        camera=argv[i];
      }
      else
      if ((strcmp(argv[i],"-d")==0) || (strcmp(argv[i],"--downcuts")==0)) {
        ++i;
        if (i>=argc) {
          help();
        }
        downcuts=atoi(argv[i]);
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
rib_concat_transform(FILE *o, Lib3dsMatrix m)
{
  int i,j;
  
  fprintf(o, "ConcatTransform [");
  for (i=0; i<4; ++i) {
    for (j=0; j<4; ++j) {
      fprintf(o, "%f ", m[i][j]);
    }
  }
  fprintf(o, "]\n");
}


static void
rib_camera(FILE *o, Lib3dsFile *f, Lib3dsMatrix M)
{
  Lib3dsNode *c;
  Lib3dsNode *t;
  const char *name=camera;

  ASSERT(f);
  if (!name) {
    if (f->cameras) {
      name=f->cameras->name;
    }
  }
  if (!name) {
    fprintf(stderr, "***ERROR*** No camera found!\n");
    exit(1);
  }
  c=lib3ds_file_node_by_name(f, name, LIB3DS_CAMERA_NODE);
  t=lib3ds_file_node_by_name(f, name, LIB3DS_TARGET_NODE);
  if (!c || !t) {
    fprintf(stderr, "***ERROR*** Invalid camera/target!\n");
    exit(1);
  }

  lib3ds_matrix_camera(M, c->data.camera.pos, t->data.target.pos, c->data.camera.roll);
  rib_concat_transform(o, M);
  fprintf(o, "Projection \"perspective\" \"fov\" [%f]\n", c->data.camera.fov);
}


static void
rib_lights(FILE *o, Lib3dsFile *f, Lib3dsMatrix M)
{
  Lib3dsLight *light;
  Lib3dsNode *l;
  Lib3dsNode *s;
  int i=1;

  for (light=f->lights; light; light=light->next) {
    l=lib3ds_file_node_by_name(f, light->name, LIB3DS_LIGHT_NODE);
    s=lib3ds_file_node_by_name(f, light->name, LIB3DS_SPOT_NODE);
    if (!l) {
      fprintf(stderr, "***ERROR*** Invalid light!\n");
      exit(1);
    }
    if (s) {
      Lib3dsVector pos,spot;
      lib3ds_vector_copy(pos, l->data.light.pos);
      lib3ds_vector_copy(spot, s->data.spot.pos);
      fprintf(o,
        "LightSource "
        "\"lib3dslight\" "
        "%d "
        "\"string lighttype\" [\"spot\"] "
        "\"point from\" [%f %f %f] "
        "\"point to\" [%f %f %f]\n",
        i,
        pos[0],
        pos[1],
        pos[2],
        spot[0],
        spot[1],
        spot[2]
      );
    }
    else {
      Lib3dsVector pos;
      lib3ds_vector_copy(pos, l->data.light.pos);
      fprintf(o,
        "LightSource "
        "\"pointlight\" "
        "%d "
        "\"from\" [%f %f %f]\n",
        i,
        pos[0],
        pos[1],
        pos[2]
      );
    }
    fprintf(o, "Illuminate %d 1\n", i);
    ++i;
  }
}


static void
create_node(Lib3dsFile *f, Lib3dsNode *node, FILE *o)
{
  Lib3dsMesh *mesh;
  
  if ((node->type==LIB3DS_OBJECT_NODE) && (strcmp(node->name,"$$$DUMMY")!=0)) {
    mesh=lib3ds_file_mesh_by_name(f, node->name);
    ASSERT(mesh);
    if (mesh) {
      Lib3dsObjectData *d=&node->data.object;
      
      fprintf(o, "AttributeBegin\n");
      fprintf(o, "Surface \"matte\" \"Kd\" [0.75]\n");
      fprintf(o, "Color 1 1 1\n");

      {
        Lib3dsMatrix N,M,X;
        lib3ds_matrix_copy(N, node->matrix);
        lib3ds_matrix_translate_xyz(N, -d->pivot[0], -d->pivot[1], -d->pivot[2]);
        lib3ds_matrix_copy(M, mesh->matrix);
        lib3ds_matrix_inv(M);
        lib3ds_matrix_mul(X,N,M);
        rib_concat_transform(o, X);
      }
      {
        unsigned p;
        Lib3dsVector *normalL=malloc(3*sizeof(Lib3dsVector)*mesh->faces);
        lib3ds_mesh_calculate_normals(mesh, normalL);
        
        for (p=0; p<mesh->faces; ++p) {
          Lib3dsFace *face=&mesh->faceL[p];
          Lib3dsMaterial *mat=lib3ds_file_material_by_name(f, face->material);
          if (mat) {
            fprintf(o, "Color [%f %f %f]\n",
              mat->diffuse[0],
              mat->diffuse[1],
              mat->diffuse[2]
            );
            fprintf(o,
              "Surface "
              "\"lib3dsmaterial\" "
              "\"color specularcolor\" [%f %f %f] "
              "\"float shininess \" [%f] "
              "\"float shin_stength \" [%f] "
              "\n",
              mat->specular[0],
              mat->specular[1],
              mat->specular[2],
              mat->shininess,
              mat->shin_strength
            );
          }
          fprintf(o, "Polygon \"P\" [%f %f %f %f %f %f %f %f %f] ",
            mesh->pointL[face->points[0]].pos[0],
            mesh->pointL[face->points[0]].pos[1],
            mesh->pointL[face->points[0]].pos[2],
            mesh->pointL[face->points[1]].pos[0],
            mesh->pointL[face->points[1]].pos[1],
            mesh->pointL[face->points[1]].pos[2],
            mesh->pointL[face->points[2]].pos[0],
            mesh->pointL[face->points[2]].pos[1],
            mesh->pointL[face->points[2]].pos[2] 
          );

          fprintf(o, "\"N\" [%f %f %f %f %f %f %f %f %f] ",
            normalL[3*p+0][0],
            normalL[3*p+0][1],
            normalL[3*p+0][2],
            normalL[3*p+1][0],
            normalL[3*p+1][1],
            normalL[3*p+1][2],
            normalL[3*p+2][0],
            normalL[3*p+2][1],
            normalL[3*p+2][2]
          );
        }

        free(normalL);
      }
      fprintf(o, "AttributeEnd\n");
    }
  }
  {
    Lib3dsNode *n;
    for (n=node->childs; n; n=n->next) {
      create_node(f,n,o);
    }
  }
}


static void
create_rib(Lib3dsFile *f, FILE *o, int current)
{
  Lib3dsMatrix M;
  fprintf(o, "FrameBegin %d\n", current);
  fprintf(o, "Display \"example%d.tiff\" \"tiff\" \"rgb\"\n", current);
  fprintf(o, "Format 400 400 1\n");
  fprintf(o, "Exposure 1.0 2.0\n");
  fprintf(o, "Clipping 1.0 20000\n");
  fprintf(o, "Scale 1 -1 1\n");
  fprintf(o, "Sides 1\n");
  fprintf(o, "Rotate 90 1 0 0\n");

  /*
  fprintf(o,
    "LightSource "
    "\"distantlight\" "
    "0 "
    "\"from\" [0 0 0] "
    "\"to\" [0 1 0] "
  );
  */
  rib_camera(o,f,M);
  
  fprintf(o, "WorldBegin\n");
  rib_lights(o,f,M);
  {
    Lib3dsNode *n;
    for (n=f->nodes; n; n=n->next) {
      create_node(f,n,o);
    }
  }
  fprintf(o, "WorldEnd\n");
  fprintf(o, "FrameEnd\n");
}


int
main(int argc, char **argv)
{
  Lib3dsFile *f=0;
  FILE *o;

  parse_args(argc, argv);
  f=lib3ds_file_load(filename);
  if (!f) {
    fprintf(stderr, "***ERROR***\nLoading file %s failed\n", filename);
    exit(1);
  }

  if (output) {
    o=fopen(output, "w+");
    if (!o) {
      fprintf(stderr, "***ERROR***\nCan't open %s for writing\n", output);
      exit(1);
    }
  }
  else {
    o=stdout;
  }

  fprintf(o, "Option \"searchpath\" \"shader\" [\".:../shaders:&\"]\n");
  if (flags&LIB3DS2RIB_ALL) {
    int i;
    for (i=f->segment_from; i<=f->segment_to; ++i) {
      lib3ds_file_eval(f,1.0f*i);
      create_rib(f,o,i);
    }
  }
  else
  if (downcuts) {
    int i;
    int delta=f->segment_to-f->segment_from;
    for (i=0; i<downcuts; ++i) {
      float frame=f->segment_from+1.0f*i*delta/(downcuts-1);
      lib3ds_file_eval(f, frame);
      create_rib(f,o,i);
    }
  }
  else {  
    lib3ds_file_eval(f,frame);
    create_rib(f,o, (int)frame);
  }

  if (o!=stdout) {
    fclose(o);
  }
  lib3ds_file_free(f);
  return(0);
}








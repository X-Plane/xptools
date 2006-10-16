/*
 * sphere - generate a triangle mesh approximating a sphere by
 *  recursive subdivision. First approximation is an platonic
 *  solid; each level of refinement increases the number of
 *  triangles by a factor of 4.
 *
 * Level 3 (128 triangles for an octahedron) is a good tradeoff if
 *  gouraud shading is used to render the database.
 *
 * Usage: sphere [level] [-p] [-c] [-f] [-t] [-i]
 *      level is an integer >= 1 setting the recursion level (default 1).
 *      -c causes triangles to be generated with vertices in counterclockwise
 *          order as viewed from the outside in a RHS coordinate system.
 *          The default is clockwise order.
 *      -t starts with a tetrahedron instead of an octahedron
 *      -i starts with a icosahedron instead of an octahedron
 *
 *  The subroutines print_object() and print_triangle() should
 *  be changed to generate whatever the desired database format is.
 *
 * Jon Leech (leech @ cs.unc.edu) 3/24/89
 * icosahedral code added by Jim Buddenhagen (jb1556@daditz.sbc.com) 5/93
 *
 * C++ version and Dime output added by pederb@sim.no
 */


// define this to create dime3DFace, otherwise dimeLine will be used. 
#define DXFSPHERE_FILLED 1

// define this to use UnknownEntity instead of dime3DFace
// #define DXFSPHERE_USE_UNKNOWNENTITY 1

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

// DIME: needed include files.
#include <dime/Model.h>
#include <dime/sections/EntitiesSection.h>
#include <dime/sections/TablesSection.h>
#include <dime/sections/BlocksSection.h>
#include <dime/tables/LayerTable.h>
#include <dime/tables/Table.h>
#include <dime/entities/3DFace.h>
#include <dime/entities/Line.h>
#include <dime/entities/Block.h>
#include <dime/entities/Insert.h>
#include <dime/entities/UnknownEntity.h>
#include <dime/Output.h>
#include <dime/util/Linear.h>

#define LAYERNAME1 "MYLAYER1" // Layernames can't have spaces
#define LAYERNAME2 "MYLAYER2" // Layernames can't have spaces

class point {
public:
  point() { }
  point(const point & p) 
    : x(p.x), y(p.y), z(p.z) { }
  point(double x, double y, double z)
    : x(x), y(y), z(z) { }
public:
  double  x, y, z;
};

class triangle {
public:
  triangle(void) { }
  triangle(const point & p0, const point & p1, const point & p2,
           double area) {
    pt[0] = p0;
    pt[1] = p1;
    pt[2] = p2;
    area = area;
  } 
public:
  point     pt[3];    /* Vertices of triangle */
  double    area;     /* Unused; might be used for adaptive subdivision */
};

class object {
public:
  object(void) : npoly(0), poly(NULL) { }
  object(int npoly, triangle * poly) 
    : npoly(npoly), poly(poly) { }
public:
  int       npoly;    /* # of triangles in object */
  triangle *poly;     /* Triangles */
};

/* Six equidistant points lying on the unit sphere */
#define XPLUS point(1,  0,  0)    /*  X */
#define XMIN  point(-1,  0,  0)    /* -X */
#define YPLUS point(0,  1,  0)    /*  Y */
#define YMIN  point(0, -1,  0)    /* -Y */
#define ZPLUS point(0,  0,  1)    /*  Z */
#define ZMIN  point(0,  0, -1)    /* -Z */

/* Vertices of a unit octahedron */
triangle octahedron[] = {
  triangle(XPLUS, ZPLUS, YPLUS, 0.0),
  triangle(YPLUS, ZPLUS, XMIN, 0.0),
  triangle(XMIN , ZPLUS, YMIN, 0.0),
  triangle(YMIN , ZPLUS, XPLUS, 0.0),
  triangle(XPLUS, YPLUS, ZMIN, 0.0),
  triangle(YPLUS, XMIN , ZMIN, 0.0),
  triangle(XMIN , YMIN , ZMIN, 0.0),
  triangle(YMIN , XPLUS, ZMIN, 0.0)
  };

/* A unit octahedron */
object oct(sizeof(octahedron) / sizeof(octahedron[0]),
  octahedron);

/* Vertices of a tetrahedron */
#define sqrt_3 0.5773502692
#define PPP point(sqrt_3,  sqrt_3,  sqrt_3)   /* +X, +Y, +Z */
#define MMP point(-sqrt_3, -sqrt_3,  sqrt_3)   /* -X, -Y, +Z */
#define MPM point(-sqrt_3,  sqrt_3, -sqrt_3)   /* -X, +Y, -Z */
#define PMM point(sqrt_3, -sqrt_3, -sqrt_3)   /* +X, -Y, -Z */

/* Structure describing a tetrahedron */
triangle tetrahedron[] = {
  triangle(PPP, MMP, MPM, 0.0),
  triangle(PPP, PMM, MMP, 0.0),
  triangle(MPM, MMP, PMM, 0.0),
  triangle(PMM, PPP, MPM, 0.0)
  };

object tet(
sizeof(tetrahedron) / sizeof(tetrahedron[0]),
  tetrahedron);

/* Twelve vertices of icosahedron on unit sphere */
#define tau 0.8506508084      /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
#define one 0.5257311121      /* one=1/sqrt(1+t^2) , unit sphere     */
#define ZA point(tau,  one,    0)
#define ZB point(-tau,  one,    0)
#define ZC point(-tau, -one,    0)
#define ZD point( tau, -one,    0)
#define YA point( one,   0 ,  tau)
#define YB point( one,   0 , -tau)
#define YC point(-one,   0 , -tau)
#define YD point(-one,   0 ,  tau)
#define XA point(  0 ,  tau,  one)
#define XB point(  0 , -tau,  one)
#define XC point(  0 , -tau, -one)
#define XD point(  0 ,  tau, -one)

/* Structure for unit icosahedron */
triangle icosahedron[] = {
  triangle(YA, XA, YD, 0.0),
  triangle(YA, YD, XB, 0.0),
  triangle(YB, YC, XD, 0.0),
  triangle(YB, XC, YC, 0.0),
  triangle(ZA, YA, ZD, 0.0),
  triangle(ZA, ZD, YB, 0.0),
  triangle(ZC, YD, ZB, 0.0),
  triangle(ZC, ZB, YC, 0.0),
  triangle(XA, ZA, XD, 0.0),
  triangle(XA, XD, ZB, 0.0),
  triangle(XB, XC, ZD, 0.0),
  triangle(XB, ZC, XC, 0.0),
  triangle(XA, YA, ZA, 0.0),
  triangle(XD, ZA, YB, 0.0),
  triangle(YA, XB, ZD, 0.0),
  triangle(YB, ZD, XC, 0.0),
  triangle(YD, XA, ZB, 0.0),
  triangle(YC, ZB, XD, 0.0),
  triangle(YD, ZC, XB, 0.0),
  triangle(YC, XC, ZC, 0.0)
};

/* unit icosahedron */
object ico(sizeof(icosahedron) / sizeof(icosahedron[0]),
  icosahedron);

/* Forward declarations */
point *normalize(point * p);
point *midpoint(point * a, point * b);
void flip_object(object * obj);
void print_object(object * obj, int level, dimeModel & model, const char * layername,
                  dimeBlock * block = NULL);
void print_triangle(triangle *t, dimeModel & model, const dimeLayer * layer,
                    dimeBlock * block = NULL);

static void
add_layer(const char * name, int colnum, dimeModel * model, dimeTable * layers)
{
  dimeLayerTable * layer = new dimeLayerTable;
  layer->setLayerName(name, NULL);
  layer->setColorNumber(colnum); // the color numbers are defined in dime/Layer.cpp.

  // need to set some extra records so that AutoCAD will stop
  // complaining
  dimeParam param;
  param.string_data = "CONTINUOUS";
  layer->setRecord(6, param);
  param.int16_data = 64;
  layer->setRecord(70, param);
  layer->registerLayer(model); // important, register layer in model
  layers->insertTableEntry(layer);
}

int
main(int ac, char ** av)
{
  object * old = &oct,         /* Default is octahedron */
    * newobj;
  int ccwflag = 1,        /* Reverse vertex order if true */
    i,
    level,              /* Current subdivision level */
    maxlevel = 0;       /* Maximum subdivision level */

  int useblock = 0;

  char * outfile = NULL;
  
  /* Parse arguments */
  for (i = 1; i < ac; i++) {
    if (!strcmp(av[i], "-c"))
      ccwflag = 1;
    else if (!strcmp(av[i], "-t"))
      old = &tet;
    else if (!strcmp(av[i], "-i"))
      old = &ico;
    else if (!strcmp(av[i], "-b"))
      useblock = 1;
    else if (!strcmp(av[i], "-o") && i < ac-1) {
      outfile = av[i+1];
      i++;
    }
    else if (isdigit(av[i][0])) {
      if ((maxlevel = atoi(av[i])) < 1) {
	fprintf(stderr, "dxfsphere: # of levels must be >= 1\n");
	exit(1);
      }
    } 
    else {
      break;
    }
  }
  
  if (i < ac || ac == 1) {
    fprintf(stderr, "dxfsphere: [-c] [-t] [-i] [-b] [-o <outfile>] <levels>\n");
    exit(1);
  }

  // DIME: initialize output file
  dimeOutput out;
  if (!outfile || !out.setFilename(outfile)) {
    out.setFileHandle(stdout);
  }
  // DIME: create dime model
  dimeModel model;

  // DIME: only needed if you need your object to be in a layer
  {
    // DIME: add tables section (needed for layers).
    dimeTablesSection * tables = new dimeTablesSection;
    model.insertSection(tables);
    
    // DIME: set up a layer table to store our layers
    dimeTable * layers = new dimeTable(NULL);
    
    // DIME: set up our layers
    add_layer(LAYERNAME1, 16, &model, layers);
    add_layer(LAYERNAME2, 8, &model, layers);

    // DIME: insert the layer in the table
    tables->insertTable(layers); 
  }

  // DIME: only needed if you want to create the sphere as a BLOCK
  dimeBlock * block = NULL;
  if (useblock) {
    dimeBlocksSection * blocks = new dimeBlocksSection;
    model.insertSection(blocks);
    block = new dimeBlock(NULL);
    block->setName("MyBlock");
    blocks->insertBlock(block);
  }

  // DIME: add the entities section.
  dimeEntitiesSection * entities = new dimeEntitiesSection;
  model.insertSection(entities);

  if (ccwflag)
    flip_object(old);
  
  /* Subdivide each starting triangle (maxlevel - 1) times */
  for (level = 1; level < maxlevel; level++) {
    /* Allocate a new object */
    /* FIXME: Valgrind reports an 8-byte memory leak here. 20030404 mortene. */
    newobj = (object *)malloc(sizeof(object));
    if (newobj == NULL) {
      fprintf(stderr, "%s: Out of memory on subdivision level %d\n",
              av[0], level);
      exit(1);
    }
    newobj->npoly = old->npoly * 4;
    
    /* Allocate 4* the number of points in the current approximation */
    newobj->poly  = (triangle *)malloc(newobj->npoly * sizeof(triangle));
    if (newobj->poly == NULL) {
      fprintf(stderr, "%s: Out of memory on subdivision level %d\n",
              av[0], level);
      exit(1);
    }
      
    /* Subdivide each triangle in the old approximation and normalize
     *  the new points thus generated to lie on the surface of the unit
     *  sphere.
     * Each input triangle with vertices labelled [0,1,2] as shown
     *  below will be turned into four new triangles:
     *
     *                      Make new points
     *                          a = (0+2)/2
     *                          b = (0+1)/2
     *                          c = (1+2)/2
     *        1
     *       /\             Normalize a, b, c
     *      /  \
     *    b/____\ c         Construct new triangles
     *    /\    /\              [0,b,a]
     *   /  \  /  \             [b,1,c]
     *  /____\/____\            [a,b,c]
     * 0      a     2           [a,c,2]
     */
    for (i = 0; i < old->npoly; i++) {
      triangle
        *oldt = &old->poly[i],
        *newt = &newobj->poly[i*4];
      point a, b, c;
      
      a = *normalize(midpoint(&oldt->pt[0], &oldt->pt[2]));
      b = *normalize(midpoint(&oldt->pt[0], &oldt->pt[1]));
      c = *normalize(midpoint(&oldt->pt[1], &oldt->pt[2]));
        
      newt->pt[0] = oldt->pt[0];
      newt->pt[1] = b;
      newt->pt[2] = a;
      newt++;
        
      newt->pt[0] = b;
      newt->pt[1] = oldt->pt[1];
      newt->pt[2] = c;
      newt++;
      
      newt->pt[0] = a;
      newt->pt[1] = b;
      newt->pt[2] = c;
      newt++;
      
      newt->pt[0] = a;
      newt->pt[1] = c;
      newt->pt[2] = oldt->pt[2];
    }
    
    if (level > 1) {
      free(old->poly);
      free(old);
    }
    
    /* Continue subdividing new triangles */
    old = newobj;
  }
  
  /* Print out resulting approximation */
  print_object(old, maxlevel, model, LAYERNAME1, block);

  if (block) {
    dimeInsert * insert = new dimeInsert;
    insert->setBlock(block);
    model.addEntity(insert);
  }

  // DIME: write the model to file
  model.write(&out);

  return 0;
}

/* Normalize a point p */
point * normalize(point * p)
{
  static point r;
  double mag;
  
  r = *p;
  mag = r.x * r.x + r.y * r.y + r.z * r.z;
  if (mag != 0.0) {
    mag = 1.0 / sqrt(mag);
    r.x *= mag;
    r.y *= mag;
    r.z *= mag;
  }
  
  return &r;
}

/* Return the midpoint on the line between two points */
point * midpoint(point * a, point * b)
{
  static point r;
  
  r.x = (a->x + b->x) * 0.5;
  r.y = (a->y + b->y) * 0.5;
  r.z = (a->z + b->z) * 0.5;
  
  return &r;
}

/* Reverse order of points in each triangle */
void flip_object(object * obj)
{
  int i;
  for (i = 0; i < obj->npoly; i++) {
    point tmp;
    tmp = obj->poly[i].pt[0];
    obj->poly[i].pt[0] = obj->poly[i].pt[2];
    obj->poly[i].pt[2] = tmp;
  }
}

/* Write out all triangles in an object */
void print_object(object * obj, int level, dimeModel & model, const char * layername,
                  dimeBlock * block)
{
  int i;
  
  const dimeLayer * layer = model.getLayer(layername);

  for (i = 0; i < obj->npoly; i++) {
    print_triangle(&obj->poly[i], model, layer, block);  
  }
}


/* Output a triangle */
void print_triangle(triangle * t, dimeModel & model, const dimeLayer * layer,
                    dimeBlock * block)
{
#if defined(DXFSPHERE_FILLED) && !defined(DXFSPHERE_USE_UNKNOWNENTITY)
  // filled, create dime3DFace
  int i;

  // DIME: create a 3DFACE entity, and set it to contain a triangle
  dime3DFace * face = new dime3DFace;
  if (layer) {
    face->setLayer(layer);
  }
  dimeVec3f v[3];

  for (i = 0; i < 3; i++) {
    v[i].x = t->pt[i].x;
    v[i].y = t->pt[i].y;
    v[i].z = t->pt[i].z;
  }
  face->setTriangle(v[0], v[1], v[2]);

  // DIME: create a unique handle for this entity.
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];
  const char * handle = model.getUniqueHandle(buf, BUFSIZE);
  
  dimeParam param;
  param.string_data = handle;
  face->setRecord(5, param);

  // DIME: add entity to model
  if (block) {
    block->insertEntity(face);
  }
  else {
    model.addEntity(face);
  }
#elif defined(DXFSPHERE_USE_UNKNOWNENTITY)

  // DIME: create a dimeUnknownEntity, and set it to contain a triangle
  dimeUnknownEntity * face = new dimeUnknownEntity("3DFACE", NULL);
  if (layer) {
    face->setLayer(layer);
  }
  dimeParam param;

  // 10,20,30 is the first vertex
  // 11,21,31 is the second vertex
  // 12,22,32 is the third vertex
  for (int i = 0; i < 3; i++) {
    param.double_data = t->pt[i].x;
    face->setRecord(i + 10, param);

    param.double_data = t->pt[i].y;
    face->setRecord(i + 20, param);

    param.double_data = t->pt[i].z;
    face->setRecord(i + 30, param);    
  }
  // to make 3DFACE contain a triangle and not a quad, the fourth
  // vertex must be equal to the third. We therefore iterate from 0 to
  // 4, but clamp the index when fetching the vertex from the triangle
  // structure.
  param.double_data = t->pt[2].x;
  face->setRecord(13, param);
  param.double_data = t->pt[2].y;
  face->setRecord(23, param);
  param.double_data = t->pt[2].z;
  face->setRecord(33, param);

  // DIME: create a unique handle for this entity.
  const int BUFSIZE = 1024;
  char buf[BUFSIZE];
  const char * handle = model.getUniqueHandle(buf, BUFSIZE);
  
  param.string_data = handle;
  face->setRecord(5, param);

  // DIME: add entity to model
  model.addEntity(face);

#else
  // create three dimeLine entities to represent the triangle
  int i;
  for (i = 0; i < 3; i++) {
    // DIME: create a LINE entity
    dimeLine * line = new dimeLine;
    if (layer) {
      line->setLayer(layer);
    }
    dimeVec3f v[2];
    v[0].x = t->pt[i].x;
    v[0].y = t->pt[i].y;
    v[0].z = t->pt[i].z;

    v[1].x = t->pt[(i+1)%3].x;
    v[1].y = t->pt[(i+1)%3].y;
    v[1].z = t->pt[(i+1)%3].z;

    line->setCoords(0, v[0]);
    line->setCoords(1, v[1]);
    
    // DIME: create unique handle for the entity (needed to load the file into AutoCAD)
    const int BUFSIZE = 1024;
    char buf[BUFSIZE];
    const char * handle = model.getUniqueHandle(buf, BUFSIZE);
    
    dimeParam param;
    param.string_data = handle;
    line->setRecord(5, param);

    // DIME: add entity to model
    if (block) {
      block->insertEntity(line);
    }
    else {
      model.addEntity(line);
    }
  }
#endif // ! DXFSPHERE_FILLED
}


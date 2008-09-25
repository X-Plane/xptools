/**************************************************************************\
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-2001 by Systems In Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License (the accompanying file named COPYING) for more
 *  details.
 *
 **************************************************************************
 *
 *  If you need DIME for a non-GPL project, contact Systems In Motion
 *  to acquire a Professional Edition License:
 *
 *  Systems In Motion                                   http://www.sim.no/
 *  Prof. Brochs gate 6                                       sales@sim.no
 *  N-7030 Trondheim                                   Voice: +47 22114160
 *  NORWAY                                               Fax: +47 22207097
 *
\**************************************************************************/

#include <dime/convert/layerdata.h>
#include <dime/Layer.h>

/*!
  \class dxfLayerData layerdata.h
  \brief The dxfLayerData class handles all geometry for a given color index.
  DXF geometry is grouped into different colors, as this is a normal way
  to group geometry data, and especially vrml data.

  The geometry can be either points, lines or polygons.
*/

/*!
  Constructor
*/
dxfLayerData::dxfLayerData(const int colidx)
{
  this->fillmode = true;
  this->colidx = colidx;
}

/*!
  Destructor.
*/
dxfLayerData::~dxfLayerData()
{
}

/*!
  Sets the fillmode for this layer. If fillmode is set (the default)
  polylines with width and/or height will be converter to polygons
  and not lines. The same goes for the SOLID and TRACE entities.
*/
void
dxfLayerData::setFillmode(const bool fillmode)
{
  this->fillmode = fillmode;
}

/*!
  Adds a line to this layer's geometry. If \a matrix != NULL, the
  points will be transformed by this matrix before they are added.
*/
void
dxfLayerData::addLine(const dimeVec3f &v0, const dimeVec3f &v1,
		      const dimeMatrix * const matrix)
{
  int i0, i1;

  if (matrix) {
    dimeVec3f t0, t1;
    matrix->multMatrixVec(v0, t0);
    matrix->multMatrixVec(v1, t1);
    i0 = linebsp.addPoint(t0);
    i1 = linebsp.addPoint(t1);
  }
  else {
    i0 = linebsp.addPoint(v0);
    i1 = linebsp.addPoint(v1);
  }

  //
  // take care of line strips (more effective than single lines)
  //
  if (lineindices.count() && lineindices[lineindices.count()-1] == i0) {
    lineindices.append(i1);
  }
  else {
    if (lineindices.count()) lineindices.append(-1);
    lineindices.append(i0);
    lineindices.append(i1);
  }
}

/*!
  Adds a point to this layer's geometry. If \a matrix != NULL, the
  point will be transformed by this matrix before they are added.
*/
void
dxfLayerData::addPoint(const dimeVec3f &v,
		       const dimeMatrix * const matrix)
{
  if (matrix) {
    dimeVec3f t;
    matrix->multMatrixVec(v, t);
    points.append(t);
  }
  else {
    points.append(v);
  }
}

/*!
  Adds a triangle to this layer's geometry. If \a matrix != NULL, the
  points will be transformed by this matrix before they are added.
*/
void
dxfLayerData::addTriangle(const dimeVec3f &v0,
			  const dimeVec3f &v1,
			  const dimeVec3f &v2,
			  const dimeMatrix * const matrix)
{
  if (this->fillmode) {
    if (matrix) {
      dimeVec3f t0, t1, t2;
      matrix->multMatrixVec(v0, t0);
      matrix->multMatrixVec(v1, t1);
      matrix->multMatrixVec(v2, t2);
      faceindices.append(facebsp.addPoint(t0));
      faceindices.append(facebsp.addPoint(t1));
      faceindices.append(facebsp.addPoint(t2));
      faceindices.append(-1);
    }
    else {
      faceindices.append(facebsp.addPoint(v0));
      faceindices.append(facebsp.addPoint(v1));
      faceindices.append(facebsp.addPoint(v2));
      faceindices.append(-1);
    }
  }
  else {
    this->addLine(v0, v1, matrix);
    this->addLine(v1, v2, matrix);
    this->addLine(v2, v0, matrix);
  }
}

/*!
  Adds a quad to this layer's geometry. If \a matrix != NULL, the
  points will be transformed by this matrix before they are added.
*/
void
dxfLayerData::addQuad(const dimeVec3f &v0,
		      const dimeVec3f &v1,
		      const dimeVec3f &v2,
		      const dimeVec3f &v3,
		      const dimeMatrix * const matrix)
{
  if (this->fillmode) {
    if (matrix) {
      dimeVec3f t0, t1, t2, t3;
      matrix->multMatrixVec(v0, t0);
      matrix->multMatrixVec(v1, t1);
      matrix->multMatrixVec(v2, t2);
      matrix->multMatrixVec(v3, t3);
      faceindices.append(facebsp.addPoint(t0));
      faceindices.append(facebsp.addPoint(t1));
      faceindices.append(facebsp.addPoint(t2));
      faceindices.append(facebsp.addPoint(t3));
      faceindices.append(-1);
    }
    else {
      faceindices.append(facebsp.addPoint(v0));
      faceindices.append(facebsp.addPoint(v1));
      faceindices.append(facebsp.addPoint(v2));
      faceindices.append(facebsp.addPoint(v3));
      faceindices.append(-1);
    }
  }
  else {
    this->addLine(v0, v1, matrix);
    this->addLine(v1, v2, matrix);
    this->addLine(v2, v3, matrix);
    this->addLine(v3, v0, matrix);
  }
}

/*!
  Exports this layer's geometry as vrml nodes.
*/
void
dxfLayerData::writeWrl(FILE *fp, int indent, const bool vrml1,
                       const bool only2d)
{
#ifndef NOWRLEXPORT
  if (!faceindices.count() && !lineindices.count() && !points.count()) return;

  int i, n;

  dxfdouble r,g,b;

  dimeLayer::colorToRGB(this->colidx, r, g, b);

  if (vrml1) {
    fprintf(fp,
            "Separator {\n");
  }
  else {
    fprintf(fp,
            "Group {\n"
            "  children [\n");
  }
  if (faceindices.count()) {
    if (vrml1) {
      fprintf(fp,
              "  Separator {\n"
              "    Material {\n"
              "      diffuseColor %g %g %g\n"
              "    }\n"
              "    ShapeHints {\n"
              "      creaseAngle 0.5\n"
              "      vertexOrdering COUNTERCLOCKWISE\n"
              "      shapeType UNKNOWN_SHAPE_TYPE\n"
              "      faceType UNKNOWN_FACE_TYPE\n"
              "    }\n"
              "    Coordinate3 {\n"
              "      point [\n", r, g, b);
    }
    else {
      fprintf(fp,
              "    Shape {\n"
              "      appearance Appearance {\n"
              "        material Material {\n"
              "          diffuseColor %g %g %g\n"
              "        }\n"
              "      }\n"
              "      geometry IndexedFaceSet {\n"
              "        convex FALSE\n"
              "        solid FALSE\n"
              "        creaseAngle 0.5\n" // a good value for most cases
              "        coord Coordinate {\n"
              "          point [\n", r, g, b);
    }
    dimeVec3f v;
    n = facebsp.numPoints();
    for (i = 0; i < n ; i++) {
      facebsp.getPoint(i, v);
      if (only2d) v[2] = 0.0f;
      if (i < n-1)
	fprintf(fp, "            %.8g %.8g %.8g,\n", v[0], v[1], v[2]);
      else
	fprintf(fp, "            %.8g %.8g %.8g\n", v[0], v[1], v[2]);
    }
    fprintf(fp,
	    "          ]\n"
	    "        }\n");
    if (vrml1) {
      fprintf(fp,
              "    IndexedFaceSet {\n"
              "      coordIndex [\n          ");
    }
    else {
      fprintf(fp,
              "        coordIndex [\n          ");
    }
    n = faceindices.count();
    int cnt = 1;
    for (i = 0; i < n; i++) {
      if ((cnt & 7) && i < n-1) // typical case
	fprintf(fp, "%d,", faceindices[i]);
      else if (!(cnt & 7) && i < n-1)
	fprintf(fp, "%d,\n          ", faceindices[i]);
      else
	fprintf(fp, "%d\n", faceindices[i]);
      cnt++;
    }
    fprintf(fp,
	    "        ]\n"
	    "      }\n"
	    "    }\n");
  }
  if (lineindices.count()) {
    // make sure line indices has a -1 at the end
    if (lineindices[lineindices.count()-1] != -1) {
      lineindices.append(-1);
    }
    if (vrml1) {
      fprintf(fp,
              "  Separator {\n"
              "    Material {\n"
              "      diffuseColor %g %g %g\n"
              "    }\n"
              "    Coordinate3 {\n"
              "      point [\n", r, g, b);
    }
    else {
      fprintf(fp,
              "    Shape {\n"
              "      appearance Appearance {\n"
              "        material Material {\n"
              "          emissiveColor %g %g %g\n"
              "        }\n"
              "      }\n"
              "      geometry IndexedLineSet {\n"
              "        coord Coordinate {\n"
              "          point [\n", r, g, b);
    }
    dimeVec3f v;
    n = linebsp.numPoints();
    for (i = 0; i < n ; i++) {
      linebsp.getPoint(i, v);
      if (only2d) v[2] = 0.0f;
      if (i < n-1)
	fprintf(fp, "            %.8g %.8g %.8g,\n", v[0], v[1], v[2]);
      else
	fprintf(fp, "            %.8g %.8g %.8g\n", v[0], v[1], v[2]);
    }
    fprintf(fp,
	    "          ]\n"
	    "        }\n");
    if (vrml1) {
      fprintf(fp,
              "    IndexedLineSet {\n"
              "      coordIndex [\n          ");
    }
    else {
      fprintf(fp, "        coordIndex [\n          ");
    }

    n = lineindices.count();
    int cnt = 1;
    for (i = 0; i < n; i++) {
      if ((cnt & 7) && i < n-1) // typical case
	fprintf(fp, "%d,", lineindices[i]);
      else if (!(cnt & 7) && i < n-1)
	fprintf(fp, "%d,\n          ", lineindices[i]);
      else
	fprintf(fp, "%d\n", lineindices[i]);
      cnt++;
    }
    fprintf(fp,
	    "        ]\n"
	    "      }\n"
	    "    }\n");
  }


  if (points.count() && 0) { // FIXME disabled, suspect bug. pederb, 2001-12-11
    if (vrml1) {
      fprintf(fp,
              "  Separator {\n"
              "    Material {\n"
              "      diffuseColor %g %g %g\n"
              "    }\n"
              "    Coordinate3 {\n"
              "      point [\n", r, g, b);
    }
    else {
      fprintf(fp,
              "    Shape {\n"
              "      appearance Appearance {\n"
              "        material Material {\n"
              "          emissiveColor %g %g %g\n"
              "        }\n"
              "      \n"
              "      geometry PointSet {\n"
              "        coord Coordinate {\n"
              "          point [\n", r, g, b);
    }
    dimeVec3f v;
    n = points.count();
    for (i = 0; i < n ; i++) {
      v = points[i];
      if (only2d) v[2] = 0.0f;
      if (i < n-1)
	fprintf(fp, "            %g %g %g,\n", v[0], v[1], v[2]);
      else
	fprintf(fp, "            %g %g %g\n", v[0], v[1], v[2]);
    }
    fprintf(fp,
	    "          ]\n"
	    "        }\n");
    if (vrml1) {
      fprintf(fp,
              "    PointSet {\n"
              "      numPoints %d\n"
              "    }\n"
              "  }\n", points.count());
    }
    else {
      fprintf(fp,
              "      }\n"
              "    }\n");

    }
  }

  if (vrml1) {
    fprintf(fp, "}\n");
  }
  else {
    fprintf(fp,
            "  ]\n"
            "}\n");
  }
#endif // NOWRLEXPORT
}




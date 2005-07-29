#ifndef MESHIO_H
#define MESHIO_H

class	CDT;
struct	XAtomContainer;
#include "EnumSystem.h"
#include "ProgressUtils.h"

void WriteMesh(FILE * fi, CDT& mesh, int inAtomID, ProgressFunc func);
void ReadMesh(XAtomContainer& container, CDT& inMesh, int atomID, const TokenConversionMap& c, ProgressFunc func);

#endif
#ifndef FORESTS_H
#define FORESTS_H

#include "ProgressUtils.h"
#include "Skeleton.h"
#include "ObjPlacement.h"
class 	Pmwx;
class 	CDT;
class	GISFace;

void GenerateForests(
				Pmwx&					ioMap,
				vector<PreinsetFace>&	inFaces,
				CDT&					ioMesh,
				ProgressFunc			inProgress);

#endif

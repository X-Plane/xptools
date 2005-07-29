#ifndef FORESTS_H
#define FORESTS_H

#include "ProgressUtils.h"

class 	Pmwx;
class 	CDT;
class	GISFace;

void GenerateForests(
				Pmwx&					ioMap,
				const set<GISFace *>&	inFaces,
				CDT&					ioMesh,
				ProgressFunc			inProgress);

#endif

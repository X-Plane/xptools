#ifndef SKELETON_H
#define SKELETON_H

#include <vector>

class	GISFace;
class	Pmwx;

bool	SK_InsetPolygon(
					GISFace *				inFace,
					Pmwx&					outMap,
					int						inTerrainIn,
					int						inTerrainOut,
					int						inSteps);

#endif

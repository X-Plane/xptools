#ifndef GREEDYMESH_H
#define GREEDYMESH_H

#include "ProgressUtils.h"
class CDT;
struct DEMGeo;

void	GreedyMeshBuild(CDT& inCDT, DEMGeo& inAvail, DEMGeo& outUsed, double err_lim, double size_lim, int max_num, ProgressFunc func);

#endif /* GREEDYMESH_H */



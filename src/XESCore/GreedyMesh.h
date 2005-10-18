#ifndef GREEDYMESH_H
#define GREEDYMESH_H

#include "ProgressUtils.h"
class CDT;
struct DEMGeo;

void	GreedyMeshBuild(CDT& inCDT, DEMGeo& inDem, double err_lim, int max_num, ProgressFunc func);

#endif /* GREEDYMESH_H */



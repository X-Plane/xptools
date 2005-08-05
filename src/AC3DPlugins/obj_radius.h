#ifndef OBJ_RADIUS_H
#define OBJ_RADIUS_H

struct XObj;

void	GetObjDimensions(const XObj& inObj,
						float	minCoords[3],
						float	maxCoords[3]);
float GetObjectLesserRadius(const XObj& inObj);


#endif

#ifndef SKELETON_H
#define SKELETON_H

#include <vector>

class	GISFace;
class	Polygon2;

typedef	vector<Polygon2>		ComplexPolygon2;
typedef vector<double>			PolygonWeight;
typedef vector<PolygonWeight>	ComplexPolygonWeight;

bool	SK_InsetPolygon(
					const ComplexPolygon2&		inPolygon,
					const ComplexPolygonWeight&	inWeight,
					vector<ComplexPolygon2>&	outHoles,
					int							inSteps);	// -1 or step limit!

#endif

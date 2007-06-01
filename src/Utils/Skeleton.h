#ifndef SKELETON_H
#define SKELETON_H

#include <vector>

class	GISFace;
struct	Polygon2;

typedef	vector<Polygon2>		ComplexPolygon2;
typedef vector<double>			PolygonWeight;
typedef vector<PolygonWeight>	ComplexPolygonWeight;
typedef vector<ComplexPolygon2>	ComplexPolygonVector;

// Result codes from skeelton processing:
enum {
	skeleton_OK,				
	skeleton_OutOfSteps,		// We hit our step limit without reaching the inset we wanted.
	skeleton_InvalidResult,		// The resulting inset failed validation tests.
	skeleton_Exception			// An exception was thrown during processing.
};

int	SK_InsetPolygon(
					const ComplexPolygon2&		inPolygon,
					const ComplexPolygonWeight&	inWeight,
					ComplexPolygonVector&		outHoles,
					int							inSteps);	// -1 or step limit!

#endif

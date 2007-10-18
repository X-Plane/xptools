/* 
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#ifndef TensorUtils_H
#define TensorUtils_H

#include "CompGeomDefs2.h"

/*

	A 2-d tensor T is a matrix of the form
	  [ cos 2T  sin2T ]
	R [ sin 2T -cos2T ]
	
	Where theta (T) represents the rotation of the vector (1,0) CCW to get the normalized major Eigenvector, and R is a magnitude.
	Whoa.  Okay...really what we're saying is:
	
	- A tensor is a 2-d matrix and therefore represents an affine transform operation.
	- The eigenvectors of that transform represent the "flow" of the distorted space.
	
	We will represent our tensors via two quantities which we can stash in a vector:
	
	v.dx = R * cos2T
	v.dy = R * sin2T
	
	Thus we can recover R and T if needed.  (If the tensor's weight R was 1 we could recover sin2T from cos2T but in practice
	the weighting matters a lot.0
	
	Tensors are ADDITIVE - that is, we can sum v = v1 + v2 to merge tensors.
	
	A tensor FIELD is essentially a plane where the tensor's value changes with XY.  How it changes depends on the field (maybe only the magnitude
	changes, or maybe the tensor changes).  
	
	

*/

Vector2		Tensor2Eigen(const Vector2& t);
Vector2		Eigen2Tensor(const Vector2& e);	// E must be normalized!
Vector2		Gradient2Tensor(const Vector2& g);

Vector2		Linear_Tensor(
					const Point2&	origin, 
					const Vector2&	direction,	// MUST BE NORMALIZED
					double			decay, 
					const Point2&	p);
					
Vector2		Radial_Tensor(
					const Point2&	origin,
					double			decay,
					const Point2&	p);

Vector2		Weighted_Tensor(
					const Vector2 *	tensors,		// Tensor value for each pt
					const double *	weights,		// Weight of each pt
					int				n);

void		Normalize_Weights(
					double *		weights,
					int				n);

void		Polygon_Weights(
					const Point2 *	polygon,
					double *		weights,
					int				n,
					const			Point2& p);

#endif /* TensorUtils_H */

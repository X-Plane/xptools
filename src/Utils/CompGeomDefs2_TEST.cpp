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

#include "CompGeomDefs2.h"
#include "AssertUtils.h"

static void	TEST_Point2(void)
{
	// Construction, copy, equality
	Point2	threeFour(3.0, 4.0);
	TEST_Run((Point2(0.0, 0.0) == Point2()));
	TEST_Run((Point2(threeFour) == Point2(3.0, 4.0)));
	Point2	threeFourCopy(1.0, 2.0);	
	TEST_Run(threeFourCopy != Point2(3.0, 4.0));
	threeFourCopy = threeFour;
	TEST_Run(threeFourCopy == Point2(3.0, 4.0));

	// +/-
	threeFourCopy += Vector2(3.0, 1.0);
	TEST_Run(threeFourCopy == Point2(6.0, 5.0));
	threeFourCopy -= Vector2(7.0, 8.0);
	TEST_Run(threeFourCopy == Point2(-1.0, -3.0));
	TEST_Run(Point2(1.0, 2.0) + Vector2(3.0, 4.0) == Point2(4.0, 6.0));
	TEST_Run(Point2(1.0, 2.0) - Vector2(3.0, 4.0) == Point2(-2.0, -2.0));
	
	// Squared distance
	TEST_Run(Point2(5.0, 3.0).squared_distance(Point2(2.0, 7.0)) == 25.0);	
}

static void TEST_Vector2(void)
{
	// Construction, copy, equality
	Vector2	threeFour(3.0, 4.0);
	TEST_Run(Vector2(0.0, 0.0) == Vector2());
	TEST_Run(Vector2(3.0, 4.0) == threeFour);
	TEST_Run(Vector2(3.0, 4.0) == Vector2(Point2(3.0, 4.0)));
	TEST_Run(Vector2(Point2(4.0, 3.0), Point2(6.0, 7.0)) == Vector2(2.0, 4.0));
	Vector2	threeFourCopy(1.0, 2.0);
	TEST_Run(threeFourCopy != Vector2(3.0, 4.0));
	threeFourCopy = threeFour;
	TEST_Run(threeFourCopy == Vector2(3.0, 4.0));
	
	// +-*/
	threeFourCopy += Vector2(6.0, 3.0);
	TEST_Run(threeFourCopy == Vector2(9.0, 7.0));
	threeFourCopy *= 4.0;
	TEST_Run(threeFourCopy == Vector2(36.0, 28.0));
	threeFourCopy /= 2.0;
	TEST_Run(threeFourCopy == Vector2(18.0, 14.0));
	TEST_Run(threeFourCopy * 2.0 == Vector2(36.0, 28.0));
	TEST_Run(threeFourCopy / 2.0 == Vector2(9.0, 7.0));
	
	// Functions
	TEST_Run(Vector2(3.0, 4.0).squared_length() == 25.0);
	Vector2	fouroh(4.0, 0.0);
	TEST_Run(fouroh == Vector2(4.0, 0.0));
	fouroh.normalize();
	TEST_Run(fouroh == Vector2(1.0, 0.0));
	TEST_Run(Vector2(3.0, 4.0).perpendicular_cw() == Vector2(4.0, -3.0));
	TEST_Run(Vector2(3.0, 4.0).perpendicular_ccw() == Vector2(-4.0, 3.0));
	TEST_Run(Vector2(3.0, 4.0).dot(Vector2(5.0, 3.0)) == 27.0);
	TEST_Run(Vector2(3.0, 3.0).signed_area(Vector2(-4.0, 4.0))==12.0);
	
	TEST_Run(Vector2(4.0, 4.0).projection(Vector2(2.0, 2.0))==Vector2(2.0,2.0));
	TEST_Run(Vector2(4.0, 0.0).projection(Vector2(3.0, 2.0))==Vector2(3.0,0.0));
	
	// Test normalize more!
	
	Vector2	n1(0.33312, 0.0);
	Vector2	n2(-0.3483, 0.0);
	Vector2	n3(0.0, 0.234235);
	Vector2	n4(0.0, -0.2354);
	n1.normalize(); n2.normalize(); n3.normalize(); n4.normalize();
	TEST_Run(n1 == Vector2(1.0, 0.0));
	TEST_Run(n2 == Vector2(-1.0, 0.0));
	TEST_Run(n3 == Vector2(0.0, 1.0));
	TEST_Run(n4 == Vector2(0.0, -1.0));
}

static void TEST_Segment2(void)
{
	Segment2 onetwothreefour(Point2(1.0, 2.0), Point2(3.0, 4.0));
	TEST_Run(Segment2(Point2(), Point2()) == Segment2());
	TEST_Run(Segment2(Point2(1.0, 2.0), Point2(3.0, 4.0)) == onetwothreefour);
	TEST_Run(Segment2(Point2(1.0, 2.0), Point2(3.0, 4.0)) == Segment2(onetwothreefour));
	Segment2 onetwothreefourcopy(Point2(3.0, 1.0), Point2(5.0, 6.0));
	TEST_Run(onetwothreefourcopy != onetwothreefour);
	onetwothreefourcopy = onetwothreefour;
	TEST_Run(onetwothreefourcopy == onetwothreefour);
	
	// Operators
	onetwothreefourcopy += Vector2(4.0, 5.0); 
	TEST_Run(Segment2(Point2(5.0, 7.0), Point2(7.0, 9.0)) == onetwothreefourcopy);
	
	Segment2	diagonal(Point2(0,3), Point2(4,7));
	Segment2	vertical(Point2(1, 5), Point2(1,9));
	Segment2	horizontal(Point2(3, 5), Point2(-5,5));
	Segment2	emptyish(Point2(3, 5), Point2(3,5));
	TEST_Run(!diagonal.is_vertical());
	TEST_Run(!diagonal.is_horizontal());
	TEST_Run(!diagonal.is_empty());
	TEST_Run( vertical.is_vertical());
	TEST_Run(!vertical.is_horizontal());
	TEST_Run(!vertical.is_empty());
	TEST_Run(!horizontal.is_vertical());
	TEST_Run( horizontal.is_horizontal());
	TEST_Run(!horizontal.is_empty());
	TEST_Run( emptyish.is_vertical());
	TEST_Run( emptyish.is_horizontal());
	TEST_Run( emptyish.is_empty());
	
	TEST_Run(diagonal.y_at_x(0.0) == 3.0);
	TEST_Run(diagonal.y_at_x(2.0) == 5.0);
	TEST_Run(diagonal.y_at_x(4.0) == 7.0);
	TEST_Run(diagonal.x_at_y(3.0) == 0.0);
	TEST_Run(diagonal.x_at_y(5.0) == 2.0);
	TEST_Run(diagonal.x_at_y(7.0) == 4.0);

	TEST_Run(vertical.x_at_y(5.0) == 1.0);
	TEST_Run(vertical.x_at_y(7.0) == 1.0);
	TEST_Run(vertical.x_at_y(9.0) == 1.0);

	TEST_Run(horizontal.y_at_x(3.0) == 5.0);
	TEST_Run(horizontal.y_at_x(-1.0) == 5.0);
	TEST_Run(horizontal.y_at_x(-5.0) == 5.0);
	
	// Functions
	Segment2	pythag(Point2(), Point2(3.0, 4.0));
	TEST_Run(pythag.squared_length() == 25.0);
	TEST_Run(pythag.midpoint() == Point2(1.5, 2.0));
	TEST_Run(pythag.midpoint(0.0) == Point2(0.0, 0.0));
	TEST_Run(pythag.midpoint(1.0) == Point2(3.0, 4.0));
	TEST_Run(pythag.midpoint(0.25) == Point2(0.75, 1.0));
	
	TEST_Run(Segment2(Point2(0,0), Point2(40, 40)).projection(Point2(-1, 11)) == Point2(5, 5));

	TEST_Run(!Segment2(Point2(0,0), Point2(40, 40)).collinear_has_on(Point2(-1.0, -1.0)));
	TEST_Run( Segment2(Point2(0,0), Point2(40, 40)).collinear_has_on(Point2(0.0, 0.0)));
	TEST_Run( Segment2(Point2(0,0), Point2(40, 40)).collinear_has_on(Point2(20.0, 20.0)));
	TEST_Run( Segment2(Point2(0,0), Point2(40, 40)).collinear_has_on(Point2(40.0, 40.0)));
	TEST_Run(!Segment2(Point2(0,0), Point2(40, 40)).collinear_has_on(Point2(41.0, 41.0)));

	// Intersect - the "1" segments are left of the "2" segments - all cross at one point.
	Segment2	horz1(Point2(-10, 0), Point2(10, 0));
	Segment2	vert1(Point2(0, -10), Point2(0, 10));
	Segment2	diag1(Point2(-10,-10), Point2(10, 10));
	Segment2	ndig1(Point2(10, -10), Point2(-10, 10));

	Segment2	horz2(Point2(40, 0), Point2(60, 0));
	Segment2	vert2(Point2(50, -10), Point2(50, 10));
	Segment2	diag2(Point2(40,-10), Point2(60, 10));
	Segment2	ndig2(Point2(60, -10), Point2(40, 10));

	Point2	result;
	TEST_Run(!horz1.intersect(horz1, result));
	TEST_Run( horz1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run( horz1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run( horz1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( vert1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run(!vert1.intersect(vert1, result));
	TEST_Run( vert1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run( vert1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( diag1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run( diag1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run(!diag1.intersect(diag1, result));
	TEST_Run( diag1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run(!ndig1.intersect(ndig1, result));

	TEST_Run(!horz1.intersect(horz2, result));
	TEST_Run(!horz1.intersect(vert2, result));
	TEST_Run(!horz1.intersect(diag2, result));
	TEST_Run(!horz1.intersect(ndig2, result));
	TEST_Run(!vert1.intersect(horz2, result));
	TEST_Run(!vert1.intersect(vert2, result));
	TEST_Run(!vert1.intersect(diag2, result));
	TEST_Run(!vert1.intersect(ndig2, result));
	TEST_Run(!diag1.intersect(horz2, result));
	TEST_Run(!diag1.intersect(vert2, result));
	TEST_Run(!diag1.intersect(diag2, result));
	TEST_Run(!diag1.intersect(ndig2, result));
	TEST_Run(!ndig1.intersect(horz2, result));
	TEST_Run(!ndig1.intersect(vert2, result));
	TEST_Run(!ndig1.intersect(diag2, result));
	TEST_Run(!ndig1.intersect(ndig2, result));	
	
}

static void TEST_Line2(void)
{
	// Constructors
	// Construction, copy, equality
	Line2	diag(Point2(0, 0), Point2(3, 3));
	TEST_Run(Line2() != diag);
	TEST_Run(diag == Line2(diag));
	TEST_Run(diag == Line2(Point2(0,0), Vector2(3,3)));
	TEST_Run(diag == Line2(Point2(0,0), Vector2(1,1)));
	TEST_Run(diag == Line2(Point2(0,0), Vector2(-1,-1)));
	TEST_Run(diag == Line2(Point2(1,1), Vector2(-1,-1)));
	diag = Line2(Point2(0,0), Point2(10.0, 4.0));
	TEST_Run(diag != Line2(Point2(0,0), Vector2(3,3)));
	TEST_Run(diag != Line2(Point2(0,0), Vector2(1,1)));
	TEST_Run(diag != Line2(Point2(0,0), Vector2(-1,-1)));
	TEST_Run(diag != Line2(Point2(1,1), Vector2(-1,-1)));
		
	// Intersect
	
	Line2	horz1(Point2(-10, 0), Point2(10, 0));
	Line2	vert1(Point2(0, -10), Point2(0, 10));
	Line2	diag1(Point2(-10,-10), Point2(10, 10));
	Line2	ndig1(Point2(10, -10), Point2(-10, 10));
	Point2	result;
	
	TEST_Run(!horz1.intersect(horz1, result));
	TEST_Run( horz1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run( horz1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run( horz1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( vert1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run(!vert1.intersect(vert1, result));
	TEST_Run( vert1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run( vert1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( diag1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run( diag1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run(!diag1.intersect(diag1, result));
	TEST_Run( diag1.intersect(ndig1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(horz1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(vert1, result) && result == Point2(0,0));
	TEST_Run( ndig1.intersect(diag1, result) && result == Point2(0,0));
	TEST_Run(!ndig1.intersect(ndig1, result));		
	
	Segment2	s1(Point2(0,10), Point2(0,-5));
	Segment2	s2(Point2(10,10), Point2(10,0));
	Line2		l1(s1);
	Line2		l2(s2);
	TEST_Run(l1 != l2);

	TEST_Run(Line2(Point2(0,0), Point2(40, 40)).squared_distance(Point2(-1, 11)) == 72.0);
	
	Line2		l(Point2(0,10), Point2(40, 10));
	l.normalize();
	TEST_Run(l.a == 0.0);
	TEST_Run(l.b == 1.0);
	TEST_Run(l.c == -10.0);

	TEST_Run(l.squared_distance(Point2(29.3249, 5.0)) == 25.0);
}



static void TEST_Bbox2(void)
{
	TEST_Run(Bbox2().is_null());
	TEST_Run(Bbox2() == Bbox2(0, 0, 0, 0));
	TEST_Run(Bbox2() != Bbox2(4, 0, 4, 0));
	TEST_Run(Bbox2(4, 0, 4, 0).is_empty());
	TEST_Run(Bbox2(Point2(3, 3), Point2(6, 6)) == Bbox2(Point2(6, 3), Point2(3, 6)));
	TEST_Run(Bbox2(3, 3, 7, 7) == Bbox2(3, 7, 7, 3));
	TEST_Run(Bbox2(3, 3, 7, 8) != Bbox2(3, 7, 7, 3));
	
	Bbox2	zerofour(0,0,4,4);
	TEST_Run(zerofour == Bbox2(0, 0, 4, 4));
	zerofour += Point2(5, -1);	
	TEST_Run(zerofour == Bbox2(0, -1, 5, 4));
	TEST_Run(zerofour.xmin() == 0.0);
	TEST_Run(zerofour.ymin() == -1.0);
	TEST_Run(zerofour.xmax() ==  5.0);
	TEST_Run(zerofour.ymax() ==  4.0);
	
	TEST_Run(Bbox2(0,0, 4, 4).overlap(Bbox2(2, 2, 6, 6)));
	TEST_Run(Bbox2(0,0, 4, 4).overlap(Bbox2(-2, 2, 2, 6)));
	TEST_Run(Bbox2(0,0, 4, 4).overlap(Bbox2(2, -22, 6, 2)));
	TEST_Run(!Bbox2(0,0, 4, 4).overlap(Bbox2(6, 6, 10, 10)));
	TEST_Run(!Bbox2(0,0, 4, 4).overlap(Bbox2(2, 6, 6, 10)));
}

static void TEST_Polygon2(void)
{
	Polygon2	triangle;
	triangle.push_back(Point2(0, 0));
	triangle.push_back(Point2(-10, 12));
	triangle.push_back(Point2(-20, 0));
	
	TEST_Run(triangle.side(0) == Segment2(Point2(0,0), Point2(-10, 12)));
	TEST_Run(triangle.side(1) == Segment2(Point2(-10,12), Point2(-20, 0)));
	TEST_Run(triangle.side(2) == Segment2(Point2(-20,0), Point2(0, 0)));
	TEST_Run(triangle.centroid() == Point2(-10.0, 4.0));
	TEST_Run(triangle.area() == 120.0);
	
	Polygon2	pentagon;
	pentagon.push_back(Point2(10, 0));
	pentagon.push_back(Point2(13, 10));
	pentagon.push_back(Point2(0, 20));
	pentagon.push_back(Point2(-13, 10));
	pentagon.push_back(Point2(-10, 0));
	
	TEST_Run(pentagon.convex());
	pentagon[2] = Point2(0, 5);
	TEST_Run(!pentagon.convex());
	pentagon[2] = Point2(0, 20);
	
	TEST_Run( pentagon.inside_convex(Point2(9, 9)));
	TEST_Run(!pentagon.inside_convex(Point2(15, 9)));
	TEST_Run(!pentagon.inside_convex(Point2(0, 0)));
	
	Polygon2	letterE;
	letterE.push_back(Point2(0,0));
	letterE.push_back(Point2(50,0));
	letterE.push_back(Point2(50,10));
	letterE.push_back(Point2(20,10));
	letterE.push_back(Point2(20,20));
	letterE.push_back(Point2(50,20));
	letterE.push_back(Point2(50,30));
	letterE.push_back(Point2(20,30));
	letterE.push_back(Point2(20,40));
	letterE.push_back(Point2(50,40));
	letterE.push_back(Point2(50,50));
	letterE.push_back(Point2(0,50));
	
	TEST_Run(letterE.inside(Point2(30,  5)));
	TEST_Run(!letterE.inside(Point2(30, 15)));
	TEST_Run(letterE.inside(Point2(30, 25)));
	TEST_Run(!letterE.inside(Point2(30, 35)));
	TEST_Run(letterE.inside(Point2(30, 45)));

	lesser_y_then_x		lyx;
	greater_y_then_x	gyx;
	
	Point2	x(0,0), a(-1,-1),b(-1,0),c(1,0),
					d(0,1),e(1,1), f(1,0),g(1,-1), h(0, -1);
	TEST_Run(!lyx(x,a));
	TEST_Run(!lyx(x,b));
	TEST_Run( lyx(x,c));
	TEST_Run( lyx(x,d));
	TEST_Run( lyx(x,e));
	TEST_Run( lyx(x,f));
	TEST_Run(!lyx(x,g));
	TEST_Run(!lyx(x,h));
	TEST_Run(!lyx(x,x));

	TEST_Run( gyx(x,a));
	TEST_Run( gyx(x,b));
	TEST_Run(!gyx(x,c));
	TEST_Run(!gyx(x,d));
	TEST_Run(!gyx(x,e));
	TEST_Run(!gyx(x,f));
	TEST_Run( gyx(x,g));
	TEST_Run( gyx(x,h));
	TEST_Run(!gyx(x,x));
	
}


// Polygon
// lesser_y_then_x, greater_y_then_x

void	TEST_CompGeomDefs2(void)
{
	TEST_Point2();
	TEST_Vector2();
	TEST_Segment2();
	TEST_Line2();
	TEST_Bbox2();
	TEST_Polygon2();
}

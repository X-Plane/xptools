// Copyright (c) 2000  Utrecht University (The Netherlands),
// ETH Zurich (Switzerland), Freie Universitaet Berlin (Germany),
// INRIA Sophia-Antipolis (France), Martin-Luther-University Halle-Wittenberg
// (Germany), Max-Planck-Institute Saarbruecken (Germany), RISC Linz (Austria),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: /CVSROOT/CGAL/Packages/Cartesian_kernel/include/CGAL/Cartesian/Line_2.h,v $
// $Revision: 1.35 $ $Date: 2003/10/21 12:14:18 $
// $Name: current_submission $
//
// Author(s)     : Andreas Fabri, Herve Bronnimann

#ifndef CGAL_CARTESIAN_LINE_2_H
#define CGAL_CARTESIAN_LINE_2_H

#include <CGAL/Threetuple.h>

CGAL_BEGIN_NAMESPACE

template < class R_ >
class LineC2
  : public R_::template Handle<Threetuple<typename R_::FT> >::type
{
CGAL_VC7_BUG_PROTECTED
  typedef typename R_::FT                   FT;
  typedef typename R_::Point_2              Point_2;
  typedef typename R_::Direction_2          Direction_2;
  typedef typename R_::Vector_2             Vector_2;
  typedef typename R_::Ray_2                Ray_2;
  typedef typename R_::Segment_2            Segment_2;
  typedef typename R_::Line_2               Line_2;
  typedef typename R_::Aff_transformation_2 Aff_transformation_2;

  typedef Threetuple<FT>	                   rep;
  typedef typename R_::template Handle<rep>::type  base;

public:
  typedef R_                                     R;

  LineC2() {}

  LineC2(const Point_2 &p, const Point_2 &q)
    : base(R().construct_line_2_object()(p, q)) {}

  LineC2(const FT &a, const FT &b, const FT &c)
    : base(rep(a, b, c)) {}

  LineC2(const Segment_2 &s)
    : base(R().construct_line_2_object()(s)) {}

  LineC2(const Ray_2 &r)
    : base(R().construct_line_2_object()(r)) {}

  LineC2(const Point_2 &p, const Direction_2 &d)
    : base(R().construct_line_2_object()(p, d)) {}

  LineC2(const Point_2 &p, const Vector_2 &v)
    : base(R().construct_line_2_object()(p, v)) {}

  bool            operator==(const LineC2 &l) const;
  bool            operator!=(const LineC2 &l) const;

  const FT & a() const
  {
      return base::Ptr()->e0;
  }
  const FT & b() const
  {
      return base::Ptr()->e1;
  }
  const FT & c() const
  {
      return base::Ptr()->e2;
  }

  FT              x_at_y(const FT &y) const;
  FT              y_at_x(const FT &x) const;

  Line_2          perpendicular(const Point_2 &p) const;
  Line_2          opposite() const;
  Point_2         point(int i) const;

  Point_2         point() const;
  Point_2         projection(const Point_2 &p) const;

  Direction_2     direction() const;
  Vector_2        to_vector() const;

  Oriented_side   oriented_side(const Point_2 &p) const;
  bool            has_on_boundary(const Point_2 &p) const;
  bool            has_on_positive_side(const Point_2 &p) const;
  bool            has_on_negative_side(const Point_2 &p) const;
  bool            has_on(const Point_2 &p) const { return has_on_boundary(p); }

  bool            is_horizontal() const;
  bool            is_vertical() const;
  bool            is_degenerate() const;

  Line_2          transform(const Aff_transformation_2 &t) const
  {
    return LineC2<R>(t.transform(point(0)),
                               t.transform(direction()));
  }
};

template < class R >
CGAL_KERNEL_INLINE
bool
LineC2<R>::operator==(const LineC2<R> &l) const
{
  if (identical(l))
      return true;
  return equal_line(*this, l);
}

template < class R >
inline
bool
LineC2<R>::operator!=(const LineC2<R> &l) const
{
  return !(*this == l);
}

template < class R >
inline
bool
LineC2<R>::is_horizontal() const
{ // FIXME : predicate
  return CGAL_NTS is_zero(a());
}

template < class R >
inline
bool
LineC2<R>::is_vertical() const
{ // FIXME : predicate
  return CGAL_NTS is_zero(b());
}

template < class R >
CGAL_KERNEL_INLINE
typename LineC2<R>::FT
LineC2<R>::x_at_y(const typename LineC2<R>::FT &y) const
{
  CGAL_kernel_precondition_msg( ! is_horizontal(),
    "Line::x_at_y(FT y) is undefined for horizontal line");
  return line_x_at_y(*this, y);
}

template < class R >
CGAL_KERNEL_INLINE
typename LineC2<R>::FT
LineC2<R>::y_at_x(const typename LineC2<R>::FT &x) const
{
  CGAL_kernel_precondition_msg( ! is_vertical(),
    "Line::y_at_x(FT x) is undefined for vertical line");
  return line_y_at_x(*this, x);
}

template < class R >
inline
typename LineC2<R>::Line_2
LineC2<R>::
perpendicular(const typename LineC2<R>::Point_2 &p) const
{
  typename R::FT fta, ftb, ftc;
  perpendicular_through_pointC2(a(), b(), p.x(), p.y(), fta, ftb, ftc);
  return Line_2(fta, ftb, ftc);
}

template < class R >
inline
typename LineC2<R>::Line_2
LineC2<R>::opposite() const
{
  return LineC2<R>( -a(), -b(), -c() );
}

template < class R >
CGAL_KERNEL_INLINE
typename LineC2<R>::Point_2
LineC2<R>::point(int i) const
{
  typename R::FT x, y;
  typename R::Construct_point_2 construct_point_2;
  line_get_pointC2(a(), b(), c(), i, x, y);
  return construct_point_2(x,y);
}

template < class R >
CGAL_KERNEL_INLINE
typename LineC2<R>::Point_2
LineC2<R>::point() const
{
  typename R::FT x, y;
  typename R::Construct_point_2 construct_point_2;
  line_get_pointC2(a(), b(), c(), 0, x, y);
  return construct_point_2(x,y);
}

template < class R >
CGAL_KERNEL_MEDIUM_INLINE
typename LineC2<R>::Point_2
LineC2<R>::
projection(const typename LineC2<R>::Point_2 &p) const
{
  typename R::FT x, y;
  typename R::Construct_point_2 construct_point_2;
  line_project_pointC2(a(), b(), c(), p.x(), p.y(), x, y);
  return construct_point_2(x, y);
}

template < class R >
inline
typename LineC2<R>::Direction_2
LineC2<R>::direction() const
{
  return Direction_2( b(), -a() );
}

template < class R >
inline
typename LineC2<R>::Vector_2
LineC2<R>::to_vector() const
{
  return Vector_2( b(), -a() );
}

template < class R >
CGAL_KERNEL_INLINE
Oriented_side
LineC2<R>::
oriented_side(const typename LineC2<R>::Point_2 &p) const
{
  return side_of_oriented_lineC2(a(), b(), c(), p.x(), p.y());
}

template < class R >
inline
bool
LineC2<R>::
has_on_boundary(const typename LineC2<R>::Point_2 &p) const
{
  return oriented_side(p) == ON_ORIENTED_BOUNDARY;
}

template < class R >
inline
bool
LineC2<R>::
has_on_positive_side(const typename LineC2<R>::Point_2 &p) const
{
  return oriented_side(p) == ON_POSITIVE_SIDE;
}

template < class R >
CGAL_KERNEL_INLINE
bool
LineC2<R>::
has_on_negative_side(const typename LineC2<R>::Point_2 &p) const
{
  return oriented_side(p) == ON_NEGATIVE_SIDE;
}

template < class R >
inline
bool
LineC2<R>::is_degenerate() const
{
  return is_horizontal() && is_vertical();
}

#ifndef CGAL_NO_OSTREAM_INSERT_LINEC2
template < class R >
std::ostream &
operator<<(std::ostream &os, const LineC2<R> &l)
{
    switch(os.iword(IO::mode)) {
    case IO::ASCII :
        return os << l.a() << ' ' << l.b() << ' ' << l.c();
    case IO::BINARY :
        write(os, l.a());
        write(os, l.b());
        write(os, l.c());
        return os;
    default:
        return os << "LineC2(" << l.a() 
		  << ", " << l.b() << ", " << l.c() <<')';
    }
}
#endif // CGAL_NO_OSTREAM_INSERT_LINEC2

#ifndef CGAL_NO_ISTREAM_EXTRACT_LINEC2
template < class R >
std::istream &
operator>>(std::istream &is, LineC2<R> &l)
{
    typename R::FT a, b, c;
    switch(is.iword(IO::mode)) {
    case IO::ASCII :
        is >> a >> b >> c;
        break;
    case IO::BINARY :
        read(is, a);
        read(is, b);
        read(is, c);
        break;
    default:
        std::cerr << "" << std::endl;
        std::cerr << "Stream must be in ascii or binary mode" << std::endl;
        break;
    }
    if (is)
	l = LineC2<R>(a, b, c);
    return is;
}
#endif // CGAL_NO_ISTREAM_EXTRACT_LINEC2

CGAL_END_NAMESPACE

#endif // CGAL_CARTESIAN_LINE_2_H

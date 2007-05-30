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
// $Source: /CVSROOT/CGAL/Packages/Cartesian_kernel/include/CGAL/Cartesian/Vector_3.h,v $
// $Revision: 1.30 $ $Date: 2003/10/21 12:14:25 $
// $Name: current_submission $
//
// Author        : Andreas Fabri

#ifndef CGAL_CARTESIAN_VECTOR_3_H
#define CGAL_CARTESIAN_VECTOR_3_H

#include <CGAL/Origin.h>
#include <CGAL/Threetuple.h>

CGAL_BEGIN_NAMESPACE

template < class R_ >
class VectorC3
  : public R_::template Handle<Threetuple<typename R_::FT> >::type
{
CGAL_VC7_BUG_PROTECTED
  typedef typename R_::FT                   FT;
  typedef typename R_::Point_3              Point_3;
  typedef typename R_::Vector_3             Vector_3;
  typedef typename R_::Direction_3          Direction_3;
  typedef typename R_::Aff_transformation_3 Aff_transformation_3;

  typedef Threetuple<FT>                           rep;
  typedef typename R_::template Handle<rep>::type  base;

public:
  typedef R_                                     R;

  VectorC3() {}

  VectorC3(const Null_vector &n)
    : base(R().construct_vector_3_object()(n)) {}

  VectorC3(const Point_3 &p)
    : base(p) {}

  VectorC3(const Point_3 &a, const Point_3 &b)
    : base(R().construct_vector_3_object()(a, b)) {}

  VectorC3(const Direction_3 &d)
    : base(d) {}

  VectorC3(const FT &x, const FT &y, const FT &z)
    : base(rep(x, y, z)) {}

  VectorC3(const FT &x, const FT &y, const FT &z, const FT &w)
  {
    if (w != FT(1))
      initialize_with(rep(x/w, y/w, z/w));
    else
      initialize_with(rep(x, y, z));
  }

  const FT & x() const
  {
      return base::Ptr()->e0;
  }
  const FT & y() const
  {
      return base::Ptr()->e1;
  }
  const FT & z() const
  {
      return base::Ptr()->e2;
  }

  const FT & hx() const
  {
      return x();
  }
  const FT & hy() const
  {
      return y();
  }
  const FT & hz() const
  {
      return z();
  }
  FT hw() const
  {
      return FT(1);
  }

  const FT & cartesian(int i) const;
  const FT & operator[](int i) const;
  FT homogeneous(int i) const;

  int dimension() const
  {
      return 3;
  }

  Vector_3 operator+(const VectorC3 &w) const;
  Vector_3 operator-(const VectorC3 &w) const;
  Vector_3 operator-() const;
  Vector_3 operator/(const FT &c) const;
  FT operator*(const VectorC3 &w) const;
  FT squared_length() const;
  Direction_3 direction() const;
  Vector_3 transform(const Aff_transformation_3 &t) const
  {
    return t.transform(*this);
  }
};

template < class R >
inline
bool
operator==(const VectorC3<R> &v, const VectorC3<R> &w)
{
  return w.x() == v.x() && w.y() == v.y() && w.z() == v.z();
}

template < class R >
inline
bool
operator!=(const VectorC3<R> &v, const VectorC3<R> &w)
{
  return !(v == w);
}

template < class R >
inline
bool
operator==(const VectorC3<R> &v, const Null_vector &) 
{
  return CGAL_NTS is_zero(v.x()) && CGAL_NTS is_zero(v.y()) &&
         CGAL_NTS is_zero(v.z());
}

template < class R >
inline
bool
operator==(const Null_vector &n, const VectorC3<R> &v) 
{
  return v == n;
}

template < class R >
inline
bool
operator!=(const VectorC3<R> &v, const Null_vector &n)
{
  return !(v == n);
}

template < class R >
inline
bool
operator!=(const Null_vector &n, const VectorC3<R> &v)
{
  return !(v == n);
}

template < class R >
inline
const typename VectorC3<R>::FT &
VectorC3<R>::cartesian(int i) const
{
  CGAL_kernel_precondition( (i>=0) && (i<3) );
  if (i==0) return x();
  if (i==1) return y();
  return z();
}

template < class R >
inline
const typename VectorC3<R>::FT &
VectorC3<R>::operator[](int i) const
{
  return cartesian(i);
}

template < class R >
typename VectorC3<R>::FT
VectorC3<R>::homogeneous(int i) const
{
  if (i==3) return FT(1);
  return cartesian(i);
}

template < class R >
inline
typename VectorC3<R>::Vector_3
VectorC3<R>::
operator+(const VectorC3<R> &w) const
{
  return VectorC3<R>(x() + w.x(), y() + w.y(), z() + w.z());
}

template < class R >
inline
typename VectorC3<R>::Vector_3
VectorC3<R>::operator-(const VectorC3<R> &w) const
{
  return VectorC3<R>(x() - w.x(), y() - w.y(), z() - w.z());
}

template < class R >
inline
typename VectorC3<R>::Vector_3
VectorC3<R>::operator-() const
{
  return R().construct_opposite_vector_3_object()(*this);
}

template < class R >
inline
typename VectorC3<R>::FT
VectorC3<R>::operator*(const VectorC3<R> &w) const
{
  return x() * w.x() + y() * w.y() + z() * w.z();
}

template < class R >
inline
typename VectorC3<R>::FT
VectorC3<R>::squared_length() const
{
  return CGAL_NTS square(x()) + CGAL_NTS square(y()) + CGAL_NTS square(z());
}

template < class R >
inline
typename VectorC3<R>::Vector_3
VectorC3<R>::
operator/(const typename VectorC3<R>::FT &c) const
{
  return VectorC3<R>(x()/c, y()/c, z()/c);
}

template < class R >
inline
typename VectorC3<R>::Direction_3
VectorC3<R>::direction() const
{
  return Direction_3(*this);
}

#ifndef CGAL_CARTESIAN_NO_OSTREAM_INSERT_VECTORC3
template < class R >
std::ostream &
operator<<(std::ostream &os, const VectorC3<R> &v)
{
  switch(os.iword(IO::mode)) {
    case IO::ASCII :
      return os << v.x() << ' ' << v.y()  << ' ' << v.z();
    case IO::BINARY :
      write(os, v.x());
      write(os, v.y());
      write(os, v.z());
      return os;
    default:
      os << "VectorC3(" << v.x() << ", " << v.y() <<  ", " << v.z() << ")";
      return os;
  }
}
#endif // CGAL_CARTESIAN_NO_OSTREAM_INSERT_VECTORC3

#ifndef CGAL_CARTESIAN_NO_ISTREAM_EXTRACT_VECTORC3
template < class R >
std::istream &
operator>>(std::istream &is, VectorC3<R> &p)
{
  typename R::FT x, y, z;
  switch(is.iword(IO::mode)) {
    case IO::ASCII :
      is >> x >> y >> z;
      break;
    case IO::BINARY :
      read(is, x);
      read(is, y);
      read(is, z);
      break;
    default:
      std::cerr << "" << std::endl;
      std::cerr << "Stream must be in ascii or binary mode" << std::endl;
      break;
  }
  if (is)
      p = VectorC3<R>(x, y, z);
  return is;
}
#endif // CGAL_CARTESIAN_NO_ISTREAM_EXTRACT_VECTORC3

CGAL_END_NAMESPACE

#endif // CGAL_CARTESIAN_VECTOR_3_H

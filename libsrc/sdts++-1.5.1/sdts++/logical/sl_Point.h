//
// This file is part of the SDTS++ toolkit, written by the U.S.
// Geological Survey.  It is experimental software, written to support
// USGS research and cartographic data production.
// 
// SDTS++ is public domain software.  It may be freely copied,
// distributed, and modified.  The USGS welcomes user feedback, but makes
// no committment to any level of support for this code.  See the SDTS
// web site at http://mcmcweb.er.usgs.gov/sdts for more information,
// including points of contact.
//
// $Id: sl_Point.h,v 1.5 2000/07/25 20:05:42 mcoletti Exp $

#ifndef INCLUDED_SL_POINT_H
#define INCLUDED_SL_POINT_H

#ifndef INCLUDED_SL_OBJECT_H
#include <sdts++/logical/sl_Object.h>
#endif

#ifndef INCLUDED_SB_SPATIAL_H
#include <sdts++/builder/sb_Spatial.h>
#endif


/**
 This represents an SDTS logical zero-dimensional point.  This
 serves as a base class for the other point types, sl_EntityPoint,
 sl_AreaPoint, sl_LabelPoint, and sl_Node.
*/
class sl_Point : public sl_Object
{
   public:

      ///
      virtual ~sl_Point() {}

      ///
      sb_Spatial const & coordinate() const { return coordinate_; }

      ///
      sb_Spatial       & coordinate()       { return coordinate_; }

   protected:

      /**
       returns the point object code; however, the sl_Point
       sub-classes are free to redefine this for their respective
       object representation codes.
      */
      virtual std::string const & objectCode() const;


   private:

      /// coordinate in IREF spatial coordinate values
      sb_Spatial coordinate_;

}; // class sl_Point



/// This represents an area point
class sl_AreaPoint : public sl_Point
{
   protected:

      /// returns the point object code
      virtual std::string const & objectCode() const;

}; // class sl_AreaPoint




/// This represents a label point
class sl_LabelPoint : public sl_Point
{
   protected:

      /// returns the point object code
      virtual std::string const & objectCode() const;

}; // class sl_LabelPoint





/// This represents an entity point
class sl_EntityPoint : public sl_Point
{
   protected:

      /// returns the point object code
      virtual std::string const & objectCode() const;

}; // class sl_EntityPoint


#endif

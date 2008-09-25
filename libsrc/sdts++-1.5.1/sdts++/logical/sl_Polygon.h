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
// $Id: sl_Polygon.h,v 1.2 2000/07/25 20:05:42 mcoletti Exp $

#ifndef INCLUDED_SL_POLYGON_H
#define INCLUDED_SL_POLYGON_H

#ifndef INCLUDED_SL_CHAIN_H
#include <sdts++/logical/sl_Chain.h>
#endif


#include <deque>




/// This represents SDTS logical polygons.
class sl_Polygon : public sl_Object
{
   public:

      ///
      virtual ~sl_Polygon() {}

      ///
      sl_Chains const & lines() const { return lines_; }

      ///
      sl_Chains       & lines()       { return lines_; }

   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const = 0;

   private:

      /// a polygon is, after all, composed of a set of lines
      /**
        XXX policy enforcement problem: how to insure that the right chains
        XXX are matched up with the sub-classes?  The template mechanism
       XXX used by rings is too complex.  Maybe an intermediate chain type
       XXX suitable only for use in GT polygons needs to be created?
      */
     sl_Chains lines_;

}; // class sl_Polygon



///
class sl_GPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_GPolygon() {}


   protected:

      /// returns the polygon object code
     virtual std::string const & objectCode() const;

}; // class sl_GPolygon



///
class sl_GTChainPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_GTChainPolygon() {}

   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;

}; // class sl_GTChainPolygon




///
class sl_GTRingPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_GTRingPolygon() {}

   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;

}; // class sl_GTRingPolygon



///
class sl_UniverseChainPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_UniverseChainPolygon() {}


   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;


}; // class sl_UniverseChainPolygon




///
class sl_UniverseRingPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_UniverseRingPolygon() {}

   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;


}; // class sl_UniverseRingPolygon




///
class sl_VoidChainPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_VoidChainPolygon() {}


   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;


}; // class sl_VoidChainPolygon




///
class sl_VoidRingPolygon : public sl_Polygon
{
   public:

      ///
      virtual ~sl_VoidRingPolygon() {}


   protected:

      /// returns the polygon object code
      virtual std::string const & objectCode() const;


}; // class sl_VoidRingPolygon

#endif

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
// $Id: sl_Node.h,v 1.2 2000/07/25 20:05:42 mcoletti Exp $

#ifndef INCLUDED_SL_NODE_H
#define INCLUDED_SL_NODE_H

#ifndef INCLUDED_SL_POINT_H
#include <sdts++/logical/sl_Point.h>
#endif


/**
 This abstract base class represents an SDTS logical
 zero-dimensional node.  This serves as a base class for the other
 node types, sl_PlanarNode and sl_NetworkNode.
*/
class sl_Node : public sl_Point
{
   public:

      ///
      virtual ~sl_Node() {}

   protected:

      /**
       returns the node object code; the sl_Node sub-classes
       redefine this for their respective object representation
       codes.
      */
      virtual std::string const & objectCode() const = 0;

}; // class sl_Point



/// This represents an planar graph node
class sl_PlanarNode : public sl_Node
{
   protected:

      /// returns the node object code
      virtual std::string const & objectCode() const;

}; // class sl_Planar Node



/// This represents a network graph node
class sl_NetworkNode : public sl_Node
{
   protected:

      /// returns the node object code
      virtual std::string const & objectCode() const;

}; // class sl_Network Node




#endif

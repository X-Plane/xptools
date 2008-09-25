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
// $Id: sl_Chain.h,v 1.6 2003/06/10 17:49:32 mcoletti Exp $

#ifndef INCLUDED_SL_CHAIN_H
#define INCLUDED_SL_CHAIN_H

#ifndef INCLUDED_SL_OBJECT_H
#include <sdts++/logical/sl_Object.h>
#endif

#ifndef INCLUDED_SB_SPATIAL_H
#include <sdts++/builder/sb_Spatial.h>
#endif

#include <boost/smart_ptr.hpp>

#include <deque>


class sl_Node;
class sl_Polygon;


/// Defines the type for a container of logical layer points.
/**
 Why a deque and not, say, a std::list or std::vector?  Given that
 the most common type of object will be points, parsimonous memory
 and processing container implementations are naturally preferred.
 Lists incur memory overhead penalties for additional inter-node
 storage.  Vectors incur run-time overhead for multiple push_back()
 calls; i.e., they must occasionally be realloc()'d and the old
 contents copied to the new block of memory.  Deques are a useful
 compromise in that they don't consume much more memory overhead
 than vectors, and are optimized for push_back() calls in that large
 blocks are allocated at once.
*/
typedef std::deque<sb_Spatial> sl_Points;



/// This abstract base class represents SDTS logical chains
class sl_Chain : public sl_Object
{
   public:

      ///
      virtual ~sl_Chain() {}

      ///
      sl_Points const & points() const { return points_; }

      ///
      sl_Points       & points()       { return points_; }

   protected:

      /// returns the chain object code
      virtual std::string const & objectCode() const = 0;


   private:

      /**
       regardless of whether a given chain type has left or right
       polygons or start and end nodes, they _all_ can have
       intermediate points
      */
      sl_Points points_;

}; // class sl_Chain


typedef boost::shared_ptr<sl_Chain> sl_ChainPtr;
// reference counted smart pointer for heterogenous chain containers


typedef std::deque<sl_ChainPtr> sl_Chains;
// type for a heterogenous chain container





// This class represents SDTS complete chains
//
class sl_CompleteChain : public sl_Chain
{
   public:

      sl_CompleteChain()
         : left_( 0 ),
           right_( 0 ),
           start_( 0 ),
           end_( 0 )
      {}

      sl_CompleteChain( sl_Node * start, sl_Node * end,
                        sl_Polygon * left, sl_Polygon * right )
         : left_( left ),
           right_( right ),
           start_( start ),
           end_( end )
      {}

      virtual ~sl_CompleteChain() {}


      sl_Node const * const start() const { return start_; }
      sl_Node       *&      start()       { return start_; }
      // start node

      sl_Node const * const end() const { return end_; }
      sl_Node       *&      end()       { return end_; }
      // end node

      sl_Polygon const * const left() const { return left_; }
      sl_Polygon       *&      left()       { return left_; }
      // left polygon

      sl_Polygon const * const right() const { return right_; }
      sl_Polygon       *&      right()       { return right_; }
      // right polygon

   protected:

      virtual std::string const & objectCode() const;
      // returns the chain object code

   private:

      sl_Polygon * left_;
      // left polygon

      sl_Polygon * right_;
      // right polygon

      sl_Node * start_;
      // start node

      sl_Node * end_;
      // end node

}; // class sl_CompleteChain




// This class represents SDTS area chains
//
class sl_AreaChain : public sl_Chain
{
   public:

      sl_AreaChain()
         : left_( 0 ),
           right_( 0 )
      {}

      sl_AreaChain( sl_Polygon * left, sl_Polygon * right )
         : left_( left ),
           right_( right )
      {}

      virtual ~sl_AreaChain() {}

      sl_Polygon const * const left() const { return left_; }
      sl_Polygon       *&      left()       { return left_; }
      // left polygon

      sl_Polygon const * const right() const { return right_; }
      sl_Polygon       *&      right()       { return right_; }
      // right polygon

   protected:

      virtual std::string const & objectCode() const;
      // returns the chain object code

   private:

      sl_Polygon * left_;
      // left polygon

      sl_Polygon * right_;
      // right polygon

}; // class sl_AreaChain





// This abstract base class represents SDTS complete network chains.
//
class sl_NetworkChain : public sl_Chain
{
   public:

      sl_NetworkChain()
         : start_( 0 ),
           end_( 0 )
      {}

      sl_NetworkChain( sl_Node * start, sl_Node * end )
         : start_( start ),
           end_( end )
      {}

      virtual ~sl_NetworkChain() {}


      sl_Node const * const start() const { return start_; }
      sl_Node       *&      start()       { return start_; }
      // start node

      sl_Node const * const end() const { return end_; }
      sl_Node       *&      end()       { return end_; }
      // end node


   protected:

      virtual std::string const & objectCode() const = 0;
      // returns the chain object code

   private:

      sl_Node * start_;
      // start node

      sl_Node * end_;
      // end node

}; // class sl_NetworkChain



// This represents a planar graph network chain
//
class sl_PlanarNetworkChain : public sl_NetworkChain
{
   protected:

      virtual std::string const & objectCode() const;
      // returns the object code

}; // class sl_PlanarNetworkChain



// This represents a planar graph network chain
//
class sl_NonPlanarNetworkChain : public sl_NetworkChain
{
   protected:

      virtual std::string const & objectCode() const;
      // returns the object code

}; // class sl_NonPlanarNetworkChain




#endif

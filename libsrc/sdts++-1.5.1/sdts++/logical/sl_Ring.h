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
// $Id: sl_Ring.h,v 1.2 2000/07/25 20:05:42 mcoletti Exp $

#ifndef INCLUDED_SL_RING_H
#define INCLUDED_SL_RING_H

#ifndef INCLUDED_SL_CHAIN_H
#include <sdts++/logical/sl_Chain.h>
#endif


///
const std::string GTRingObjectCode = "RU";

///
struct sl_GTRingObjectCode
{
      ///
      static std::string const & code() { return GTRingObjectCode; }
}; // struct sl_GTRingObjectCode



/// This represents SDTS logical rings.
/**
 Since rings merely differ by how they store their lines and their
 object codes, this is implemented as a templated class.  The first
 parameter specifies the line container implementation, the second
 declares what the object code will be.  The latter interface
 requirement be that it defines a ::code() member that returns a
 const std::string reference to the object code.
*/
template<class LineType, class sl_ObjectCode>
class sl_Ring : public sl_Object
{
   public:

      ///
      virtual ~sl_Ring() {}

      ///
      sl_Points const & points() const { return line_.points(); }

      ///
      sl_Points       & points()       { return line_.points(); }

   protected:

      /// returns the ring object code
      virtual std::string const & objectCode() const
      { return sl_ObjectCode::code(); }


   private:

      ///
      LineType line_;

}; // class sl_Ring


/// Defines a GT-Ring that uses complete chains
typedef sl_Ring<sl_CompleteChain,sl_GTRingObjectCode> sl_CompleteChainRing;


/// Defines a GT-Ring that uses area chains
typedef sl_Ring<sl_AreaChain, sl_GTRingObjectCode>    sl_AreaChainRing;


#endif

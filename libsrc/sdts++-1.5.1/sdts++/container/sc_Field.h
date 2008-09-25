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
// sc_Field.h: interface for the sc_Field class.
//
// $Id: sc_Field.h,v 1.9 2003/01/03 20:41:05 mcoletti Exp $
//

#ifndef INCLUDED_SC_FIELD_H
#define INCLUDED_SC_FIELD_H


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <list>


#ifndef INCLUDED_SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif


/// Canonical container type for subfields.
typedef std::list<sc_Subfield> sc_SubfieldCntr;


/**
  SDTS logical field
*/
class sc_Field : public sc_SubfieldCntr
{
   public:

      ///
      typedef sc_SubfieldCntr::iterator iterator;

      ///
      typedef sc_SubfieldCntr::const_iterator const_iterator;


      /// default ctor needed for STL
      sc_Field() {}

      ///
      sc_Field( std::string const& name, std::string const& mnemonic );

      /// Returns the SDTS Name of this field (if one has been set).
      std::string const& getName() const;
      std::string const& name() const { return getName(); }

      /// Returns the SDTS Mnemonic of this field (if one has been set).
      std::string const& getMnemonic() const;
      std::string const& mnemonic() const { return getMnemonic(); }

      ///
      std::string const& setName( std::string const& name );
      std::string const& name( std::string const& name )
      { return setName( name ); }

      ///
      std::string const& setMnemonic( std::string const& mnemonic );
      std::string const& mnemonic( std::string const& mnemonic )
      { return setMnemonic( mnemonic ); }

   private:

      /// SDTS Field Name
      std::string name_;

      /// SDTS Field Mnemonic
      std::string mnemonic_;

      friend std::ostream& operator<<( std::ostream&, sc_Field const& );

}; // class sc_Field

#endif

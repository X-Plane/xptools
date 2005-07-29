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
// sc_Field.cpp: implementation of the sc_Field class.
//


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <sdts++/container/sc_Field.h>

#include <iterator>
#include <algorithm>


using namespace std;


static const char* ident_ = "$Id: sc_Field.cpp,v 1.5 2002/11/24 22:07:43 mcoletti Exp $";


sc_Field::sc_Field( string const& name, string const& mnemonic )
  : mnemonic_( mnemonic ), name_( name )
{
}


string const&
sc_Field::getName() const
{
   return name_;
}


string const&
sc_Field::getMnemonic() const
{
   return mnemonic_;
}


string const&
sc_Field::setName(string const& name)
{
   return name_ = name;
}


string const&
sc_Field::setMnemonic(string const& mnem)
{
   return mnemonic_ = mnem;
}


ostream&
operator<<( ostream& os, sc_Field const& field )
{
  os << field.getMnemonic() << " : " << field.getName() << "\n";

  ostream_iterator<sc_Subfield> os_itr( os, "\n" );

  copy( field.begin(), field.end(), os_itr );

  return os;
}

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
//
// sio_8211SubfieldFormat.cpp
//


#include <sdts++/io/sio_8211SubfieldFormat.h>

#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif

#include <iostream>

using namespace std;


static const char* ident_ = "$Id: sio_8211SubfieldFormat.cpp,v 1.5 2002/10/07 20:44:24 mcoletti Exp $";



// This contains the implementation details for
// sio_8211SubfieldFormat.

struct sio_8211SubfieldFormatImp
{
  sio_8211SubfieldFormatImp() :
    label_(""),
    type_(sio_8211SubfieldFormat::A),
    format_(sio_8211SubfieldFormat::variable),
    control_(sio_8211UnitTerminator),
    converter_(0) {}

  ~sio_8211SubfieldFormatImp()
    {
      converter_ = 0;
    }

  // destructor, assignment operator, and copy ctor not
  // implemented

  string label_;	// a.k.a. subfield mnemonic

  sio_8211SubfieldFormat::type type_;

  sio_8211SubfieldFormat::format format_;

  sio_8211Converter const * converter_;

  // XXX Since a subfield format is either going to be
  // XXX character delimited or read by length, there's no
  // XXX reason to store the character delimeter and length
  // XXX separately.  The union effects this and conveys
  // XXX this mutual exclusive nature.
  union control
  {
    char delimiter_;
    int  length_;

    explicit control( char delimiter ) : delimiter_( delimiter ) {}
    explicit control( int length ) : length_( length ) {}
  } control_;

}; // struct sio_8211SubfieldFormatImp






sio_8211SubfieldFormat::sio_8211SubfieldFormat()
  :	imp_( new sio_8211SubfieldFormatImp )
{

} // sio_8211SubfieldFormat ctor



sio_8211SubfieldFormat::sio_8211SubfieldFormat( sio_8211SubfieldFormat const & sf )
  :	imp_( new sio_8211SubfieldFormatImp(*sf.imp_) )
{

} // sio_8211SubfieldFormat copy ctor



sio_8211SubfieldFormat::~sio_8211SubfieldFormat()
{
  if ( imp_ ) { delete imp_; }
} // sio_8211SubfieldFormat dtor



sio_8211SubfieldFormat&
sio_8211SubfieldFormat::operator=( sio_8211SubfieldFormat const & rhs )
{
  if ( &rhs != this )
    {
      *imp_ = *rhs.imp_;
    }

  return *this;
} // sio_8211SubfieldFormat::operator=




string const &
sio_8211SubfieldFormat::getLabel() const
{
  return imp_->label_;
} // sio_8211SubfieldFormat::getLabel() const




sio_8211SubfieldFormat::type
sio_8211SubfieldFormat::getType() const
{
  return imp_->type_;
} // sio_8211SubfieldFormat::getType() const




sio_8211SubfieldFormat::format
sio_8211SubfieldFormat::getFormat() const
{
  return imp_->format_;
} // sio_8211SubfieldFormat::getFormat() const



int
sio_8211SubfieldFormat::getLength() const
{
  return imp_->control_.length_;
} // sio_8211SubfieldFormat::getLabel() const



char
sio_8211SubfieldFormat::getDelimiter() const
{
  return imp_->control_.delimiter_;
} // sio_8211SubfieldFormat::getDelimiter() const



sio_8211Converter const *
sio_8211SubfieldFormat::getConverter() const
{
  return imp_->converter_;
} // sio_8211SubfieldFormat::getConverter() const










void
sio_8211SubfieldFormat::setLabel( string const & label )
{
  imp_->label_ = label;
} // sio_8211SubfieldFormat::setLabel()




void
sio_8211SubfieldFormat::setType( sio_8211SubfieldFormat::type t )
{
  imp_->type_ = t;

#ifdef BOGUS
  // if the subfield type is binary, its length will be in bits__ and
  // not in characters.  If so, convert to character units to be
  // consistent with the other subfields.
  if ( imp_->control_._length > 0 &&
       imp_->type_ == sio_8211SubfieldFormat::B )
    {
      imp_->control_._length /= 8;
    }
#endif

} // sio_8211SubfieldFormat::setType()




void
sio_8211SubfieldFormat::setFormat( sio_8211SubfieldFormat::format f )
{
  imp_->format_ = f;
} // sio_8211SubfieldFormat::setFormat()



void
sio_8211SubfieldFormat::setLength( int val )
{

  imp_->control_.length_ = val;
  imp_->format_ = sio_8211SubfieldFormat::fixed;
} // sio_8211SubfieldFormat::setLabel()



void
sio_8211SubfieldFormat::setDelimiter( char val )
{
  imp_->control_.delimiter_ = val;
  imp_->format_ = sio_8211SubfieldFormat::variable;
} // sio_8211SubfieldFormat::setDelimiter()



void
sio_8211SubfieldFormat::setConverter( sio_8211Converter const * converter )
{
  imp_->converter_ = converter;
} // sio_8211SubfieldFormat::setConverter()




ostream&
operator<<( ostream& os, sio_8211SubfieldFormat const & sff )
{
  os << "subfield format: ("
     << sff.getLabel() << ",";

  switch ( sff.getType() )
    {
    case sio_8211SubfieldFormat::A :
      os << "A";
      break;
    case sio_8211SubfieldFormat::I :
      os << "I";
      break;
    case sio_8211SubfieldFormat::R :
      os << "R";
      break;
    case sio_8211SubfieldFormat::S :
      os << "S";
      break;
    case sio_8211SubfieldFormat::C :
      os << "C";
      break;
    case sio_8211SubfieldFormat::B :
      os << "B";
      break;
    case sio_8211SubfieldFormat::X :
      os << "X";
      break;
    default :
      os << "?";
      break;
    }

  os << ",";

  switch ( sff.getFormat() )
    {
    case sio_8211SubfieldFormat::fixed :
      os << "fixed," << sff.getLength();
      break;
    case sio_8211SubfieldFormat::variable :
      os << "variable,[" << hex << static_cast<int>(sff.getDelimiter()) << "]" ;
      break;
    default :
      os << "???";
      break;
    }

  os << "," << hex << sff.getConverter() << ")" << dec;

  return os;
} // operator<<()

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

// $Id: sc_MultiTypeValue.cpp,v 1.5 2002/11/24 22:07:43 mcoletti Exp $


#include <sdts++/container/sc_MultiTypeValue.h>


using namespace std;


sc_MultiTypeValue::sc_MultiTypeValue()
  : type_( isNull )
{
  val_.lval = 0;
}

sc_MultiTypeValue::sc_MultiTypeValue(sc_MultiTypeValue const& right)
  : type_( right.type_ )
{
 switch( right.type_ )
    {
    case isLong :
      val_.lval = right.val_.lval;
      break;
    case isUnsignedLong :
      val_.ulval = right.val_.ulval;
      break;
    case isDouble :
      val_.dval = right.val_.dval;
      break;
    case isString :
      val_.sval = new string(*(right.val_.sval));
      break;
    case isNull: // NOP
      break;
    }
}

sc_MultiTypeValue::~sc_MultiTypeValue()
{
  reset();
}


//
// This does what you'd expect.
//
inline
string
clipTrailingSpaces( string const & str )
{
  string tmp_str( str );

  string::size_type pos = tmp_str.find_last_not_of( " " );

  if ( string::npos != pos ) { tmp_str.resize( pos + 1 ); }

  return tmp_str;
} // clipTrailingSpaces




sc_MultiTypeValue const&
sc_MultiTypeValue::operator=(sc_MultiTypeValue const& right)
{
  if (this == &right)
    return *this;

  reset();
 
 type_ = right.type_;
 
 switch(type_)
    {
    case isLong :
      val_.lval = right.val_.lval;
      break;
    case isUnsignedLong :
      val_.ulval = right.val_.ulval;
      break;
    case isDouble :
      val_.dval = right.val_.dval;
      break;
    case isString :
      val_.sval = new string(*(right.val_.sval));
      break;
    case isNull: // NOP
      break;
    }
  return *this;
}

bool
sc_MultiTypeValue::operator==(sc_MultiTypeValue const& right) const
{
  if ( this == &right )
    return true;

  if ( type_ != right.type_ )
    return false;

  switch( type_ )
    {
    case(isLong):
      return val_.lval == right.val_.lval;
    case(isUnsignedLong):
      return val_.ulval == right.val_.ulval;
    case(isDouble):
      return val_.dval == right.val_.dval;
    case(isString):
    {  // I had to do this because string::operator==() wasn't working
       // right for some weird reason.  This is semantically identical, though.

       // Actually, I just changed it AGAIN.  This is because some
       // morons decided to generate SDTS datasets that WASTE A LOT OF
       // SPACE because they were too stupid to use variable length
       // strings so all the string subfields contain a bazillion
       // trailing blanks.  Thus, this makes string comparisons dodgy.
       // Ergo, I must strip any trailing spaces and THEN do the
       // string comparison.

       return clipTrailingSpaces( *val_.sval ) == 
              clipTrailingSpaces( *right.val_.sval );
    }
    case isNull:                // two nulls are always equal
      return true;
    }

  return false;

} // usgs/libsdts++/sdts++/builder/




sc_MultiTypeValue::ValueType
sc_MultiTypeValue::getValueType() const
{
  return type_;
}



bool
sc_MultiTypeValue::getLong(long& val) const
{
  if (type_ != isLong)
  {
    if ( type_ == isUnsignedLong )
    {
      val = static_cast<long>(val_.ulval);
    }
    return false;
  }

  val = val_.lval;
  return true;
}

bool
sc_MultiTypeValue::getUnsignedLong(unsigned long& val) const
{
  if (type_ != isUnsignedLong)
  {
    if ( type_ == isLong )
    {
       val = static_cast<unsigned long>(val_.lval);
       return true;
    }
    return false;
  }

  val = val_.ulval;
  return true;
}

bool
sc_MultiTypeValue::getDouble(double& val) const
{
  if (type_ != isDouble)
    return false;

  val = val_.dval;
  return true;
}

bool
sc_MultiTypeValue::getString(string& val) const
{
  if (type_ != isString)
    return false;

  val = *(val_.sval);
  return true;
}

void
sc_MultiTypeValue::setLong(long val)
{
  if (type_ != isLong)
    {
      reset();
      type_ = isLong;
    }
  val_.lval = val;
}

void
sc_MultiTypeValue::setUnsignedLong(unsigned long val)
{
  if (type_ != isUnsignedLong)
    {
      reset();
      type_ = isUnsignedLong;
    }
  val_.ulval = val;
}

void
sc_MultiTypeValue::setDouble(double val)
{
  if (type_ != isDouble)
    {
      reset();
      type_ = isDouble;
    }
  val_.dval = val;
}

void
sc_MultiTypeValue::setString(string const& val)
{
  if (type_ != isString)
    {
      reset();
      type_ = isString;
      val_.sval = new string( val );
    }
  else
  {
    *val_.sval = val;
  }
}

void
sc_MultiTypeValue::reset()
{
  // Delete any dynamically allocated value
  if ( isString == type_ )
    {
      delete val_.sval;
    }

  type_     = isNull;
  val_.lval = 0;
}


void
sc_MultiTypeValue::setNull()
{
  reset();
} // sc_MultiTypeValue::setNull()

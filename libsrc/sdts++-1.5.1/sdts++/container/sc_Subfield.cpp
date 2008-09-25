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
// $Id: sc_Subfield.cpp,v 1.8 2002/11/24 22:07:43 mcoletti Exp $
//

//#ifndef INCLUDED_CONVERSIONS_HXX
//#include "container/conversions.h"
//#endif

#include <sdts++/container/sc_Subfield.h>


static const char* const iden_ = "$Id: sc_Subfield.cpp,v 1.8 2002/11/24 22:07:43 mcoletti Exp $";



using namespace std;



sc_Subfield::sc_Subfield()
  : type_( is_I )
{
}



sc_Subfield::sc_Subfield( string const & name, string const & mnemonic )
  : name_( name ),
    mnemonic_( mnemonic ),
    type_( is_I )
{
}



sc_Subfield::sc_Subfield(sc_Subfield const& right)
  : name_( right.name_ ),
    mnemonic_( right.mnemonic_ ),
    type_( right.type_ ),
    value_( right.value_ )
{
}



sc_Subfield::~sc_Subfield()
{
}



sc_Subfield& sc_Subfield::operator=(const sc_Subfield& right)
{
  if (this == &right)
    return *this;

  name_     = right.name_;
  mnemonic_ = right.mnemonic_;
  type_     = right.type_;
  value_    = right.value_;

  return *this;
}



bool
sc_Subfield::operator==(const sc_Subfield &right) const
{
  return (mnemonic_ == right.mnemonic_) &&
    (name_     == right.name_) &&
    (type_     == right.type_) &&
    (value_    == right.value_);
}



bool
sc_Subfield::operator!=(sc_Subfield const& right) const
{
  return ! this->operator==(right);
}



sc_Subfield::SubfieldType
sc_Subfield::getSubfieldType() const
{
  return type_;
}



string const&
sc_Subfield::getName() const
{
  return name_;
}



string const&
sc_Subfield::getMnemonic() const
{
  return mnemonic_;
}



bool
sc_Subfield::getA(string& val) const
{
  if (type_ == is_A && ! value_.null() )
    return value_.getString(val);
  return false;
}



bool
sc_Subfield::getI(long& val) const
{
  if ( type_ == is_I  && ! value_.null() )
    return value_.getLong(val);
  return false;
}



bool
sc_Subfield::getR(double& val) const
{
  if (type_ == is_R && ! value_.null() )
    return value_.getDouble(val);
  return false;
}



bool
sc_Subfield::getS(double& val) const
{
  if (type_ == is_S && ! value_.null() )
    return value_.getDouble(val);
  return false;
}



bool
sc_Subfield::getC(string& val) const
{
  if (type_ == is_C && ! value_.null() )
    return value_.getString(val);
  return false;
}



bool
sc_Subfield::getBI8(long& val) const
{
  if ( is_BI8 == type_ )
    {
      return value_.getLong(val);
    }
  return false;
} // sc_Subfield::getBI8



bool
sc_Subfield::getBI16(long& val) const
{
  if ( is_BI16 == type_ )
    {
      return value_.getLong(val);
    }
  return false;
}



bool
sc_Subfield::getBI24(long& val) const
{
  if ( is_BI24 == type_ )
    {
      return value_.getLong(val);
    }
  return false;
}



bool
sc_Subfield::getBI32(long& val) const
{
  if ( is_BI32 == type_ )
    {
      return value_.getLong(val);
    }
  return false;
}



bool
sc_Subfield::getBUI8(unsigned long& val) const
{
  if ( is_BUI8 == type_ )
    {
      return value_.getUnsignedLong(val);
    }
  return false;
}




bool
sc_Subfield::getBUI16(unsigned long& val) const
{
  if ( is_BUI16 == type_ )
    {
      return value_.getUnsignedLong(val);
    }
  return false;
}



bool
sc_Subfield::getBUI24(unsigned long& val) const
{
  if ( is_BUI24 == type_ )
    {
      return value_.getUnsignedLong(val);
    }
  return false;
}




bool
sc_Subfield::getBUI32(unsigned long& val) const
{
  if ( is_BUI32 == type_ )
    {
      return value_.getUnsignedLong(val);
    }
  return false;
}



bool
sc_Subfield::getBFP32(float& val) const
{
  if ( type_ == is_BFP32 )
    {
       double dval;

       if ( ! value_.getDouble( dval ) )
       {
          return false;
       }

       val = static_cast<float>(dval);

       return true;
    }
  return false;
}




bool
sc_Subfield::getBFP64(double& val) const
{
  if (type_ == is_BFP64)
    return value_.getDouble(val);
  return false;
}



bool
sc_Subfield::getInt( int & val ) const
{
   long   l;
   unsigned long   ul;
   double d;

   if ( getValue().getLong( l ) )
   {
      val = static_cast<int>(l);

      return true;
   }
   else if ( getValue().getUnsignedLong( ul ) )
   {
      val = static_cast<int>(ul);

      return true;
   }
   else if ( getValue().getDouble( d ) )
   {
      val = static_cast<int>(d);

      return true;
   }

   return false;

} // sc_Subfield::getInt



bool
sc_Subfield::getFloat( float & val ) const
{
   long   l;
   unsigned long   ul;
   double d;

   if ( getValue().getDouble( d ) )
   {
      val = static_cast<float>(d);

      return true;

   }
   else if ( getValue().getLong( l ) )
   {
      val = static_cast<float>(l);

      return true;
   }
   else if ( getValue().getUnsignedLong( ul ) )
   {
      val = static_cast<float>(ul);

      return true;
   }

   return false;

} // sc_Subfield::getFloat




bool
sc_Subfield::getDouble( double & val ) const
{
   long l;
   unsigned long ul;

   if ( getValue().getDouble( val ) )
   {
      return true;
   }
   else if ( getValue().getLong( l  ) )
   {
      val = static_cast<double>( l );

      return true;
   }
   else if ( getValue().getUnsignedLong( ul ) )
   {
      val = static_cast<double>(ul);

      return true;
   }

   return false;

} // sc_Subfield::getDouble




sc_MultiTypeValue const&
sc_Subfield::getValue() const
{
  return value_;
}



string const&
sc_Subfield::setName(string const& name)
{
  return name_ = name;
}



string const&
sc_Subfield::setMnemonic(string const& mnem)
{
  return mnemonic_ = mnem;
}



void
sc_Subfield::setA(string const& val)
{
  type_ = is_A;
  value_.setString(val);
}




void
sc_Subfield::setI(long val)
{
  type_ = is_I;
  value_.setLong(val);
}



void
sc_Subfield::setR(double val)
{
  type_ = is_R;
  value_.setDouble(val);
}



void
sc_Subfield::setS(double val)
{
  type_ = is_S;
  value_.setDouble(val);
}



void
sc_Subfield::setC(string const& val)
{
  type_ = is_C;
  value_.setString(val);
}



void
sc_Subfield::setBI8(long val)
{
  type_ = is_BI8;
  value_.setLong(val);
}



void
sc_Subfield::setBI16(long val)
{
  type_ = is_BI16;
  value_.setLong(val);
}



void
sc_Subfield::setBI24(long val)
{
  type_ = is_BI24;
  value_.setLong(val);
}



void
sc_Subfield::setBI32(long val)
{
  type_ = is_BI32;
  value_.setLong(val);
}




void
sc_Subfield::setBUI8(unsigned long val)
{
  type_ = is_BUI8;
  value_.setUnsignedLong(val);
}




void
sc_Subfield::setBUI16(unsigned long val)
{
  type_ = is_BUI16;
  value_.setUnsignedLong(val);
}




void
sc_Subfield::setBUI24(unsigned long val)
{
  type_ = is_BUI24;
  value_.setUnsignedLong(val);
}



void
sc_Subfield::setBUI32(unsigned long val)
{
  type_ = is_BUI32;
  value_.setUnsignedLong(val);
}




void
sc_Subfield::setBFP32(float val)
{
  type_ = is_BFP32;
  value_.setDouble(val);
}




void
sc_Subfield::setBFP64(double val)
{
  type_ = is_BFP64;
  value_.setDouble(val);
}



void
sc_Subfield::setUnvalued()
{				// note that the type_ is unchanged
  value_.setNull();
} // sc_Subfield::setUnvalued()




ostream&
operator<<( ostream& s, const sc_Subfield& subfield )
{
  // Dump the given sc_Subfield to stream s in a format suitable for
  // use in debugging.

  s << "\t" << subfield.getMnemonic() << " : " << subfield.getName() << " = ";

  string        tmp_string;
  long          tmp_long;
  unsigned long tmp_ulong;
  float         tmp_float;
  double        tmp_double;

  switch ( subfield.getSubfieldType() )
    {
    case sc_Subfield::is_A :
      if ( subfield.getA( tmp_string ) )
      {
	s << tmp_string;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_I :
      if ( subfield.getI( tmp_long ) )
      {
	s << tmp_long;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_R :
      if ( subfield.getR( tmp_double ) )
      {
        s.setf( ios::fixed );
        s.precision( 8 );
	s << tmp_double;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_S :
      if ( subfield.getS( tmp_double ) )
      {
        s.setf( ios::fixed );
        s.precision( 8 );
	s << tmp_double;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_C :
      if ( subfield.getC( tmp_string ) )
      {
	s << tmp_string;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BI8 :
      if ( subfield.getBI8( tmp_long ) )
      {
	s << tmp_long;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BI16 :
      if ( subfield.getBI16( tmp_long ) )
      {
	s << tmp_long;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BI24 :
      if ( subfield.getBI24( tmp_long ) )
      {
	s << tmp_long;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BI32 :
      if ( subfield.getBI32( tmp_long ) )
      {
	s << /* hex << */ tmp_long /* << dec*/;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BUI8 :
      if ( subfield.getBUI8( tmp_ulong ) )
      {
	s << tmp_ulong;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BUI16 :
      if ( subfield.getBUI16( tmp_ulong ) )
      {
	s << tmp_ulong;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BUI24 :
      if ( subfield.getBUI24( tmp_ulong ) )
      {
	s << tmp_ulong;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BUI32 :
      if ( subfield.getBUI32( tmp_ulong ) )
      {
	s << tmp_ulong;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BFP32 :
      if ( subfield.getBFP32( tmp_float ) )
      {
        s.setf( ios::fixed );
        s.precision( 8 );
	s << tmp_float;
      }
      else
      {
	s << string("null value");
      }
      break;

    case sc_Subfield::is_BFP64 :
      if ( subfield.getBFP64( tmp_double ) )
      {
        s.setf( ios::fixed  );
        s.precision( 8 );
	s << tmp_double;
      }
      else
      {
	s << string("null value");
      }
      break;

    default :
      s << "unsupported subfield type";
    }


  switch ( subfield.getSubfieldType() )
    {
    case sc_Subfield::is_A :
      s << " (A)";
      break;
    case sc_Subfield::is_I :
      s << " (I)";
      break;
    case sc_Subfield::is_R :
      s << " (R)";
      break;
    case sc_Subfield::is_S :
      s << " (S)";
      break;
    case sc_Subfield::is_C :
      s << " (C)";
      break;
    case sc_Subfield::is_B :
      s << " (B)";
      break;
    case sc_Subfield::is_BI8 :
      s << " (BI8)";
      break;
    case sc_Subfield::is_BI16 :
      s << " (BI16)";
      break;
    case sc_Subfield::is_BI24 :
      s << " (BI24)";
      break;
    case sc_Subfield::is_BI32 :
      s << " (BI32)";
      break;
    case sc_Subfield::is_BUI :
      s << " (BUI)";
      break;
    case sc_Subfield::is_BUI8 :
      s << " (BUI8)";
      break;
    case sc_Subfield::is_BUI16 :
      s << " (BUI16)";
      break;
    case sc_Subfield::is_BUI24 :
      s << " (BUI24)";
      break;
    case sc_Subfield::is_BUI32 :
      s << " (BUI32)";
      break;
    case sc_Subfield::is_BFP32 :
      s << " (FP32)";
      break;
    case sc_Subfield::is_BFP64 :
      s << " (FP64)";
      break;
    }

  return s;
}


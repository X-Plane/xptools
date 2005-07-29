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
// $Id: sb_At.cpp,v 1.8 2002/11/24 22:07:42 mcoletti Exp $
//


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <sdts++/builder/sb_At.h>

#include <iostream>
#include <sstream>

#include <algorithm>
#include <functional>

#include <limits.h>
#include <float.h>

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif

#ifndef INCLUDED_SC_FIELD_H
#include <sdts++/container/sc_Field.h>
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif


using namespace std;



static const char* ident_ = "$Id: sb_At.cpp,v 1.8 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;



struct sb_At_Imp
{

				// The attributes are stored in this
				// sc_Subfield container.  Each
				// Subfield will have the attribute
				// name, type, and value.

      list<sc_Subfield> attributes_;


      sb_At_Imp() {}

}; // struct sb_At_Imp



sb_At::sb_At()
   : imp_( new sb_At_Imp() )
{
   setID( 1 );

   // insert static initializers

} // At ctor


sb_At::~sb_At()
{
   delete imp_;
} // At dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;
static sio_8211Converter_S converter_S;
static sio_8211Converter_BI8 converter_BI8;
static sio_8211Converter_BI16 converter_BI16;
static sio_8211Converter_BI24 converter_BI24;
static sio_8211Converter_BI32 converter_BI32;
static sio_8211Converter_BUI8 converter_BUI8;
static sio_8211Converter_BUI16 converter_BUI16;
static sio_8211Converter_BUI24 converter_BUI24;
static sio_8211Converter_BUI32 converter_BUI32;
static sio_8211Converter_BFP32 converter_BFP32;
static sio_8211Converter_BFP64 converter_BFP64;


static
bool
ingest_record_( sb_At& at, sb_At_Imp &at_imp, sc_Record const& record )
{

   // Make sure we have a record from an
   // External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record,"AT",curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_At::sb_At(sc_Record const&): "
           << "Not an attribute record.";
      cerr << endl;
#endif
      return false;
   }


   // We have a primary field from a  module. Start// picking it apart.

   sc_SubfieldCntr::const_iterator cursubfield;

   string tmp_str;
   long   tmp_int;


   // MODN
   if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
   {
      cursubfield->getA( tmp_str );
      at.setMnemonic( tmp_str );
   }


   // RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      at.setID( tmp_int );
   }

#ifdef NOTWORKINGYET
   // AT
   if (sb_Utils::getSubfieldByMnem(*curfield,"AT",cursubfield))
   {
      cursubfield->getA( at_imp.Attributes_);
   }
   else
   {
      return false;
   }
#endif

   return true;


} // ingest__record



list<sc_Subfield> const&
sb_At::getAttributes( ) const
{
   return imp_->attributes_;
} // sb_At::getAttributes



bool
sb_At::getAttributeTypes( attribute_types& val ) const
{

   val.clear();                  // insure we start with a clean slate

   for ( list<sc_Subfield>::const_iterator i = imp_->attributes_.begin();
         i != imp_->attributes_.end();
         i++ )
   {
      val.push_back( make_pair( (*i).getName(), (*i).getSubfieldType() ) );
   }

   return val.size() == imp_->attributes_.size();
} // sb_At::getAttributeTypes



bool
sb_At::setAttributeTypes( attribute_types const& val )
{
   imp_->attributes_.clear();

   for ( attribute_types::const_iterator i = val.begin();
         i != val.end();
         i++ )
   {
      imp_->attributes_.push_back( sc_Subfield() );

				// pair<string, sc_SubfieldType>'s first
				// element is the attribute name

      imp_->attributes_.back().setName( (*i).first );

				// now we have the set the subfield
				// to the proper type and give it a
				// proper NULL-like value
      switch( (*i).second )
      {
         case sc_Subfield::is_A :
            imp_->attributes_.back().setA( "" );
            imp_->attributes_.back().setUnvalued( );
            break;

         case sc_Subfield::is_I :
            imp_->attributes_.back().setI( (long) 0 );
            imp_->attributes_.back().setUnvalued( );
            break; 

         case sc_Subfield::is_R :
            imp_->attributes_.back().setR( 0 );
            imp_->attributes_.back().setUnvalued();
            break; 

         case sc_Subfield::is_S :
            imp_->attributes_.back().setS( 0 );
            imp_->attributes_.back().setUnvalued();
            break; 

         case sc_Subfield::is_C :
            imp_->attributes_.back().setC( "" );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BI8 :
            imp_->attributes_.back().setBI8( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BI16 :
            imp_->attributes_.back().setBI16( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BI24 :
            imp_->attributes_.back().setBI24( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BI32 :
            imp_->attributes_.back().setBI32( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BUI8 :
            imp_->attributes_.back().setBUI8( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BUI16 :
            imp_->attributes_.back().setBUI16( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BUI24 :
            imp_->attributes_.back().setBUI24( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BUI32 :
            imp_->attributes_.back().setBUI32( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BFP32 :
            imp_->attributes_.back().setBFP32( 0 );
            imp_->attributes_.back().setUnvalued();
            break;

         case sc_Subfield::is_BFP64 :
            imp_->attributes_.back().setBFP64( 0 );
            imp_->attributes_.back().setUnvalued();
            break;


         default :
            break;

      }
   }

   return true;
} // sb_At::setAttributes


struct
findAttribute
{
      bool operator()( sc_Subfield const& sf )
      {
         return sf.getName() == val_;
      }

      findAttribute( string const& val ) : val_( val ) {}

      string val_;
}; // unary predicate findAttribute


bool
sb_At::unDefineAttribute( string const& attribute_name )
{
   list<sc_Subfield>::iterator i = 
      find_if( imp_->attributes_.begin(), imp_->attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp_->attributes_.end() )
   {
      (*i).setUnvalued();
   }

   return false;

} // sb_At::unDefineAttributes



// functions for getting non-string subfield attribute values, one
// each for the four different numeric types

static
bool
getNumericAttribute_( string const& attribute_name, sb_At_Imp& imp, long& val )
{
   list<sc_Subfield>::const_iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            return imp.attributes_.back().getI( val );
            break; 

         case sc_Subfield::is_R :
            return false;
            break; 

         case sc_Subfield::is_S :
            return false;
            break; 

         case sc_Subfield::is_BI8 :
            return imp.attributes_.back().getBI8( val );
            break;

         case sc_Subfield::is_BI16 :
            return imp.attributes_.back().getBI16( val );
            break;

         case sc_Subfield::is_BI24 :
            return imp.attributes_.back().getBI24( val );
            break;

         case sc_Subfield::is_BI32 :
            return imp.attributes_.back().getBI32( val );
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            return false;
            break;

         case sc_Subfield::is_BFP64 :
            return false;
            break;


         default :
            break;

      }
   }

   return false;

} // getNumericAttribute_



static
bool
getNumericAttribute_( string const& attribute_name, sb_At_Imp& imp, unsigned long& val )
{
   list<sc_Subfield>::const_iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            long tmp_long;
            return imp.attributes_.back().getI( tmp_long );
            val = static_cast<unsigned long>(tmp_long);
            break; 

         case sc_Subfield::is_R :
            return false;
            break; 

         case sc_Subfield::is_S :
            return false;
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            return imp.attributes_.back().getBUI8( val );
            break;

         case sc_Subfield::is_BUI16 :
            return imp.attributes_.back().getBUI16( val );
            break;

         case sc_Subfield::is_BUI24 :
            return imp.attributes_.back().getBUI24( val );
            break;

         case sc_Subfield::is_BUI32 :
            return imp.attributes_.back().getBUI32( val );
            break;

         case sc_Subfield::is_BFP32 :
            return false;
            break;

         case sc_Subfield::is_BFP64 :
            return false;
            break;


         default :
            break;

      }
   }

   return false;

} // getNumericAttribute_



static
bool
getNumericAttribute_( string const& attribute_name, sb_At_Imp& imp, float& val )
{
   list<sc_Subfield>::const_iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   double tmp_double;            // for casting convenience

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            return false;
            break; 

         case sc_Subfield::is_R :
            if ( ! imp.attributes_.back().getR( tmp_double ) ) return false;
            val = static_cast<float>(tmp_double); // XXX dangerout cast
            break; 

         case sc_Subfield::is_S :
            if ( ! imp.attributes_.back().getS( tmp_double ) ) return false;
            val = static_cast<float>(tmp_double); // XXX dangerout cast
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            if ( ! imp.attributes_.back().getBFP32( val ) ) return false;
            break;

         case sc_Subfield::is_BFP64 :
            if ( ! imp.attributes_.back().getBFP64( tmp_double ) ) return false;
            val = static_cast<float>(tmp_double);
            break;

         default :
            break;

      }
   }

   return false;

} // getNumericAttribute_







static
bool
getNumericAttribute_( string const& attribute_name, sb_At_Imp& imp, double& val )
{
   list<sc_Subfield>::const_iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            return false;
            break; 

         case sc_Subfield::is_R :
            return imp.attributes_.back().getR( val );
            break; 

         case sc_Subfield::is_S :
            return imp.attributes_.back().getS( val );
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            float tmp_float;
            if ( ! imp.attributes_.back().getBFP32( tmp_float ) ) return false;
            val = static_cast<double>(tmp_float);
            break;

         case sc_Subfield::is_BFP64 :
            return imp.attributes_.back().getBFP64( val );
            break;


         default :
            break;

      }
   }

   return false;

} // getNumericAttribute_



bool
sb_At::getAttribute( string const& attribute_name, long&   val ) const
{
   return getNumericAttribute_( attribute_name, *imp_, val );
}

bool
sb_At::getAttribute( string const& attribute_name, unsigned long&   val ) const
{
   return getNumericAttribute_( attribute_name,  *imp_, val );
}

bool
sb_At::getAttribute( string const& attribute_name, float&  val ) const
{
   return getNumericAttribute_( attribute_name,  *imp_, val );
}

bool
sb_At::getAttribute( string const& attribute_name, double& val ) const
{
   return getNumericAttribute_( attribute_name,  *imp_, val );
}

bool
sb_At::getAttribute( string const& attribute_name, string& val ) const
{
   list<sc_Subfield>::const_iterator i = 
      find_if( imp_->attributes_.begin(), imp_->attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp_->attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_A :
            return imp_->attributes_.back().getA( val );
            break; 

         case sc_Subfield::is_C :
            return imp_->attributes_.back().getC( val );
            break; 

         default :
            break;

      }
   }

   return false;

} // getAttribute








// functions for setting numeric subfield attribute values
static
bool
_setNumericAttribute( string const& attribute_name, sb_At_Imp& imp, long const& val )
{
   list<sc_Subfield>::iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            (*i).setI( val );
            return true;
            break; 

         case sc_Subfield::is_R :
            return false;
            break; 

         case sc_Subfield::is_S :
            return false;
            break; 

         case sc_Subfield::is_BI8 :
            (*i).setBI8( val );
            return true;
            break;

         case sc_Subfield::is_BI16 :
            (*i).setBI16( val );
            return true;
            break;

         case sc_Subfield::is_BI24 :
            (*i).setBI24( val );
            return true;
            break;

         case sc_Subfield::is_BI32 :
            (*i).setBI32( val );
            return true;
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            return false;
            break;

         case sc_Subfield::is_BFP64 :
            return false;
            break;


         default :
            break;

      }
   }

   return false;

} // setNumericAttribute_



static
bool
_setNumericAttribute( string const& attribute_name, sb_At_Imp& imp, unsigned long const& val )
{
   list<sc_Subfield>::iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            (*i).setI( val );
            return true;
            break; 

         case sc_Subfield::is_R :
            return false;
            break; 

         case sc_Subfield::is_S :
            return false;
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            (*i).setBUI8( val );
            return true;
            break;

         case sc_Subfield::is_BUI16 :
            (*i).setBUI16( val );
            return true;
            break;

         case sc_Subfield::is_BUI24 :
            (*i).setBUI24( val );
            return true;
            break;

         case sc_Subfield::is_BUI32 :
            (*i).setBUI32( val );
            return true;
            break;

         case sc_Subfield::is_BFP32 :
            return false;
            break;

         case sc_Subfield::is_BFP64 :
            return false;
            break;


         default :
            break;

      }

      return true;

   }

   return false;

} // setNumericAttribute_


static
bool
_setNumericAttribute( string const& attribute_name, sb_At_Imp& imp, float const& val )
{
   list<sc_Subfield>::iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   double tmp_double = static_cast<double>(val);

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            return false;
            break; 

         case sc_Subfield::is_R :
            (*i).setR( tmp_double );
            return true;
            break; 

         case sc_Subfield::is_S :
            (*i).setS( tmp_double );
            return true;
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            (*i).setBFP32( val );
            return true;
            break;

         case sc_Subfield::is_BFP64 :
            (*i).setBFP64( tmp_double );
            return true;
            break;


         default :
            break;

      }
   }

   return false;

} // setNumericAttribute_




static
bool
setNumericAttribute_( string const& attribute_name, sb_At_Imp& imp, double const& val )
{
   list<sc_Subfield>::iterator i = 
      find_if( imp.attributes_.begin(), imp.attributes_.end(), 
               findAttribute( attribute_name) );

   float tmp_float;

   if ( i != imp.attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_I :
            return false;
            break; 

         case sc_Subfield::is_R :
            (*i).setR( val );
            return true;
            break; 

         case sc_Subfield::is_S :
            (*i).setS( val );
            return true;
            break; 

         case sc_Subfield::is_BI8 :
            return false;
            break;

         case sc_Subfield::is_BI16 :
            return false;
            break;

         case sc_Subfield::is_BI24 :
            return false;
            break;

         case sc_Subfield::is_BI32 :
            return false;
            break;

         case sc_Subfield::is_BUI8 :
            return false;
            break;

         case sc_Subfield::is_BUI16 :
            return false;
            break;

         case sc_Subfield::is_BUI24 :
            return false;
            break;

         case sc_Subfield::is_BUI32 :
            return false;
            break;

         case sc_Subfield::is_BFP32 :
            tmp_float = static_cast<float>(val); // XXX yes, unsafe cast
            (*i).setBFP32( tmp_float );
            return true;
            break;

         case sc_Subfield::is_BFP64 :
            (*i).setBFP64( val );
            return true;
            break;


         default :
            break;

      }
   }

   return false;

} // setNumericAttribute_


bool
sb_At::setAttribute( string const& attribute_name, long   const& val ) 
{
   return setNumericAttribute_( attribute_name,  *imp_, val );
} // setAttribute

bool
sb_At::setAttribute( string const& attribute_name, unsigned long   const& val ) 
{
   return setNumericAttribute_( attribute_name,  *imp_, val );
} // setAttribute

bool
sb_At::setAttribute( string const& attribute_name, float  const& val ) 
{
   return setNumericAttribute_( attribute_name,  *imp_, val );
} // setAttribute

bool
sb_At::setAttribute( string const& attribute_name, double const& val ) 
{
   return setNumericAttribute_( attribute_name,  *imp_, val );
} // setAttribute

bool
sb_At::setAttribute( string const& attribute_name, string const& val ) 
{
   list<sc_Subfield>::iterator i = 
      find_if( imp_->attributes_.begin(), imp_->attributes_.end(), 
               findAttribute( attribute_name) );

   if ( i != imp_->attributes_.end() )
   {

      switch( (*i).getSubfieldType() )
      {
         case sc_Subfield::is_A :
            i->setA( val );
            break; 

         case sc_Subfield::is_C :
            i->setC( val );
            break; 

         default :
            break;

      }

      return true;

   }

   return false;
} // setAttribute







//
// Primary Attributes
//


// convenience class to be used by STL for_each() to insert subfield
// formats for a given schema

class
addSubfieldFormat
{
   public:

      addSubfieldFormat( sio_8211Schema& schema ) : schema_( schema ) {}

      void operator()( sc_Subfield const& sf )
      {
                                // convenient syntactic sugar reference

         sio_8211FieldFormat& field_format = schema_.back();


         field_format.push_back( sio_8211SubfieldFormat() );

         field_format.back().setLabel( sf.getName() );

         switch ( sf.getSubfieldType() )
         {

            case sc_Subfield::is_A :
               field_format.back().setType( sio_8211SubfieldFormat::A );
               field_format.back().setConverter( &converter_A );
               field_format.back().setFormat( sio_8211SubfieldFormat::variable );
               break; 

            case sc_Subfield::is_I :
               field_format.back().setType( sio_8211SubfieldFormat::I );
               field_format.back().setConverter( &converter_I );
               field_format.back().setFormat( sio_8211SubfieldFormat::variable );
               break; 

            case sc_Subfield::is_R :
               field_format.back().setType( sio_8211SubfieldFormat::R );
               field_format.back().setConverter( &converter_R );
               field_format.back().setFormat( sio_8211SubfieldFormat::variable );
               break; 

            case sc_Subfield::is_S :
               field_format.back().setType( sio_8211SubfieldFormat::S );
               field_format.back().setConverter( &converter_S );
               field_format.back().setFormat( sio_8211SubfieldFormat::variable );
               break; 

            case sc_Subfield::is_BI8 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setLength( 8 );
               field_format.back().setConverter( &converter_BI8 );
               break;

            case sc_Subfield::is_BI16 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BI16 );
               field_format.back().setLength( 16 );
               break;

            case sc_Subfield::is_BI24 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BI24 );
               field_format.back().setLength( 24 );
               break;

            case sc_Subfield::is_BI32 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BI32 );
               field_format.back().setLength( 32 );
               break;

            case sc_Subfield::is_BUI8 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BUI8 );
               field_format.back().setLength( 8 );
               break;

            case sc_Subfield::is_BUI16 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BUI16 );
               field_format.back().setLength( 16 );
               break;

            case sc_Subfield::is_BUI24 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BUI24 );
               field_format.back().setLength( 24 );
               break;

            case sc_Subfield::is_BUI32 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BUI32 );
               field_format.back().setLength( 32 );
               break;

            case sc_Subfield::is_BFP32 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BFP32 );
               field_format.back().setLength( 32 );
               break;

            case sc_Subfield::is_BFP64 :
               field_format.back().setType( sio_8211SubfieldFormat::B );
               field_format.back().setConverter( &converter_BFP64 );
               field_format.back().setLength( 64 );
               break;

            default :
               break;
      
         }
      }

   private:

      sio_8211Schema& schema_;

}; // addSubfieldFormat




void
sb_Atpr::buildSpecificSchema_( )
{
   list<sc_Subfield> const& attributes = getAttributes();

   schema_().push_back( sio_8211FieldFormat() );

   sio_8211FieldFormat& field_format = schema_().back();

   field_format.setDataStructCode( sio_8211FieldFormat::vector );
   field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
   field_format.setName( "Attribute Primary" );
   field_format.setTag( "ATPR" );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "MODN" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "RCID" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_I );


   schema_().push_back( sio_8211FieldFormat() );


   schema_().back().setDataStructCode( sio_8211FieldFormat::vector );
   schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
   schema_().back().setName( "Primary Attribute" );
   schema_().back().setTag( "ATTP" );

                                // now add the attribute fields

   for_each( attributes.begin(), attributes.end(), 
             addSubfieldFormat( schema_() ) );

} // build__atpr_schema




struct sb_Atpr_Imp
{
      sio_8211Schema schema_;
}; // struct sb_Atpr_Imp



sb_Atpr::sb_Atpr() : imp_( new sb_Atpr_Imp() ) 
{
   setMnemonic( "ATPR" );
}


sb_Atpr::~sb_Atpr() { delete imp_; }



sio_8211Schema&
sb_Atpr::schema_()
{
   return imp_->schema_;
} // sb_Atpr::schema_()







// convenience class to be used by STL for_each() to insert subfields
// into the given record's last field

class
addSubfield
{
   public:

      addSubfield( sc_Record& record ) : record_( record ) {}

      void operator()( sc_Subfield const& sf )
      {
         record_.back().push_back( sf );
      }

   private:

      sc_Record& record_;

}; // addSubfield



bool
sb_Atpr::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

   // first field, which contains module name and record number

   record.push_back( sc_Field() );

   record.back().setMnemonic( "ATPR" );

   record.back().setName( "Attribute Primary" );

   string tmp_str;

   getMnemonic( tmp_str );
   sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
   sb_Utils::add_subfield( record.back(), "RCID", getID() );


   record.push_back( sc_Field() );

   record.back().setMnemonic( "ATTP" );


                                // now add the other subfields based
                                // on the current set of attributes

   for_each( getAttributes().begin(), getAttributes().end(), 
             addSubfield( record ) );

   return true;


} // At::getRecord




#ifdef NOTIMPLEMENTED
bool
sb_Atpr::setRecord( sc_Record const& record )
{
   return ingest__record( *this, *imp_, record );
} // sb_At::setRecord
#endif



//
// Secondary Attributes
//


// XXX NEEDS TO BE IMPLEMENTED!!

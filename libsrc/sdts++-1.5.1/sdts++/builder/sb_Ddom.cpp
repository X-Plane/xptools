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
// $Id: sb_Ddom.cpp,v 1.9 2002/11/24 22:07:42 mcoletti Exp $
//

#include "sb_Ddom.h"


#include <iostream>
#include <strstream>
#include <algorithm>

#include <limits.h>
#include <float.h>

#ifndef INCLUDED_SB_UTILS_H
#include "sb_Utils.h"
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include "sb_ForeignID.h"
#endif

#ifndef INCLUDED_SC_RECORD_H
#include "../container/sc_Record.h"
#endif

#ifndef INCLUDED_SC_FIELD_H
#include "../container/sc_Field.h"
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include "../container/sc_Subfield.h"
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include "../io/sio_8211Converter.h"
#endif


using namespace std;


static const char* ident_ = "$Id: sb_Ddom.cpp,v 1.9 2002/11/24 22:07:42 mcoletti Exp $";



// XXX as many places as I use this, I need to consolidate this into some
// XXX sort of utility library
//
inline
static
string::value_type
lowercase_( string::value_type c ) { return tolower( c ); }



// This converts the given subfield type to its string equivalent.
//
static
string
subfieldTypeToStr_( sc_Subfield::SubfieldType const & st )
{
  switch ( st )
    {
    case sc_Subfield::is_A :
      return "A";

    case sc_Subfield::is_I :
      return "I";

    case sc_Subfield::is_R :
      return "R";

    case sc_Subfield::is_S :
      return "S";

    case sc_Subfield::is_C :
      return "C";

    case sc_Subfield::is_B :
      return "B";

    case sc_Subfield::is_BI8 :
      return "BI8";

    case sc_Subfield::is_BI16 :
      return "BI16";

    case sc_Subfield::is_BI24 :
      return "BI24";

    case sc_Subfield::is_BI32 :
      return "BI32";

    case sc_Subfield::is_BUI :
      return "BUI";

    case sc_Subfield::is_BUI8 :
      return "BUI8";

    case sc_Subfield::is_BUI16 :
      return "BUI16";

    case sc_Subfield::is_BUI24 :
      return "BUI24";

    case sc_Subfield::is_BUI32 :
      return "BUI32";

    case sc_Subfield::is_BFP32 :
      return "BFP32";

    case sc_Subfield::is_BFP64 :
      return "BFP64";

    }

  return "";
} // subfieldTypeToStr_







// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Ddom_Imp
{
  string   Name_;
  string   Type_;
  string   AttributeLabel_;
  string   AttributeAuthority_;
  string   AttributeDomainType_;
  sc_Subfield::SubfieldType AttributeDomainValueFormat_;
  string   AttributeDomainValueMeasurementUnit_;
  string   RangeOrValue_;
  sc_Subfield DomainValue_;
  string   DomainValueDefinition_;


  sb_Ddom_Imp()
    : 
    Name_( UNVALUED_STRING ),
    Type_( UNVALUED_STRING ),
    AttributeLabel_( UNVALUED_STRING ),
    AttributeAuthority_( UNVALUED_STRING ),
    AttributeDomainType_( UNVALUED_STRING ),
    AttributeDomainValueFormat_( sc_Subfield::is_I ),
    AttributeDomainValueMeasurementUnit_( UNVALUED_STRING ),
    RangeOrValue_( UNVALUED_STRING ),
    DomainValue_( "", "DVAL" ),
    DomainValueDefinition_( UNVALUED_STRING )
    {}

};


sb_Ddom::sb_Ddom()
  : imp_( new sb_Ddom_Imp() )
{
  setMnemonic("DDOM");
  setID( 1 );


  // insert static initializers

} // Ddom ctor


sb_Ddom::~sb_Ddom()
{
  delete imp_;
} // Ddom dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema schema_; // module specific schema


sio_8211Schema&
sb_Ddom::schema_()
{
   return ::schema_;
} // sb_Ddom::schema_()



void
sb_Ddom::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Data Dictionary/Domain" );
  field_format.setTag( "DDOM" );


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

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NAME" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "TYPE" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ATLB" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "AUTH" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ATYP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ADVF" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ADMU" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RAVA" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DVAL" );
  // XXX FIX THIS AS IT DOESN'T HANDLE THE DIFFERENT TYPES THIS CAN HAVE
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DVDF" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


} // build__schema


static
bool
ingest_record_( sb_Ddom& ddom, sb_Ddom_Imp &ddom_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"DDOM",curfield) )
    {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Ddom::sb_Ddom(sc_Record const&): "
           << "Not an data dictionary/domain record.";
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
      ddom.setMnemonic( tmp_str );
    }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
    {
      cursubfield->getI( tmp_int );
      ddom.setID( tmp_int );
    }


  // NAME
  if (sb_Utils::getSubfieldByMnem(*curfield,"NAME",cursubfield))
    {
      cursubfield->getA( ddom_imp.Name_);
    }


  // TYPE
  if (sb_Utils::getSubfieldByMnem(*curfield,"TYPE",cursubfield))
    {
      cursubfield->getA( ddom_imp.Type_);
    }


  // ATLB
  if (sb_Utils::getSubfieldByMnem(*curfield,"ATLB",cursubfield))
    {
      cursubfield->getA( ddom_imp.AttributeLabel_);
    }


  // AUTH
  if (sb_Utils::getSubfieldByMnem(*curfield,"AUTH",cursubfield))
    {
      cursubfield->getA( ddom_imp.AttributeAuthority_);
    }


  // ATYP
  if (sb_Utils::getSubfieldByMnem(*curfield,"ATYP",cursubfield))
    {
      cursubfield->getA( ddom_imp.AttributeDomainType_);
    }


  // ADVF
  if (sb_Utils::getSubfieldByMnem(*curfield,"ADVF",cursubfield))
    {
      ddom_imp.AttributeDomainValueFormat_ = cursubfield->getSubfieldType();
    }


  // ADMU
  if (sb_Utils::getSubfieldByMnem(*curfield,"ADMU",cursubfield))
    {
      cursubfield->getA( ddom_imp.AttributeDomainValueMeasurementUnit_);
    }


  // RAVA
  if (sb_Utils::getSubfieldByMnem(*curfield,"RAVA",cursubfield))
    {
      cursubfield->getA( ddom_imp.RangeOrValue_);
    }


  // DVAL
  if (sb_Utils::getSubfieldByMnem(*curfield,"DVAL",cursubfield))
    {
      if ( ! ddom.setDVAL( (*cursubfield) ) ) return false;
    }


  // DVDF
  if (sb_Utils::getSubfieldByMnem(*curfield,"DVDF",cursubfield))
    {
      cursubfield->getA( ddom_imp.DomainValueDefinition_);
    }


  return true;


} // ingest_record_




bool
sb_Ddom::getName( string& val ) const
{
  if ( imp_->Name_ == UNVALUED_STRING )
    return false;

  val = imp_->Name_;

  return true;
} // sb_Ddom::getName


bool
sb_Ddom::getType( string& val ) const
{
  if ( imp_->Type_ == UNVALUED_STRING )
    return false;

  val = imp_->Type_;

  return true;
} // sb_Ddom::getType


bool
sb_Ddom::getAttributeLabel( string& val ) const
{
  if ( imp_->AttributeLabel_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeLabel_;

  return true;
} // sb_Ddom::getAttributeLabel


bool
sb_Ddom::getAttributeAuthority( string& val ) const
{
  if ( imp_->AttributeAuthority_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeAuthority_;

  return true;
} // sb_Ddom::getAttributeAuthority


bool
sb_Ddom::getAttributeDomainType( string& val ) const
{
  if ( imp_->AttributeDomainType_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeDomainType_;

  return true;
} // sb_Ddom::getAttributeDomainType


bool
sb_Ddom::getAttributeDomainValueFormat( sc_Subfield::SubfieldType& val ) const
{
  val = imp_->AttributeDomainValueFormat_;

  return true;
} // sb_Ddom::getAttributeDomainValueFormat


bool
sb_Ddom::getAttributeDomainValueMeasurementUnit( string& val ) const
{
  if ( imp_->AttributeDomainValueMeasurementUnit_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeDomainValueMeasurementUnit_;

  return true;
} // sb_Ddom::getAttributeDomainValueMeasurementUnit


bool
sb_Ddom::getRangeOrValue( string& val ) const
{
  if ( imp_->RangeOrValue_ == UNVALUED_STRING )
    return false;

  val = imp_->RangeOrValue_;

  return true;
} // sb_Ddom::getRangeOrValue


bool
sb_Ddom::getDomainValue( sc_Subfield& val ) const
{
  val = imp_->DomainValue_;

  return true;
} // sb_Ddom::getDomainValue


bool
sb_Ddom::getDomainValueDefinition( string& val ) const
{
  if ( imp_->DomainValueDefinition_ == UNVALUED_STRING )
    return false;

  val = imp_->DomainValueDefinition_;

  return true;
} // sb_Ddom::getDomainValueDefinition





bool
sb_Ddom::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DDOM" );

  record.back().setName( "Data Dictionary/Domain" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getName( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"NAME", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "NAME", sc_Subfield::is_A );
    }


  if ( getType( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"TYPE", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "TYPE", sc_Subfield::is_A );
    }


  if ( getAttributeLabel( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"ATLB", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ATLB", sc_Subfield::is_A );
    }


  if ( getAttributeAuthority( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"AUTH", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "AUTH", sc_Subfield::is_A );
    }


  if ( getAttributeDomainType( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"ATYP", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ATYP", sc_Subfield::is_A );
    }

  sc_Subfield::SubfieldType tmp_subfield_type;

  if ( getAttributeDomainValueFormat( tmp_subfield_type ) )
    {
      tmp_str = subfieldTypeToStr_( tmp_subfield_type );
      sb_Utils::add_subfield( record.back(), "ADVF", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ADVF", sc_Subfield::is_A );
    }


  if ( getAttributeDomainValueMeasurementUnit( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"ADMU", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ADMU", sc_Subfield::is_A );
    }


  if ( getRangeOrValue( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"RAVA", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "RAVA", sc_Subfield::is_A );
    }


  sc_Subfield tmp_subfield;

  if ( getDomainValue( tmp_subfield ) )
    {
      record.back().push_back( tmp_subfield );
    }
  else                          // hmmm, that should exist, so there was a
    {                           // serious problem, so bail ...
      return false;
    }


  if ( getDomainValueDefinition( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"DVDF", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "DVDF", sc_Subfield::is_A );
    }


  return true;


} // Ddom::getRecord




bool
sb_Ddom::setName( string const& val )
{
  imp_->Name_ = val;

  return true;
} // sb_Ddom::setName


bool
sb_Ddom::setType( string const& val )
{
  imp_->Type_ = val;

  return true;
} // sb_Ddom::setType


bool
sb_Ddom::setAttributeLabel( string const& val )
{
  imp_->AttributeLabel_ = val;

  return true;
} // sb_Ddom::setAttributeLabel


bool
sb_Ddom::setAttributeAuthority( string const& val )
{
  imp_->AttributeAuthority_ = val;

  return true;
} // sb_Ddom::setAttributeAuthority


bool
sb_Ddom::setAttributeDomainType( string const& val )
{
  imp_->AttributeDomainType_ = val;

  return true;
} // sb_Ddom::setAttributeDomainType


bool
sb_Ddom::setAttributeDomainValueFormat( sc_Subfield::SubfieldType const& val )
{
  imp_->AttributeDomainValueFormat_ = val;

  return true;
} // sb_Ddom::setAttributeDomainValueFormat


bool
sb_Ddom::setAttributeDomainValueMeasurementUnit( string const& val )
{
  imp_->AttributeDomainValueMeasurementUnit_ = val;

  return true;
} // sb_Ddom::setAttributeDomainValueMeasurementUnit


bool
sb_Ddom::setRangeOrValue( string const& val )
{
  imp_->RangeOrValue_ = val;

  return true;
} // sb_Ddom::setRangeOrValue


bool
sb_Ddom::setDomainValue( sc_Subfield const& val )
{
  imp_->DomainValue_ = val;

  return true;
} // sb_Ddom::setDomainValue


bool
sb_Ddom::setDomainValueDefinition( string const& val )
{
  imp_->DomainValueDefinition_ = val;

  return true;
} // sb_Ddom::setDomainValueDefinition


bool
sb_Ddom::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Ddom::setRecord




void
sb_Ddom::unDefineName( )
{
  imp_->Name_ = UNVALUED_STRING;
} // sb_Ddom::unDefineName


void
sb_Ddom::unDefineType( )
{
  imp_->Type_ = UNVALUED_STRING;
} // sb_Ddom::unDefineType


void
sb_Ddom::unDefineAttributeLabel( )
{
  imp_->AttributeLabel_ = UNVALUED_STRING;
} // sb_Ddom::unDefineAttributeLabel


void
sb_Ddom::unDefineAttributeAuthority( )
{
  imp_->AttributeAuthority_ = UNVALUED_STRING;
} // sb_Ddom::unDefineAttributeAuthority


void
sb_Ddom::unDefineAttributeDomainType( )
{
  imp_->AttributeDomainType_ = UNVALUED_STRING;
} // sb_Ddom::unDefineAttributeDomainType


void
sb_Ddom::unDefineAttributeDomainValueFormat( )
{
  imp_->AttributeDomainValueFormat_ = sc_Subfield::is_I;

                                // as a side-effect, also notify the
                                // actual subfield that it ain't got
                                // no value

  imp_->DomainValue_.setUnvalued();
} // sb_Ddom::unDefineAttributeDomainValueFormat


void
sb_Ddom::unDefineAttributeDomainValueMeasurementUnit( )
{
  imp_->AttributeDomainValueMeasurementUnit_ = UNVALUED_STRING;
} // sb_Ddom::unDefineAttributeDomainValueMeasurementUnit


void
sb_Ddom::unDefineRangeOrValue( )
{
  imp_->RangeOrValue_ = UNVALUED_STRING;
} // sb_Ddom::unDefineRangeOrValue


void
sb_Ddom::unDefineDomainValue( )
{
  imp_->DomainValue_.setUnvalued();
} // sb_Ddom::unDefineDomainValue


void
sb_Ddom::unDefineDomainValueDefinition( )
{
  imp_->DomainValueDefinition_ = UNVALUED_STRING;
} // sb_Ddom::unDefineDomainValueDefinition



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
// $Id: sb_Cats.cpp,v 1.7 2002/11/24 22:07:42 mcoletti Exp $
//

#include <sdts++/builder/sb_Cats.h>



#include <iostream>
#include <strstream>

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


static const char* ident_ = "$Id: sb_Cats.cpp,v 1.7 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Cats_Imp
{
  string   Name_;
  string   Type_;
  string   Domain_;
  string   Map_;
  string   Them_;
  string   AggregateObject_;
  string   AggregateObjectType_;
  string   Comment_;


  sb_Cats_Imp()
    :
    Name_( UNVALUED_STRING ),
    Type_( UNVALUED_STRING ),
    Domain_( UNVALUED_STRING ),
    Map_( UNVALUED_STRING ),
    Them_( UNVALUED_STRING ),
    AggregateObject_( UNVALUED_STRING ),
    AggregateObjectType_( UNVALUED_STRING ),
    Comment_( UNVALUED_STRING )
  {}

};


sb_Cats::sb_Cats()
  : imp_( new sb_Cats_Imp() )
{
  setMnemonic("CATS");
  setID( 1 );


  // insert static initializers

} // Cats ctor


sb_Cats::~sb_Cats()
{
  delete imp_;
} // Cats dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema schema_; // module specific schema



sio_8211Schema&
sb_Cats::schema_()
{
   return ::schema_;
} // sb_Cats::schema_




void
sb_Cats::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Catalog/Spatial Domain" );
  field_format.setTag( "CATS" );


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

  field_format.back().setLabel( "DOMN" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MAP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );
  field_format.back().setLabel( "THEM" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "AGOB" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "AGTP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


} // build__schema


static
bool
ingest_record_( sb_Cats& cats, sb_Cats_Imp &cats_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"CATS",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Cats::sb_Cats(sc_Record const&): "
         << "Not an catalog/spatial domain record.";
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
    cats.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    cats.setID( tmp_int );
  }


  // NAME
  if (sb_Utils::getSubfieldByMnem(*curfield,"NAME",cursubfield))
  {
    cursubfield->getA( cats_imp.Name_);
  }


  // TYPE
  if (sb_Utils::getSubfieldByMnem(*curfield,"TYPE",cursubfield))
  {
    cursubfield->getA( cats_imp.Type_);
  }


  // DOMN
  if (sb_Utils::getSubfieldByMnem(*curfield,"DOMN",cursubfield))
  {
    cursubfield->getA( cats_imp.Domain_);
  }


  // MAP
  if (sb_Utils::getSubfieldByMnem(*curfield,"MAP",cursubfield))
  {
    cursubfield->getA( cats_imp.Map_);
  }


  // THEM
  if (sb_Utils::getSubfieldByMnem(*curfield,"THEM",cursubfield))
  {
     cursubfield->getA( cats_imp.Them_);
  }
  else
  {
     return false;
  }


  // AGOB
  if (sb_Utils::getSubfieldByMnem(*curfield,"AGOB",cursubfield))
  {
    cursubfield->getA( cats_imp.AggregateObject_);
  }


  // AGTP
  if (sb_Utils::getSubfieldByMnem(*curfield,"AGTP",cursubfield))
  {
    cursubfield->getA( cats_imp.AggregateObjectType_);
  }


  // COMT
  if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
  {
    cursubfield->getA( cats_imp.Comment_);
  }


  return true;


} // ingest_record_




bool
sb_Cats::getName( string& val ) const
{
  if ( imp_->Name_ == UNVALUED_STRING )
    return false;

  val = imp_->Name_;

  return true;
} // sb_Cats::getName


bool
sb_Cats::getType( string& val ) const
{
  if ( imp_->Type_ == UNVALUED_STRING )
    return false;

  val = imp_->Type_;

  return true;
} // sb_Cats::getType


bool
sb_Cats::getDomain( string& val ) const
{
  if ( imp_->Domain_ == UNVALUED_STRING )
    return false;

  val = imp_->Domain_;

  return true;
} // sb_Cats::getDomain


bool
sb_Cats::getMap( string& val ) const
{
  if ( imp_->Map_ == UNVALUED_STRING )
    return false;

  val = imp_->Map_;

  return true;
} // sb_Cats::getMap


bool
sb_Cats::getThem( string& val ) const
{
  if ( imp_->Them_ == UNVALUED_STRING )
    return false;

  val = imp_->Them_;

  return true;
} // sb_Cats::getThem


bool
sb_Cats::getAggregateObject( string& val ) const
{
  if ( imp_->AggregateObject_ == UNVALUED_STRING )
    return false;

  val = imp_->AggregateObject_;

  return true;
} // sb_Cats::getAggregateObject


bool
sb_Cats::getAggregateObjectType( string& val ) const
{
  if ( imp_->AggregateObjectType_ == UNVALUED_STRING )
    return false;

  val = imp_->AggregateObjectType_;

  return true;
} // sb_Cats::getAggregateObjectType


bool
sb_Cats::getComment( string& val ) const
{
  if ( imp_->Comment_ == UNVALUED_STRING )
    return false;

  val = imp_->Comment_;

  return true;
} // sb_Cats::getComment





bool
sb_Cats::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "CATS" );

  record.back().setName( "Catalog/Spatial Domain" );

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


  if ( getDomain( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"DOMN", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "DOMN", sc_Subfield::is_A );
  }


  if ( getMap( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"MAP", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "MAP", sc_Subfield::is_A );
  }


  if ( getThem( tmp_str ) )
  {
     sb_Utils::add_subfield( record.back(),"THEM", tmp_str );
  }
  else
  {
     sb_Utils::add_empty_subfield( record.back(), "THEM", sc_Subfield::is_A );
  }


  if ( getAggregateObject( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"AGOB", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "AGOB", sc_Subfield::is_A );
  }


  if ( getAggregateObjectType( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"AGTP", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "AGTP", sc_Subfield::is_A );
  }


  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // Cats::getRecord




bool
sb_Cats::setName( string const& val )
{
  imp_->Name_ = val;

  return true;
} // sb_Cats::setName


bool
sb_Cats::setType( string const& val )
{
  imp_->Type_ = val;

  return true;
} // sb_Cats::setType


bool
sb_Cats::setDomain( string const& val )
{
  imp_->Domain_ = val;

  return true;
} // sb_Cats::setDomain


bool
sb_Cats::setMap( string const& val )
{
  imp_->Map_ = val;

  return true;
} // sb_Cats::setMap


bool
sb_Cats::setThem( string const& val )
{
  imp_->Them_ = val;

  return true;
} // sb_Cats::setThem


bool
sb_Cats::setAggregateObject( string const& val )
{
  imp_->AggregateObject_ = val;

  return true;
} // sb_Cats::setAggregateObject


bool
sb_Cats::setAggregateObjectType( string const& val )
{
  imp_->AggregateObjectType_ = val;

  return true;
} // sb_Cats::setAggregateObjectType


bool
sb_Cats::setComment( string const& val )
{
  imp_->Comment_ = val;

  return true;
} // sb_Cats::setComment


bool
sb_Cats::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Cats::setRecord




void
sb_Cats::unDefineName( )
{
  imp_->Name_ = UNVALUED_STRING;
} // sb_Cats::unDefineName


void
sb_Cats::unDefineType( )
{
  imp_->Type_ = UNVALUED_STRING;
} // sb_Cats::unDefineType


void
sb_Cats::unDefineDomain( )
{
  imp_->Domain_ = UNVALUED_STRING;
} // sb_Cats::unDefineDomain


void
sb_Cats::unDefineMap( )
{
  imp_->Map_ = UNVALUED_STRING;
} // sb_Cats::unDefineMap


void
sb_Cats::unDefineThem( )
{
  imp_->Them_ = UNVALUED_STRING;
} // sb_Cats::unDefineThem


void
sb_Cats::unDefineAggregateObject( )
{
  imp_->AggregateObject_ = UNVALUED_STRING;
} // sb_Cats::unDefineAggregateObject


void
sb_Cats::unDefineAggregateObjectType( )
{
  imp_->AggregateObjectType_ = UNVALUED_STRING;
} // sb_Cats::unDefineAggregateObjectType


void
sb_Cats::unDefineComment( )
{
  imp_->Comment_ = UNVALUED_STRING;
} // sb_Cats::unDefineComment



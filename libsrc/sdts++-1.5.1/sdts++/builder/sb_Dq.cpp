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
// $Id: sb_Dq.cpp,v 1.10 2002/11/24 22:07:42 mcoletti Exp $
//



#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <sdts++/builder/sb_Dq.h>


#include <iostream>
#include <strstream>

#include <limits.h>
#include <float.h>

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
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


static const char* ident_ = "$Id: sb_Dq.cpp,v 1.10 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Dq_Imp
{
  string   Comment_;


  sb_Dq_Imp()
    : 
    Comment_( UNVALUED_STRING )
  {}

};


sb_Dq::sb_Dq()
  : imp_( new sb_Dq_Imp() )
{
  setID( 1 );


  // insert static initializers

} // Dq ctor


sb_Dq::~sb_Dq()
{
  delete imp_;
} // Dq dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;


static sio_8211Schema dqhl__schema; // module specific schema
static sio_8211Schema dqpa__schema;
static sio_8211Schema dqaa__schema;
static sio_8211Schema dqlc__schema;
static sio_8211Schema dqcg__schema;


static
void
build_schema_( sio_8211Schema& schema, 
	       string const& name, 
	       string const& mnemonic )
{
  schema.push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema.back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( name );
  field_format.setTag( mnemonic );


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

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


} // build__schema


static
bool
ingest_record_( sb_Dq& dq,
		sb_Dq_Imp &dq_imp,
		sc_Record const& record,
		string const& mnemonic )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record, mnemonic, curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Dq::sb_Dq(sc_Record const&): "
	 << "Not an data quality record.";
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
    dq.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    dq.setID( tmp_int );
  }


  // COMT
  if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
  {
    cursubfield->getA( dq_imp.Comment_);
  }


  return true;


} // ingest__record




bool
sb_Dq::getComment( string& val ) const
{
  if ( imp_->Comment_ == UNVALUED_STRING )
    return false;

  val = imp_->Comment_;

  return true;
} // sb_Dq::getComment






bool
sb_Dq::setComment( string const& val )
{
  imp_->Comment_ = val;

  return true;
} // sb_Dq::setComment




void
sb_Dq::unDefineComment( )
{
  imp_->Comment_ = UNVALUED_STRING;
} // sb_Dq::unDefineComment



//
//
//


sb_Dqhl::sb_Dqhl()
{
  setMnemonic( "DQHL" );
} // sb_Dqhl ctor



bool
sb_Dqhl::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DQHL" );

  record.back().setName( "Lineage" );

  string tmp_str;

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif


  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // sb_Dqhl::getRecord



bool
sb_Dqhl::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, getImp(), record, "DQHL" );
} // sb_Dqhl::setRecord



void
sb_Dqhl::buildSpecificSchema_( )
{
   build_schema_( schema_(), "Lineage", "DQHL" );
} // sb_Dqhl::buildSpecificSchema_( )


static sio_8211Schema dqhl_schema_;


sio_8211Schema&
sb_Dqhl::schema_( )
{ return dqhl_schema_; }




//
//
//


sb_Dqpa::sb_Dqpa()
{
  setMnemonic( "DQPA" );
} // sb_Dqpa ctor



bool
sb_Dqpa::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DQPA" );

  record.back().setName( "Positional Accuracy" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // sb_Dqpa::getRecord



bool
sb_Dqpa::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, getImp(), record, "DQPA" );
} // sb_Dqpa::setRecord





void
sb_Dqpa::buildSpecificSchema_( )
{
   build_schema_( schema_(), "Positional Accuracy", "DQPA" );
} // sb_Dqhl::buildSpecificSchema_( )





static sio_8211Schema dqpa_schema_;

sio_8211Schema&
sb_Dqpa::schema_( )
{ return dqpa_schema_; }











//
//
//


sb_Dqaa::sb_Dqaa()
{
  setMnemonic( "DQAA" );
} // sb_Dqaa ctor



bool
sb_Dqaa::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DQAA" );

  record.back().setName( "Attribute Accuracy" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // sb_Dqaa::getRecord



bool
sb_Dqaa::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, getImp(), record, "DQAA" );
} // sb_Dqaa::setRecord







void
sb_Dqaa::buildSpecificSchema_( )
{
   build_schema_( schema_(), "Attribute Accuracy", "DQAA" );
} // sb_Dqhl::buildSpecificSchema_( )





static sio_8211Schema dqaa_schema_;

sio_8211Schema&
sb_Dqaa::schema_( )
{ return dqaa_schema_; }











//
//
//


sb_Dqlc::sb_Dqlc()
{
  setMnemonic( "DQLC" );
} // sb_Dqlc ctor



bool
sb_Dqlc::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DQLC" );

  record.back().setName( "Logical Consistency" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // sb_Dqlc::getRecord



bool
sb_Dqlc::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, getImp(), record, "DQLC" );
} // sb_Dqlc::setRecord








static sio_8211Schema dqlc_schema_;



void
sb_Dqlc::buildSpecificSchema_( )
{
   build_schema_( schema_(), "Logical Consistency", "DQLC" );
} // sb_Dqhl::buildSpecificSchema_( )



sio_8211Schema&
sb_Dqlc::schema_( )
{ return dqlc_schema_; }






//
//
//


sb_Dqcg::sb_Dqcg()
{
  setMnemonic( "DQCG" );
} // sb_Dqcg ctor



bool
sb_Dqcg::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DQCG" );

  record.back().setName( "Completeness" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  return true;


} // sb_Dqcg::getRecord



bool
sb_Dqcg::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, getImp(), record, "DQCG" );
} // sb_Dqcg::setRecord







void
sb_Dqcg::buildSpecificSchema_( )
{
   build_schema_( schema_(), "Completeness", "DQCG" );
} // sb_Dqcg::buildSpecificSchema_( )





static sio_8211Schema dqcg_schema_;



sio_8211Schema&
sb_Dqcg::schema_( )
{ return dqcg_schema_; }




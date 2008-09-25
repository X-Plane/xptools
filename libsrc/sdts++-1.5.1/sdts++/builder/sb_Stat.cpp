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
// $Id: sb_Stat.cpp,v 1.7 2002/11/24 22:07:43 mcoletti Exp $
//

#include <sdts++/builder/sb_Stat.h>



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


static const char* ident_ = "$Id: sb_Stat.cpp,v 1.7 2002/11/24 22:07:43 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Stat_Imp
{
  string   ModuleTypeReferred_;
  string   ModuleNameReferred_;
  long     ModuleRecordCount_;
  long     SpatialAddressCount_;


  sb_Stat_Imp()
    :
    ModuleTypeReferred_( UNVALUED_STRING ),
    ModuleNameReferred_( UNVALUED_STRING ),
    ModuleRecordCount_( UNVALUED_LONG ),
    SpatialAddressCount_( UNVALUED_LONG )
  {}

};


sb_Stat::sb_Stat()
  : imp_( new sb_Stat_Imp() )
{
  setMnemonic("STAT");
  setID( 1 );


  // insert static initializers

} // Stat ctor


sb_Stat::~sb_Stat()
{
  delete imp_;
} // Stat dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema schema_; // module specific schema


sio_8211Schema&
sb_Stat::schema_()
{
   return ::schema_;
} // sb_Stat::schema_()



void
sb_Stat::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Transfer Statistics" );
  field_format.setTag( "STAT" );


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

  field_format.back().setLabel( "MNTF" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MNRF" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NREC" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NSAD" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


} // build__schema


static
bool
ingest_record_( sb_Stat& stat, sb_Stat_Imp &stat_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"STAT",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Stat::sb_Stat(sc_Record const&): "
         << "Not an transfer statistics record.";
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
    stat.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    stat.setID( tmp_int );
  }


  // MNTF
  if (sb_Utils::getSubfieldByMnem(*curfield,"MNTF",cursubfield))
  {
    cursubfield->getA( stat_imp.ModuleTypeReferred_);
  }

  // MNRF
  if (sb_Utils::getSubfieldByMnem(*curfield,"MNRF",cursubfield))
  {
    cursubfield->getA( stat_imp.ModuleNameReferred_);
  }

  // NREC
  if (sb_Utils::getSubfieldByMnem(*curfield,"NREC",cursubfield))
  {
    cursubfield->getI( stat_imp.ModuleRecordCount_);
  }

  // NSAD
  if (sb_Utils::getSubfieldByMnem(*curfield,"NSAD",cursubfield))
  {
    cursubfield->getI( stat_imp.SpatialAddressCount_);
  }

  return true;


} // ingest__record




bool
sb_Stat::getModuleTypeReferred( string& val ) const
{
  if ( imp_->ModuleTypeReferred_ == UNVALUED_STRING )
    return false;

  val = imp_->ModuleTypeReferred_;

  return true;
} // sb_Stat::getModuleTypeReferred


bool
sb_Stat::getModuleNameReferred( string& val ) const
{
  if ( imp_->ModuleNameReferred_ == UNVALUED_STRING )
    return false;

  val = imp_->ModuleNameReferred_;

  return true;
} // sb_Stat::getModuleNameReferred


bool
sb_Stat::getModuleRecordCount( long& val ) const
{
  if ( imp_->ModuleRecordCount_ == UNVALUED_LONG )
    return false;

  val = imp_->ModuleRecordCount_;

  return true;
} // sb_Stat::getModuleRecordCount


bool
sb_Stat::getSpatialAddressCount( long& val ) const
{
  if ( imp_->SpatialAddressCount_ == UNVALUED_LONG )
    return false;

  val = imp_->SpatialAddressCount_;

  return true;
} // sb_Stat::getSpatialAddressCount






bool
sb_Stat::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "STAT" );

  record.back().setName( "Transfer Statistics" );

  string tmp_str;
  long   tmp_long;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getModuleTypeReferred( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"MNTF", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "MNTF", sc_Subfield::is_A );
  }


  if ( getModuleNameReferred( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"MNRF", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "MNRF", sc_Subfield::is_A );
  }


  if ( getModuleRecordCount( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"NREC", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "NREC", sc_Subfield::is_I );
  }


  if ( getSpatialAddressCount( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"NSAD", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "NSAD", sc_Subfield::is_I );
  }


  return true;


} // Stat::getRecord




bool
sb_Stat::setModuleTypeReferred( string const& val )
{
  imp_->ModuleTypeReferred_ = val;

  return true;
} // sb_Stat::setModuleTypeReferred


bool
sb_Stat::setModuleNameReferred( string const& val )
{
  imp_->ModuleNameReferred_ = val;

  return true;
} // sb_Stat::setModuleNameReferred


bool
sb_Stat::setModuleRecordCount( long val )
{
  imp_->ModuleRecordCount_ = val;

  return true;
} // sb_Stat::setModuleRecordCount


bool
sb_Stat::setSpatialAddressCount( long val )
{
  imp_->SpatialAddressCount_ = val;

  return true;
} // sb_Stat::setSpatialAddressCount


bool
sb_Stat::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Stat::setRecord




void
sb_Stat::unDefineModuleTypeReferred( )
{
  imp_->ModuleTypeReferred_ = UNVALUED_STRING;
} // sb_Stat::unDefineModuleTypeReferred


void
sb_Stat::unDefineModuleNameReferred( )
{
  imp_->ModuleNameReferred_ = UNVALUED_STRING;
} // sb_Stat::unDefineModuleNameReferred


void
sb_Stat::unDefineModuleRecordCount( )
{
  imp_->ModuleRecordCount_ = UNVALUED_LONG;
} // sb_Stat::unDefineModuleRecordCount


void
sb_Stat::unDefineSpatialAddressCount( )
{
  imp_->SpatialAddressCount_ = UNVALUED_LONG;
} // sb_Stat::unDefineSpatialAddressCount



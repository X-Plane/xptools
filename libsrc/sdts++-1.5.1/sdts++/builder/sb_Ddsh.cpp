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
// $Id: sb_Ddsh.cpp,v 1.9 2002/11/24 22:07:42 mcoletti Exp $
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <sdts++/builder/sb_Ddsh.h>


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


static const char* ident_ = "$Id: sb_Ddsh.cpp,v 1.9 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

static set<string> TYPE_domain;
static set<string> KEY_domain;



struct sb_Ddsh_Imp
{
  string   Name_;
  string   Type_;
  string   EntityLabel_;
  string   EntityAuthority_;
  string   AttributeLabel_;
  string   AttributeAuthority_;
  string   Format_;
  string   Unit_;
  double   Precision_;
  long     MaximumSubfieldLength_;
  string   Key_;


  sb_Ddsh_Imp()
    :
    Name_( UNVALUED_STRING ),
    Type_( UNVALUED_STRING ),
    EntityLabel_( UNVALUED_STRING ),
    EntityAuthority_( UNVALUED_STRING ),
    AttributeLabel_( UNVALUED_STRING ),
    AttributeAuthority_( UNVALUED_STRING ),
    Format_( UNVALUED_STRING ),
    Unit_( UNVALUED_STRING ),
    Precision_( UNVALUED_DOUBLE ),
    MaximumSubfieldLength_( UNVALUED_LONG ),
    Key_( UNVALUED_STRING )
    {}

};


sb_Ddsh::sb_Ddsh()
  : imp_( new sb_Ddsh_Imp() )
{
  setMnemonic("DDSH");
  setID( 1 );


  // initialize static domain types

  if ( TYPE_domain.empty() )
    {
      TYPE_domain.insert( "ATPR" );
      TYPE_domain.insert( "ATSC" );
      TYPE_domain.insert( "CELL" );
    }

  if ( KEY_domain.empty() )
    {
      KEY_domain.insert( "NOKEY" );
      KEY_domain.insert( "PKEY" );
      KEY_domain.insert( "FKEY" );
      KEY_domain.insert( "PFKEY" );
    }

} // Ddsh ctor


sb_Ddsh::~sb_Ddsh()
{
  delete imp_;
} // Ddsh dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;

static sio_8211Schema schema_; // module specific schema


sio_8211Schema&
sb_Ddsh::schema_()
{
   return ::schema_;
} // sb_Ddsh::schema_()




void
sb_Ddsh::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Data Dictionary/Schema" );
  field_format.setTag( "DDSH" );


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

  field_format.back().setLabel( "ETLB" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "EUTH" );
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

  field_format.back().setLabel( "FMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "UNIT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PREC" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MXLN" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "KEY" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


} // build__schema


static
bool
ingest_record_( sb_Ddsh& ddsh, sb_Ddsh_Imp &ddsh_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"DDSH",curfield) )
    {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Ddsh::sb_Ddsh(sc_Record const&): "
           << "Not an data dictionary/schema record.";
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
      ddsh.setMnemonic( tmp_str );
    }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
    {
      cursubfield->getI( tmp_int );
      ddsh.setID( tmp_int );
    }


  // NAME
  if (sb_Utils::getSubfieldByMnem(*curfield,"NAME",cursubfield))
    {
      cursubfield->getA( ddsh_imp.Name_);
    }


  // TYPE
  if (sb_Utils::getSubfieldByMnem(*curfield,"TYPE",cursubfield))
    {
      cursubfield->getA( ddsh_imp.Type_);
    }


  // ETLB
  if (sb_Utils::getSubfieldByMnem(*curfield,"ETLB",cursubfield))
    {
      cursubfield->getA( ddsh_imp.EntityLabel_);
    }


  // EUTH
  if (sb_Utils::getSubfieldByMnem(*curfield,"EUTH",cursubfield))
    {
      cursubfield->getA( ddsh_imp.EntityAuthority_);
    }


  // ATLB
  if (sb_Utils::getSubfieldByMnem(*curfield,"ATLB",cursubfield))
    {
      cursubfield->getA( ddsh_imp.AttributeLabel_);
    }


  // AUTH
  if (sb_Utils::getSubfieldByMnem(*curfield,"AUTH",cursubfield))
    {
      cursubfield->getA( ddsh_imp.AttributeAuthority_);
    }


  // FMT
  if (sb_Utils::getSubfieldByMnem(*curfield,"FMT",cursubfield))
    {
      cursubfield->getA( ddsh_imp.Format_);
    }


  // UNIT
  if (sb_Utils::getSubfieldByMnem(*curfield,"UNIT",cursubfield))
    {
      cursubfield->getA( ddsh_imp.Unit_);
    }


  // PREC
  if (sb_Utils::getSubfieldByMnem(*curfield,"PREC",cursubfield))
    {
      cursubfield->getR( ddsh_imp.Precision_);
    }


  // MXLN
  if (sb_Utils::getSubfieldByMnem(*curfield,"MXLN",cursubfield))
    {
      cursubfield->getI( ddsh_imp.MaximumSubfieldLength_);
    }


  // KEY
  if (sb_Utils::getSubfieldByMnem(*curfield,"KEY",cursubfield))
    {
      cursubfield->getA( ddsh_imp.Key_);
    }


  return true;


} // ingest_record_




bool
sb_Ddsh::getName( string& val ) const
{
  if ( imp_->Name_ == UNVALUED_STRING )
    return false;

  val = imp_->Name_;

  return true;
} // sb_Ddsh::getName


bool
sb_Ddsh::getType( string& val ) const
{
  if ( imp_->Type_ == UNVALUED_STRING )
    return false;

  val.assign( imp_->Type_.begin(), imp_->Type_.end() );

  return true;
} // sb_Ddsh::getType


bool
sb_Ddsh::getEntityLabel( string& val ) const
{
  if ( imp_->EntityLabel_ == UNVALUED_STRING )
    return false;

  val = imp_->EntityLabel_;

  return true;
} // sb_Ddsh::getEntityLabel


bool
sb_Ddsh::getEntityAuthority( string& val ) const
{
  if ( imp_->EntityAuthority_ == UNVALUED_STRING )
    return false;

  val = imp_->EntityAuthority_;

  return true;
} // sb_Ddsh::getEntityAuthority


bool
sb_Ddsh::getAttributeLabel( string& val ) const
{
  if ( imp_->AttributeLabel_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeLabel_;

  return true;
} // sb_Ddsh::getAttributeLabel


bool
sb_Ddsh::getAttributeAuthority( string& val ) const
{
  if ( imp_->AttributeAuthority_ == UNVALUED_STRING )
    return false;

  val = imp_->AttributeAuthority_;

  return true;
} // sb_Ddsh::getAttributeAuthority


bool
sb_Ddsh::getFormat( string& val ) const
{
  if ( imp_->Format_ == UNVALUED_STRING )
    return false;

  val = imp_->Format_;

  return true;
} // sb_Ddsh::getFormat


bool
sb_Ddsh::getUnit( string& val ) const
{
  if ( imp_->Unit_ == UNVALUED_STRING )
    return false;

  val = imp_->Unit_;

  return true;
} // sb_Ddsh::getUnit


bool
sb_Ddsh::getPrecision( double& val ) const
{
  if ( imp_->Precision_ == UNVALUED_DOUBLE )
    return false;

  val = imp_->Precision_;

  return true;
} // sb_Ddsh::getPrecision


bool
sb_Ddsh::getMaximumSubfieldLength( long& val ) const
{
  if ( imp_->MaximumSubfieldLength_ == UNVALUED_LONG )
    return false;

  val = imp_->MaximumSubfieldLength_;

  return true;
} // sb_Ddsh::getMaximumSubfieldLength


bool
sb_Ddsh::getKey( string& val ) const
{
  if ( imp_->Key_ == UNVALUED_STRING )
    return false;

  val = imp_->Key_;

  return true;
} // sb_Ddsh::getKey



bool
sb_Ddsh::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "DDSH" );
  record.back().setName( "Data Dictionary/Schema" );

  string tmp_str;
  double tmp_double;
  long   tmp_long;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getName( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"NAME", tmp_str );
    }
  else
    {
      return false;
    }


  if ( getType( tmp_str) )
    {

      if ( sb_Utils::valid_domain( tmp_str, TYPE_domain ) )
        {
          sb_Utils::add_subfield( record.back(),"TYPE", tmp_str );
        }
      else
        {
          return false;
        }
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(),"TYPE", sc_Subfield::is_A );
    }

  if ( getEntityLabel( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"ETLB", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(),"ETLB", sc_Subfield::is_A );
    }

  if ( getEntityAuthority( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"EUTH", tmp_str );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"EUTH", sc_Subfield::is_A );
  }

  if ( getAttributeLabel( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"ATLB", tmp_str );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"ATLB", sc_Subfield::is_A );
  }

  if ( getAttributeAuthority( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"AUTH", tmp_str );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"AUTH", sc_Subfield::is_A );
  }

  if ( getFormat( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"FMT", tmp_str );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"FMT", sc_Subfield::is_A );
  }

  if ( getUnit( tmp_str) )
    {
      sb_Utils::add_subfield( record.back(),"UNIT", tmp_str );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"UNIT" , sc_Subfield::is_A);
  }

  if ( getPrecision( tmp_double) )
    {
      sb_Utils::add_subfield( record.back(),"PREC", tmp_double );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"PREC", sc_Subfield::is_R );
  }

  if ( getMaximumSubfieldLength( tmp_long) )
    {
      sb_Utils::add_subfield( record.back(),"MXLN", tmp_long );
    }
  else
  {
      sb_Utils::add_empty_subfield( record.back(),"MXLN", sc_Subfield::is_I );
  }

  if ( getKey( tmp_str) )
    {
      if ( sb_Utils::valid_domain( tmp_str, KEY_domain ) )
        {
          sb_Utils::add_subfield( record.back(),"KEY", tmp_str );
        }
      else
        {
          return false;
        }
    }
  else
  {
    sb_Utils::add_empty_subfield( record.back(),"MXLN", sc_Subfield::is_A );
  }

  return true;


} // Ddsh::getRecord




bool
sb_Ddsh::setName( string const& val )
{
  imp_->Name_ = val;

  return true;
} // sb_Ddsh::setName


bool
sb_Ddsh::setType( string const& val )
{
  if ( sb_Utils::valid_domain( val, TYPE_domain ) )
    imp_->Type_ = val;
  else
    return false;

  return true;
} // sb_Ddsh::setType


bool
sb_Ddsh::setEntityLabel( string const& val )
{
  imp_->EntityLabel_ = val;

  return true;
} // sb_Ddsh::setEntityLabel


bool
sb_Ddsh::setEntityAuthority( string const& val )
{
  imp_->EntityAuthority_ = val;

  return true;
} // sb_Ddsh::setEntityAuthority


bool
sb_Ddsh::setAttributeLabel( string const& val )
{
  imp_->AttributeLabel_ = val;

  return true;
} // sb_Ddsh::setAttributeLabel


bool
sb_Ddsh::setAttributeAuthority( string const& val )
{
  imp_->AttributeAuthority_ = val;

  return true;
} // sb_Ddsh::setAttributeAuthority


bool
sb_Ddsh::setFormat( string const& val )
{
  imp_->Format_ = val;

  return true;
} // sb_Ddsh::setFormat


bool
sb_Ddsh::setUnit( string const& val )
{
  imp_->Unit_ = val;

  return true;
} // sb_Ddsh::setUnit


bool
sb_Ddsh::setPrecision( double val )
{
  imp_->Precision_ = val;

  return true;
} // sb_Ddsh::setPrecision


bool
sb_Ddsh::setMaximumSubfieldLength( long val )
{
  imp_->MaximumSubfieldLength_ = val;

  return true;
} // sb_Ddsh::setMaximumSubfieldLength


bool
sb_Ddsh::setKey( string const& val )
{
  if ( sb_Utils::valid_domain( val, KEY_domain ) )
    imp_->Key_ = val;
  else
    return false;

  return true;
} // sb_Ddsh::setKey


bool
sb_Ddsh::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Ddsh::setRecord



void
sb_Ddsh::unDefineName( )
{
   imp_->Name_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineName


void
sb_Ddsh::unDefineType( )
{
   imp_->Type_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineType


void
sb_Ddsh::unDefineEntityLabel( )
{
   imp_->EntityLabel_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineEntityLabel


void
sb_Ddsh::unDefineEntityAuthority( )
{
   imp_->EntityAuthority_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineEntityAuthority


void
sb_Ddsh::unDefineAttributeLabel( )
{
   imp_->AttributeLabel_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineAttributeLabel


void
sb_Ddsh::unDefineAttributeAuthority( )
{
   imp_->AttributeAuthority_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineAttributeAuthority


void
sb_Ddsh::unDefineFormat( )
{
   imp_->Format_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineFormat


void
sb_Ddsh::unDefineUnit( )
{
   imp_->Unit_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineUnit


void
sb_Ddsh::unDefinePrecision( )
{
   imp_->Precision_ = UNVALUED_DOUBLE;
} // sb_Ddsh::unDefinePrecision


void
sb_Ddsh::unDefineMaximumSubfieldLength( )
{
   imp_->MaximumSubfieldLength_ = UNVALUED_LONG;
} // sb_Ddsh::unDefineMaximumSubfieldLength


void
sb_Ddsh::unDefineKey( )
{
   imp_->Key_ = UNVALUED_STRING;
} // sb_Ddsh::unDefineKey



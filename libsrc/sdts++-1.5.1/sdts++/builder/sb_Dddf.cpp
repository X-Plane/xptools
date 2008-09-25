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
// $Id: sb_Dddf.cpp,v 1.4 2002/11/24 22:07:42 mcoletti Exp $
//

#include "builder/sb_Dddf.h"


#include <iostream>
#include <strstream>
#include <string>

using namespace std;

#include <limits.h>
#include <float.h>

#ifndef INCLUDED_SB_UTILS_H
#include "builder/sb_Utils.h"
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include "builder/sb_ForeignID.h"
#endif

#ifndef INCLUDED_SC_RECORD_H
#include "container/sc_Record.h"
#endif

#ifndef INCLUDED_SC_FIELD_H
#include "container/sc_Field.h"
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include "container/sc_Subfield.h"
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include "io/sio_8211Converter.h"
#endif


using namespace std;


static const char* _ident = "$Id: sb_Dddf.cpp,v 1.4 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Dddf_Imp
{
      string   _EntityOrAttribute;
      string   _Label;
      string   _Source;
      string   _Definition;
      string   _AttributeAuthority;
      string   _AttributeAuthorityDescription;


      sb_Dddf_Imp()
         : _EntityOrAttribute( UNVALUED_STRING ),
           _Label( UNVALUED_STRING ),
           _Source( UNVALUED_STRING ),
           _Definition( UNVALUED_STRING ),
           _AttributeAuthority( UNVALUED_STRING ),
           _AttributeAuthorityDescription( UNVALUED_STRING )
      {}

      void reset()
      {
         _EntityOrAttribute = UNVALUED_STRING;
         _Label = UNVALUED_STRING;
         _Source = UNVALUED_STRING;
         _Definition = UNVALUED_STRING;
         _AttributeAuthority = UNVALUED_STRING;
         _AttributeAuthorityDescription = UNVALUED_STRING;

      }

};// struct sb_Dddf_Imp


sb_Dddf::sb_Dddf()
   : _imp( new sb_Dddf_Imp() )
{
   setMnemonic("DDDF");
   setID( 1 );


// insert static initializers

} // Dddf ctor


sb_Dddf::~sb_Dddf()
{
   delete _imp;
} // Dddf dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema _schema; // module specific schema

static
void
_build_schema( sio_8211Schema& schema )
{
   schema.clear();               // make sure we are starting with clean schema

   schema.push_back( sio_8211FieldFormat() );

   sio_8211FieldFormat& field_format = schema.back();

   field_format.setDataStructCode( sio_8211FieldFormat::vector );
   field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
   field_format.setName( "Dddf" );
   field_format.setTag( "DDDF" );


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

   field_format.back().setLabel( "EORA" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "EALB" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "SRCE" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "DFIN" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "AUTH" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "ADSC" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );



} // _build_schema


static
bool
_ingest_record( sb_Dddf& dddf, sb_Dddf_Imp &dddf_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record,"DDDF",curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Dddf::sb_Dddf(sc_Record const&): "
           << "Not an data dictionary/definition record.";
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
      dddf.setMnemonic( tmp_str );
   }


// RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      dddf.setID( tmp_int );
   }


// EORA
   if (sb_Utils::getSubfieldByMnem(*curfield,"EORA",cursubfield))
   {
      cursubfield->getA( dddf_imp._EntityOrAttribute);
   }
   else
   {
//      return false;
   }


// EALB
   if (sb_Utils::getSubfieldByMnem(*curfield,"EALB",cursubfield))
   {
      cursubfield->getA( dddf_imp._Label);
   }
   else
   {
//      return false;
   }


// SRCE
   if (sb_Utils::getSubfieldByMnem(*curfield,"SRCE",cursubfield))
   {
      cursubfield->getA( dddf_imp._Source);
   }
   else
   {
//      return false;
   }


// DFIN
   if (sb_Utils::getSubfieldByMnem(*curfield,"DFIN",cursubfield))
   {
      cursubfield->getA( dddf_imp._Definition);
   }
   else
   {
//      return false;
   }


// AUTH
   if (sb_Utils::getSubfieldByMnem(*curfield,"AUTH",cursubfield))
   {
      cursubfield->getA( dddf_imp._AttributeAuthority);
   }
   else
   {
//      return false;
   }


// ADSC
   if (sb_Utils::getSubfieldByMnem(*curfield,"ADSC",cursubfield))
   {
      cursubfield->getA( dddf_imp._AttributeAuthorityDescription);
   }
   else
   {
//      return false;
   }


   return true;


} // _ingest_record




bool
sb_Dddf::getEntityOrAttribute( string & val ) const
{
   if ( _imp->_EntityOrAttribute == UNVALUED_STRING )
      return false;

   val = _imp->_EntityOrAttribute;

   return true;

} // sb_Dddf::getEntityOrAttribute


bool
sb_Dddf::getLabel( string & val ) const
{
   if ( _imp->_Label == UNVALUED_STRING )
      return false;

   val = _imp->_Label;

   return true;

} // sb_Dddf::getLabel


bool
sb_Dddf::getSource( string & val ) const
{
   if ( _imp->_Source == UNVALUED_STRING )
      return false;

   val = _imp->_Source;

   return true;

} // sb_Dddf::getSource


bool
sb_Dddf::getDefinition( string & val ) const
{
   if ( _imp->_Definition == UNVALUED_STRING )
      return false;

   val = _imp->_Definition;

   return true;

} // sb_Dddf::getDefinition


bool
sb_Dddf::getAttributeAuthority( string & val ) const
{
   if ( _imp->_AttributeAuthority == UNVALUED_STRING )
      return false;

   val = _imp->_AttributeAuthority;

   return true;

} // sb_Dddf::getAttributeAuthority


bool
sb_Dddf::getAttributeAuthorityDescription( string & val ) const
{
   if ( _imp->_AttributeAuthorityDescription == UNVALUED_STRING )
      return false;

   val = _imp->_AttributeAuthorityDescription;

   return true;

} // sb_Dddf::getAttributeAuthorityDescription


bool
sb_Dddf::getSchema( sio_8211Schema& schema ) const
{
// If the schema hasn't been
// initialized, please do so.

   if ( _schema.empty() )
   {
      _build_schema( _schema );
   }

   if ( _schema.empty() )   // oops ... something screwed up
   {
      return false;
   }

   schema = _schema;

   return true;

} // sb_Dddf::getSchema




bool
sb_Dddf::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

// first field, which contains module name and record number

   sb_ForeignID tmp_foreign_id;

   record.push_back( sc_Field() );

   record.back().setMnemonic( "DDDF" );

   record.back().setName( "Dddf" );

   string tmp_str;

   getMnemonic( tmp_str );
   sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
   sb_Utils::add_subfield( record.back(), "RCID", getID() );

   if ( getEntityOrAttribute( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"EORA", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "EORA", sc_Subfield::is_A );
   }


   if ( getLabel( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"EALB", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "EALB", sc_Subfield::is_A );
   }


   if ( getSource( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"SRCE", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "SRCE", sc_Subfield::is_A );
   }


   if ( getDefinition( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"DFIN", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "DFIN", sc_Subfield::is_A );
   }


   if ( getAttributeAuthority( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"AUTH", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "AUTH", sc_Subfield::is_A );
   }


   if ( getAttributeAuthorityDescription( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"ADSC", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "ADSC", sc_Subfield::is_A );
   }


   return true;


} // Dddf::getRecord




bool
sb_Dddf::setEntityOrAttribute( string const& val )
{
   _imp->_EntityOrAttribute = val;

   return true;
} // sb_Dddf::setEntityOrAttribute


bool
sb_Dddf::setLabel( string const& val )
{
   _imp->_Label = val;

   return true;
} // sb_Dddf::setLabel


bool
sb_Dddf::setSource( string const& val )
{
   _imp->_Source = val;

   return true;
} // sb_Dddf::setSource


bool
sb_Dddf::setDefinition( string const& val )
{
   _imp->_Definition = val;

   return true;
} // sb_Dddf::setDefinition


bool
sb_Dddf::setAttributeAuthority( string const& val )
{
   _imp->_AttributeAuthority = val;

   return true;
} // sb_Dddf::setAttributeAuthority


bool
sb_Dddf::setAttributeAuthorityDescription( string const& val )
{
   _imp->_AttributeAuthorityDescription = val;

   return true;
} // sb_Dddf::setAttributeAuthorityDescription


bool
sb_Dddf::setRecord( sc_Record const& record )
{
   _imp->reset();      // reset to new state; i.e., clear out foreign
   // identifiers, etc.
   return _ingest_record( *this, *_imp, record );
} // sb_Dddf::setRecord




void
sb_Dddf::unDefineEntityOrAttribute( )
{
   _imp->_EntityOrAttribute = UNVALUED_STRING;
} // sb_Dddf::unDefineEntityOrAttribute


void
sb_Dddf::unDefineLabel( )
{
   _imp->_Label = UNVALUED_STRING;
} // sb_Dddf::unDefineLabel


void
sb_Dddf::unDefineSource( )
{
   _imp->_Source = UNVALUED_STRING;
} // sb_Dddf::unDefineSource


void
sb_Dddf::unDefineDefinition( )
{
   _imp->_Definition = UNVALUED_STRING;
} // sb_Dddf::unDefineDefinition


void
sb_Dddf::unDefineAttributeAuthority( )
{
   _imp->_AttributeAuthority = UNVALUED_STRING;
} // sb_Dddf::unDefineAttributeAuthority


void
sb_Dddf::unDefineAttributeAuthorityDescription( )
{
   _imp->_AttributeAuthorityDescription = UNVALUED_STRING;
} // sb_Dddf::unDefineAttributeAuthorityDescription


sio_8211Schema&
sb_Dddf::schema_()
{
   if ( _schema.empty() )
   {
      buildSpecificSchema_();
   }

   return _schema;
} // sb_Dddf::schema_()



void
sb_Dddf::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Dddf::buildSpecificSchema_()

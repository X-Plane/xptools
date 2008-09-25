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
// $Id: sb_Comp.cpp,v 1.4 2002/10/11 19:55:50 mcoletti Exp $
//

#include "builder/sb_Comp.h"


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

#ifndef INCLUDED_SIO_CONVERTERFACTORY_H
#include "io/sio_ConverterFactory.h"
#endif



static const char* _ident = "$Id: sb_Comp.cpp,v 1.4 2002/10/11 19:55:50 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Comp_Imp
{
      string   _ObjectRepresentation;
      sb_AttributeIDs   _AttributeIDs;
      sb_ForeignIDs   _ForeignIDs;
      sb_ForeignIDs   _CompositeIDs;


      sb_Comp_Imp()
         : _ObjectRepresentation( UNVALUED_STRING )
      {}

      void reset()
      {
         _ObjectRepresentation = UNVALUED_STRING;
         _AttributeIDs.clear ();
         _ForeignIDs.clear ();
         _CompositeIDs.clear ();

      }

};// struct sb_Comp_Imp


sb_Comp::sb_Comp()
   : _imp( new sb_Comp_Imp() )
{
   setMnemonic("COMP");
   setID( 1 );


// insert static initializers

} // Comp ctor


sb_Comp::~sb_Comp()
{
} // Comp dtor





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
   field_format.setName( "Comp" );
   field_format.setTag( "COMPOSITE" );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "MODN" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "RCID" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "OBRP" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ));


   sb_ForeignID  foreign_id;
   sb_AttributeID attribute_id;

   attribute_id.addFieldToSchema( schema, "AttributeID", "ATID", true );
   foreign_id.addFieldToSchema( schema, "ForeignID", "FRID", true );
   foreign_id.addFieldToSchema( schema, "CompositeID", "CPID", true );

} // _build_schema


static
bool
_ingest_record( sb_Comp& comp, sb_Comp_Imp &comp_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record,"COMP",curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Comp::sb_Comp(sc_Record const&): "
           << "Not an composite record.";
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
      comp.setMnemonic( tmp_str );
   }


// RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      comp.setID( tmp_int );
   }


// OBRP
   if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
   {
      cursubfield->getA( comp_imp._ObjectRepresentation);
   }
   else
   {
      return false;
   }


// ATID
   if ( sb_Utils::getFieldByMnem( record,"ATID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anAttributeID
      while ( curfield != record.end() && curfield->mnemonic() == "ATID" )
      {

         comp_imp._AttributeIDs.push_back( sb_AttributeID() );
         if ( ! comp_imp._AttributeIDs.back().assign( *curfield ) )
         {

            return false;
         }

         curfield++;
      }
   }
   else
   {
      // this is an optional filed, so no worries if we get here
   }


// FRID
   if ( sb_Utils::getFieldByMnem( record,"FRID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anForeignID
      while ( curfield != record.end() && curfield->mnemonic() == "FRID" )
      {

         comp_imp._ForeignIDs.push_back( sb_ForeignID() );
         if ( ! comp_imp._ForeignIDs.back().assign( *curfield ) )
         {

            return false;
         }

         curfield++;
      }
   }
   else
   {
      // this is an optional filed, so no worries if we get here
   }


// CPID
   if ( sb_Utils::getFieldByMnem( record,"CPID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anCompositeID
      while ( curfield != record.end() && curfield->mnemonic() == "CPID" )
      {

         comp_imp._CompositeIDs.push_back( sb_ForeignID() );
         if ( ! comp_imp._CompositeIDs.back().assign( *curfield ) )
         {

            return false;
         }

         curfield++;
      }
   }
   else
   {
      // this is an optional filed, so no worries if we get here
   }


   return true;


} // _ingest_record




bool
sb_Comp::getObjectRepresentation( string & val ) const
{
   if ( _imp->_ObjectRepresentation == UNVALUED_STRING )
      return false;

   val = _imp->_ObjectRepresentation;

   return true;

} // sb_Comp::getObjectRepresentation


bool
sb_Comp::getAttributeID( std::list<std::string> & val ) const
{
   if ( _imp->_AttributeIDs.empty() )
      return false;

// We'll let the programmer worry about the proper maintanence of
// the given list of strings.  HTat is, it'll be up to them to
// clear the contents or not before invoking this member function.
   string tmp_string;

   for ( sb_AttributeIDs::const_iterator i =_imp->_AttributeIDs.begin(); i != _imp->_AttributeIDs.end();i++ )
   {
      if ( ! i->packedIdentifierString( tmp_string ) )
      {
         return false;
      }
      val.push_back( tmp_string );
   }
   return true;
} //sb_Comp::getAttributeID(std::string& val)


bool
sb_Comp::getAttributeID( sb_AttributeIDs & val ) const
{

   val = _imp->_AttributeIDs;
   return true;

} // sb_Comp::getAttributeID


bool
sb_Comp::getForeignID( std::list<std::string> & val ) const
{
   if ( _imp->_ForeignIDs.empty() )
      return false;

// We'll let the programmer worry about the proper maintanence of
// the given list of strings.  HTat is, it'll be up to them to
// clear the contents or not before invoking this member function.
   string tmp_string;

   for ( sb_ForeignIDs::const_iterator i =_imp->_ForeignIDs.begin(); i != _imp->_ForeignIDs.end();i++ )
   {
      if ( ! i->packedIdentifierString( tmp_string ) )
      {
         return false;
      }
      val.push_back( tmp_string );
   }
   return true;
} //sb_Comp::getForeignID(std::string& val)


bool
sb_Comp::getForeignID( sb_ForeignIDs & val ) const
{

   val = _imp->_ForeignIDs;
   return true;

} // sb_Comp::getForeignID


bool
sb_Comp::getCompositeID( std::list<std::string> & val ) const
{
   if ( _imp->_CompositeIDs.empty() )
      return false;

// We'll let the programmer worry about the proper maintanence of
// the given list of strings.  HTat is, it'll be up to them to
// clear the contents or not before invoking this member function.
   string tmp_string;

   for ( sb_ForeignIDs::const_iterator i =_imp->_CompositeIDs.begin(); i != _imp->_CompositeIDs.end();i++ )
   {
      if ( ! i->packedIdentifierString( tmp_string ) )
      {
         return false;
      }
      val.push_back( tmp_string );
   }
   return true;
} //sb_Comp::getCompositeID(std::string& val)


bool
sb_Comp::getCompositeID( sb_ForeignIDs & val ) const
{

   val = _imp->_CompositeIDs;
   return true;

} // sb_Comp::getCompositeID


bool
sb_Comp::getSchema( sio_8211Schema& schema ) const
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

} // sb_Comp::getSchema




bool
sb_Comp::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

// first field, which contains module name and record number

   sb_ForeignID tmp_foreign_id;

   record.push_back( sc_Field() );

   record.back().setMnemonic( "COMP" );

   record.back().setName( "Composite" );

   string tmp_str;

   getMnemonic( tmp_str );
   sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
   sb_Utils::add_subfield( record.back(), "RCID", getID() );

   if ( getObjectRepresentation( tmp_str ) )
   {
      sb_Utils::add_subfield( record.back(),"OBRP", tmp_str );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "OBRP", sc_Subfield::is_A );
   }


   for ( sb_AttributeIDs::const_iterator i = _imp->_AttributeIDs.begin();
         i != _imp->_AttributeIDs.end(); i++ )
   {
      sb_Utils::add_foreignID ( record, *i );
   }


   for ( sb_ForeignIDs::const_iterator j = _imp->_ForeignIDs.begin();
         j != _imp->_ForeignIDs.end();
         j++ )
   {
      sb_Utils::add_foreignID ( record, *j );
   }


   for ( sb_ForeignIDs::const_iterator k = _imp->_CompositeIDs.begin();
         k != _imp->_CompositeIDs.end();
         k++ )
   {
      sb_Utils::add_foreignID ( record, *k );
   }


   return true;


} // Comp::getRecord




bool
sb_Comp::setObjectRepresentation( string const& val )
{
   _imp->_ObjectRepresentation = val;

   return true;
} // sb_Comp::setObjectRepresentation


bool
sb_Comp::setAttributeID( sb_AttributeIDs const& val )
{
   _imp->_AttributeIDs = val;

   return true;
} // sb_Comp::setAttributeID


bool
sb_Comp::setForeignID( sb_ForeignIDs const& val )
{
   _imp->_ForeignIDs = val;

   return true;
} // sb_Comp::setForeignID


bool
sb_Comp::setCompositeID( sb_ForeignIDs const& val )
{
   _imp->_CompositeIDs = val;

   return true;
} // sb_Comp::setCompositeID


bool
sb_Comp::setRecord( sc_Record const& record )
{
   _imp->reset();      // reset to new state; i.e., clear out foreign
   // identifiers, etc.
   return _ingest_record( *this, *_imp, record );
} // sb_Comp::setRecord




void
sb_Comp::unDefineObjectRepresentation( )
{
   _imp->_ObjectRepresentation = UNVALUED_STRING;
} // sb_Comp::unDefineObjectRepresentation


void
sb_Comp::unDefineAttributeID( )
{
   _imp->_AttributeIDs.clear();
} // sb_Comp::unDefineAttributeID


void
sb_Comp::unDefineForeignID( )
{
   _imp->_ForeignIDs.clear();
} // sb_Comp::unDefineForeignID


void
sb_Comp::unDefineCompositeID( )
{
   _imp->_CompositeIDs.clear();
} // sb_Comp::unDefineCompositeID


sio_8211Schema&
sb_Comp::schema_()
{
   if ( _schema.empty() )
   {
      buildSpecificSchema_();
   }

   return _schema;
} // sb_Comp::schema_()



void
sb_Comp::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Comp::buildSpecificSchema_()

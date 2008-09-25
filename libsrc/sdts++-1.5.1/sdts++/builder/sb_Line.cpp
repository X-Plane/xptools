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
// $Id: sb_Line.cpp,v 1.24 2002/10/11 19:55:50 mcoletti Exp $
//

#include "builder/sb_Line.h"


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



static const char* _ident = "$Id: sb_Line.cpp,v 1.24 2002/10/11 19:55:50 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Line_Imp
{
      string   _ObjectRepresentation;
      sb_AttributeIDs   _AttributeIDs;
      sb_ForeignID   _PolygonIDLeft;
      sb_ForeignID   _PolygonIDRight;
      sb_ForeignID   _StartNodeID;
      sb_ForeignID   _EndNodeID;
      sb_ForeignIDs   _ChainComponentIDs;
      sb_Spatials  _SpatialAddress;
      sb_ForeignIDs   _CompositeIDs;
      sb_ForeignIDs   _RepresentationModuleIDs;


      sb_Line_Imp()
         : _ObjectRepresentation( UNVALUED_STRING ),
           _PolygonIDLeft( "PolygonIDLeft", "PIDL" ),
           _PolygonIDRight( "PolygonIDRight", "PIDR" ),
           _StartNodeID( "StartNodeID", "SNID" ),
           _EndNodeID( "EndNodeID", "ENID" )
      {}

      void reset()
      {
         _ObjectRepresentation = UNVALUED_STRING;
         _AttributeIDs.clear ();
         _PolygonIDLeft.moduleName ( UNVALUED_STRING );
         _PolygonIDRight.moduleName ( UNVALUED_STRING );
         _StartNodeID.moduleName ( UNVALUED_STRING );
         _EndNodeID.moduleName ( UNVALUED_STRING );
         _ChainComponentIDs.clear ();
         _SpatialAddress.clear();
         _CompositeIDs.clear ();
         _RepresentationModuleIDs.clear ();

      }

};// struct sb_Line_Imp


sb_Line::sb_Line()
   : _imp( new sb_Line_Imp() )
{
   setMnemonic("LINE");
   setID( 1 );


// insert static initializers

} // Line ctor


sb_Line::~sb_Line()
{
} // Line dtor





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
   field_format.setName( "Line" );
   field_format.setTag( "LINE" );


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
   foreign_id.addFieldToSchema( schema, "PolygonIDLeft", "PIDL", true );
   foreign_id.addFieldToSchema( schema, "PolygonIDRight", "PIDR", true );
   foreign_id.addFieldToSchema( schema, "StartNodeID", "SNID", true );
   foreign_id.addFieldToSchema( schema, "EndNodeID", "ENID", true );
   foreign_id.addFieldToSchema( schema, "ChainComponentID", "CCID", true );

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "SADR" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );
   field_format.setIsRepeating( true );


   foreign_id.addFieldToSchema( schema, "CompositeID", "CPID", true );
   foreign_id.addFieldToSchema( schema, "RepresentationModuleID", "RPID", true );

} // _build_schema


static
bool
_ingest_record( sb_Line& line, sb_Line_Imp &line_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record,"LINE",curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Line::sb_Line(sc_Record const&): "
           << "Not an line record.";
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
      line.setMnemonic( tmp_str );
   }


// RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      line.setID( tmp_int );
   }


// OBRP
   if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
   {
      cursubfield->getA( line_imp._ObjectRepresentation);
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

         line_imp._AttributeIDs.push_back( sb_AttributeID() );
         if ( ! line_imp._AttributeIDs.back().assign( *curfield ) )
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




//PIDL
   if ( sb_Utils::getFieldByMnem( record, line_imp._PolygonIDLeft.mnemonic(),curfield) )
   {
      if ( ! line_imp._PolygonIDLeft.assign( *curfield ) )
         return false;
   }
   else
   {
// XXX Add sanity checking for whenPolygonIDLeft field is required
   }


//PIDR
   if ( sb_Utils::getFieldByMnem( record, line_imp._PolygonIDRight.mnemonic(),curfield) )
   {
      if ( ! line_imp._PolygonIDRight.assign( *curfield ) )
         return false;
   }
   else
   {
// XXX Add sanity checking for whenPolygonIDRight field is required
   }


//SNID
   if ( sb_Utils::getFieldByMnem( record, line_imp._StartNodeID.mnemonic(),curfield) )
   {
      if ( ! line_imp._StartNodeID.assign( *curfield ) )
         return false;
   }
   else
   {
// XXX Add sanity checking for whenStartNodeID field is required
   }


//ENID
   if ( sb_Utils::getFieldByMnem( record, line_imp._EndNodeID.mnemonic(),curfield) )
   {
      if ( ! line_imp._EndNodeID.assign( *curfield ) )
         return false;
   }
   else
   {
// XXX Add sanity checking for whenEndNodeID field is required
   }
// CCID
   if ( sb_Utils::getFieldByMnem( record,"CCID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anChainComponentID
      while ( curfield != record.end() && curfield->mnemonic() == "CCID" )
      {

         line_imp._ChainComponentIDs.push_back( sb_ForeignID() );
         if ( ! line_imp._ChainComponentIDs.back().assign( *curfield ) )
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



//SADR
//SADR are typically repeating fields and so will be stored as
//consolidatedSADRsc_Fields.  Therfore we'll set an sb_Spatial
//with the current (X,Y) from the currentSADR, add it to the
//_SpatialAddress container, and then increment to the nextSADR,
//if any
   if ( sb_Utils::getFieldByMnem( record,"SADR" ,curfield) )
   {
      // XXX consider optimizing this loop
      do
      {
         sb_Spatial tmp_spatial;

         for ( cursubfield = curfield->begin();
               cursubfield != curfield->end();
               cursubfield++ )
         {
            if ( cursubfield->mnemonic() == "X" )
            {
               tmp_spatial.x() = *cursubfield;
            }
            else if  ( cursubfield->mnemonic() == "Y" )
            {
               tmp_spatial.y() = *cursubfield;
            }
            else
            {
               return false;    // not a valid SADR subfield
            }
         }

         line_imp._SpatialAddress.push_back( tmp_spatial );

         curfield++;	//increment to next field, if any

      } while ( curfield != record.end() && curfield->mnemonic() == "SADR" );

   }
   else
   {
      return false;
   }

// CPID
   if ( sb_Utils::getFieldByMnem( record,"CPID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anCompositeID
      while ( curfield != record.end() && curfield->mnemonic() == "CPID" )
      {

         line_imp._CompositeIDs.push_back( sb_ForeignID() );
         if ( ! line_imp._CompositeIDs.back().assign( *curfield ) )
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


// RPID
   if ( sb_Utils::getFieldByMnem( record,"RPID",curfield) )
   {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anRepresentationModuleID
      while ( curfield != record.end() && curfield->mnemonic() == "RPID" )
      {

         line_imp._RepresentationModuleIDs.push_back( sb_ForeignID() );
         if ( ! line_imp._RepresentationModuleIDs.back().assign( *curfield ) )
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
sb_Line::getObjectRepresentation( string & val ) const
{
   if ( _imp->_ObjectRepresentation == UNVALUED_STRING )
      return false;

   val = _imp->_ObjectRepresentation;

   return true;

} // sb_Line::getObjectRepresentation


bool
sb_Line::getAttributeID( std::list<std::string> & val ) const
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
} //sb_Line::getAttributeID(std::string& val)


bool
sb_Line::getAttributeID( sb_AttributeIDs & val ) const
{

   val = _imp->_AttributeIDs;
   return true;

} // sb_Line::getAttributeID


bool
sb_Line::getPolygonIDLeft( string & val ) const
{
   if ( _imp->_PolygonIDLeft.moduleName() == UNVALUED_STRING )
      return false;

   return _imp->_PolygonIDLeft.packedIdentifierString( val );
} // sb_Line::getPolygonIDLeft


bool
sb_Line::getPolygonIDLeft( sb_ForeignID& val ) const
{
   if ( _imp->_PolygonIDLeft.moduleName() == UNVALUED_STRING )
      return false;

   val = _imp->_PolygonIDLeft;

   return true;

} // sb_Line::getPolygonIDLeft


bool
sb_Line::getPolygonIDRight( string & val ) const
{
   if ( _imp->_PolygonIDRight.moduleName() == UNVALUED_STRING )
      return false;

   return _imp->_PolygonIDRight.packedIdentifierString( val );
} // sb_Line::getPolygonIDRight


bool
sb_Line::getPolygonIDRight( sb_ForeignID& val ) const
{
   if ( _imp->_PolygonIDRight.moduleName() == UNVALUED_STRING )
      return false;

   val = _imp->_PolygonIDRight;

   return true;

} // sb_Line::getPolygonIDRight


bool
sb_Line::getStartNodeID( string & val ) const
{
   if ( _imp->_StartNodeID.moduleName() == UNVALUED_STRING )
      return false;

   return _imp->_StartNodeID.packedIdentifierString( val );
} // sb_Line::getStartNodeID


bool
sb_Line::getStartNodeID( sb_ForeignID& val ) const
{
   if ( _imp->_StartNodeID.moduleName() == UNVALUED_STRING )
      return false;

   val = _imp->_StartNodeID;

   return true;

} // sb_Line::getStartNodeID


bool
sb_Line::getEndNodeID( string & val ) const
{
   if ( _imp->_EndNodeID.moduleName() == UNVALUED_STRING )
      return false;

   return _imp->_EndNodeID.packedIdentifierString( val );
} // sb_Line::getEndNodeID


bool
sb_Line::getEndNodeID( sb_ForeignID& val ) const
{
   if ( _imp->_EndNodeID.moduleName() == UNVALUED_STRING )
      return false;

   val = _imp->_EndNodeID;

   return true;

} // sb_Line::getEndNodeID


bool
sb_Line::getChainComponentID( std::list<std::string> & val ) const
{
   if ( _imp->_ChainComponentIDs.empty() )
      return false;

// We'll let the programmer worry about the proper maintanence of
// the given list of strings.  HTat is, it'll be up to them to
// clear the contents or not before invoking this member function.
   string tmp_string;

   for ( sb_ForeignIDs::const_iterator i =_imp->_ChainComponentIDs.begin(); i != _imp->_ChainComponentIDs.end();i++ )
   {
      if ( ! i->packedIdentifierString( tmp_string ) )
      {
         return false;
      }
      val.push_back( tmp_string );
   }
   return true;
} //sb_Line::getChainComponentID(std::string& val)


bool
sb_Line::getChainComponentID( sb_ForeignIDs & val ) const
{

   val = _imp->_ChainComponentIDs;
   return true;

} // sb_Line::getChainComponentID


bool
sb_Line::getSpatialAddress( sb_Spatials & val ) const
{
   if ( _imp->_SpatialAddress.empty()  )
      return false;

   val = _imp->_SpatialAddress;

   return true;

} // sb_Line::getSpatialAddress


bool
sb_Line::getCompositeID( std::list<std::string> & val ) const
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
} //sb_Line::getCompositeID(std::string& val)


bool
sb_Line::getCompositeID( sb_ForeignIDs & val ) const
{

   val = _imp->_CompositeIDs;
   return true;

} // sb_Line::getCompositeID


bool
sb_Line::getRepresentationModuleID( std::list<std::string> & val ) const
{
   if ( _imp->_RepresentationModuleIDs.empty() )
      return false;

// We'll let the programmer worry about the proper maintanence of
// the given list of strings.  HTat is, it'll be up to them to
// clear the contents or not before invoking this member function.
   string tmp_string;

   for ( sb_ForeignIDs::const_iterator i =_imp->_RepresentationModuleIDs.begin(); i != _imp->_RepresentationModuleIDs.end();i++ )
   {
      if ( ! i->packedIdentifierString( tmp_string ) )
      {
         return false;
      }
      val.push_back( tmp_string );
   }
   return true;
} //sb_Line::getRepresentationModuleID(std::string& val)


bool
sb_Line::getRepresentationModuleID( sb_ForeignIDs & val ) const
{

   val = _imp->_RepresentationModuleIDs;
   return true;

} // sb_Line::getRepresentationModuleID


bool
sb_Line::getSchema( sio_8211Schema& schema ) const
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

} // sb_Line::getSchema




bool
sb_Line::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

// first field, which contains module name and record number

   sb_ForeignID tmp_foreign_id;

   record.push_back( sc_Field() );

   record.back().setMnemonic( "LINE" );

   record.back().setName( "Line" );

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


   if ( getPolygonIDLeft(tmp_foreign_id ) )
   {
      sb_Utils::add_foreignID ( record, tmp_foreign_id );
   }
   else
   {
//NOP because it's an optional field
   }


   if ( getPolygonIDRight(tmp_foreign_id ) )
   {
      sb_Utils::add_foreignID ( record, tmp_foreign_id );
   }
   else
   {
//NOP because it's an optional field
   }


   if ( getStartNodeID(tmp_foreign_id ) )
   {
      sb_Utils::add_foreignID ( record, tmp_foreign_id );
   }
   else
   {
//NOP because it's an optional field
   }


   if ( getEndNodeID(tmp_foreign_id ) )
   {
      sb_Utils::add_foreignID ( record, tmp_foreign_id );
   }
   else
   {
//NOP because it's an optional field
   }


   for ( sb_ForeignIDs::const_iterator j = _imp->_ChainComponentIDs.begin();
         j != _imp->_ChainComponentIDs.end();
         j++ )
   {
      sb_Utils::add_foreignID ( record, *j );
   }


// for each spatial address, add a SpatialAddress to a repeating field

   for ( sb_Spatials::const_iterator k = _imp->_SpatialAddress.begin();
         k != _imp->_SpatialAddress.end();
         k++ )
   {
//just shove the sc_subfields directly onto the end of the field

      sb_Utils::add_field( record, "SpatialAddress","SADR" );

      record.back().push_back( (*k).x() );
      record.back().push_back( (*k).y() );
   }


   for ( sb_ForeignIDs::const_iterator l = _imp->_CompositeIDs.begin();
         l != _imp->_CompositeIDs.end();
         l++ )
   {
      sb_Utils::add_foreignID ( record, *l );
   }


   for ( sb_ForeignIDs::const_iterator m = _imp->_RepresentationModuleIDs.begin();
         m != _imp->_RepresentationModuleIDs.end();
         m++ )
   {
      sb_Utils::add_foreignID ( record, *m );
   }


   return true;


} // Line::getRecord




bool
sb_Line::setObjectRepresentation( string const& val )
{
   _imp->_ObjectRepresentation = val;

   return true;
} // sb_Line::setObjectRepresentation


bool
sb_Line::setAttributeID( sb_AttributeIDs const& val )
{
   _imp->_AttributeIDs = val;

   return true;
} // sb_Line::setAttributeID


bool
sb_Line::setPolygonIDLeft( sb_ForeignID const& val )
{
   _imp->_PolygonIDLeft = val;

   return true;
} // sb_Line::setPolygonIDLeft


bool
sb_Line::setPolygonIDRight( sb_ForeignID const& val )
{
   _imp->_PolygonIDRight = val;

   return true;
} // sb_Line::setPolygonIDRight


bool
sb_Line::setStartNodeID( sb_ForeignID const& val )
{
   _imp->_StartNodeID = val;

   return true;
} // sb_Line::setStartNodeID


bool
sb_Line::setEndNodeID( sb_ForeignID const& val )
{
   _imp->_EndNodeID = val;

   return true;
} // sb_Line::setEndNodeID


bool
sb_Line::setChainComponentID( sb_ForeignIDs const& val )
{
   _imp->_ChainComponentIDs = val;

   return true;
} // sb_Line::setChainComponentID


bool
sb_Line::setSpatialAddress( sb_Spatials const& val )
{
   _imp->_SpatialAddress = val;

   return true;
} // sb_Line::setSpatialAddress


bool
sb_Line::setCompositeID( sb_ForeignIDs const& val )
{
   _imp->_CompositeIDs = val;

   return true;
} // sb_Line::setCompositeID


bool
sb_Line::setRepresentationModuleID( sb_ForeignIDs const& val )
{
   _imp->_RepresentationModuleIDs = val;

   return true;
} // sb_Line::setRepresentationModuleID


bool
sb_Line::setRecord( sc_Record const& record )
{
   _imp->reset();      // reset to new state; i.e., clear out foreign
   // identifiers, etc.
   return _ingest_record( *this, *_imp, record );
} // sb_Line::setRecord




void
sb_Line::unDefineObjectRepresentation( )
{
   _imp->_ObjectRepresentation = UNVALUED_STRING;
} // sb_Line::unDefineObjectRepresentation


void
sb_Line::unDefineAttributeID( )
{
   _imp->_AttributeIDs.clear();
} // sb_Line::unDefineAttributeID


void
sb_Line::unDefinePolygonIDLeft( )
{
   _imp->_PolygonIDLeft.moduleName( UNVALUED_STRING );
} // sb_Line::unDefinePolygonIDLeft


void
sb_Line::unDefinePolygonIDRight( )
{
   _imp->_PolygonIDRight.moduleName( UNVALUED_STRING );
} // sb_Line::unDefinePolygonIDRight


void
sb_Line::unDefineStartNodeID( )
{
   _imp->_StartNodeID.moduleName( UNVALUED_STRING );
} // sb_Line::unDefineStartNodeID


void
sb_Line::unDefineEndNodeID( )
{
   _imp->_EndNodeID.moduleName( UNVALUED_STRING );
} // sb_Line::unDefineEndNodeID


void
sb_Line::unDefineChainComponentID( )
{
   _imp->_ChainComponentIDs.clear();
} // sb_Line::unDefineChainComponentID


void
sb_Line::unDefineSpatialAddress( )
{
   _imp->_SpatialAddress.clear();
} // sb_Line::unDefineSpatialAddress


void
sb_Line::unDefineCompositeID( )
{
   _imp->_CompositeIDs.clear();
} // sb_Line::unDefineCompositeID


void
sb_Line::unDefineRepresentationModuleID( )
{
   _imp->_RepresentationModuleIDs.clear();
} // sb_Line::unDefineRepresentationModuleID


sio_8211Schema&
sb_Line::schema_()
{
   if ( _schema.empty() )
   {
      buildSpecificSchema_();
   }

   return _schema;
} // sb_Line::schema_()



void
sb_Line::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Line::buildSpecificSchema_()

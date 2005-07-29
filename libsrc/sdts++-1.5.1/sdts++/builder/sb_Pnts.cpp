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
// $Id: sb_Pnts.cpp,v 1.2 2002/11/24 22:07:42 mcoletti Exp $
//

#include "builder/sb_Pnts.h"


#include <iostream>
#include <strstream>
#include <string>


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


using namespace std;


static const char* _ident = "$Id: sb_Pnts.cpp,v 1.2 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Pnts_Imp
{
  string   _ObjectRepresentation;
  sb_Spatial   _SpatialAddress;
  sb_AttributeIDs   _AttributeIDs;
  sb_ForeignIDs   _LineIDs;
  sb_ForeignIDs   _AreaIDs;
  sb_ForeignIDs   _CompositeIDs;
  sb_ForeignIDs   _RepresentationModuleIDs;
  sb_ForeignIDs   _OrientationSpatialAddresss;
  sb_ForeignIDs   _AttributePrimaryForeignIDs;
  sb_AttributeIDs   _ArrtibuteLabels;
  sb_Spatial   _SymbolOrientationSpatialAddress;


  sb_Pnts_Imp()
    : _ObjectRepresentation( UNVALUED_STRING )
    {}

  void reset()
    {
      _ObjectRepresentation = UNVALUED_STRING;
      _SpatialAddress.x().setUnvalued();
      _SpatialAddress.y().setUnvalued();
      _SpatialAddress.z().setUnvalued();
      _AttributeIDs.clear ();
      _LineIDs.clear ();
      _AreaIDs.clear ();
      _CompositeIDs.clear ();
      _RepresentationModuleIDs.clear ();
      _OrientationSpatialAddresss.clear ();
      _AttributePrimaryForeignIDs.clear ();
      _ArrtibuteLabels.clear ();
      _SymbolOrientationSpatialAddress.x().setUnvalued();
      _SymbolOrientationSpatialAddress.y().setUnvalued();
      _SymbolOrientationSpatialAddress.z().setUnvalued();

    }

};// struct sb_Pnts_Imp


sb_Pnts::sb_Pnts()
  : _imp( new sb_Pnts_Imp() )
{
  setMnemonic("PNTS");
  setID( 1 );


  // insert static initializers

} // Pnts ctor


sb_Pnts::~sb_Pnts()
{
} // Pnts dtor





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
  field_format.setName( "Pnts" );
  field_format.setTag( "POINT" );


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



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SADR" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );
  field_format.setIsRepeating( false );


  sb_ForeignID  foreign_id;
  sb_AttributeID attribute_id;

  attribute_id.addFieldToSchema( schema, "AttributeID", "ATID", true );
  foreign_id.addFieldToSchema( schema, "LineID", "LNID", true );
  foreign_id.addFieldToSchema( schema, "AreaID", "ARID", true );
  foreign_id.addFieldToSchema( schema, "CompositeID", "CPID", true );
  foreign_id.addFieldToSchema( schema, "RepresentationModuleID", "RPID", true );
  foreign_id.addFieldToSchema( schema, "OrientationSpatialAddress", "OSAD", true );
  foreign_id.addFieldToSchema( schema, "AttributePrimaryForeignID", "PAID", true );
  attribute_id.addFieldToSchema( schema, "ArrtibuteLabel", "ATLB", true );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SSAD" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );
  field_format.setIsRepeating( false );



} // _build_schema


static
bool
_ingest_record( sb_Pnts& pnts, sb_Pnts_Imp &pnts_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;
  
  if ( ! sb_Utils::getFieldByMnem( record,"PNTS",curfield) )
    {
      #ifdef SDTSXX_DEBUG
      cerr << "sb_Pnts::sb_Pnts(sc_Record const&): "
	   << "Not an point record.";
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
      pnts.setMnemonic( tmp_str );
    }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
    {
      cursubfield->getI( tmp_int );
      pnts.setID( tmp_int );
    }


  // OBRP
  if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
    {
      cursubfield->getA( pnts_imp._ObjectRepresentation);
    }
  else
    {
      return false;
    }



  //SADR
  if ( sb_Utils::getFieldByMnem( record,"SADR" ,curfield) )
    {
      sb_Spatial tmp_spatial;
      
      if ( sb_Utils::getSubfieldByMnem(*curfield,"X",cursubfield ) )
	{
	  tmp_spatial.x() = *cursubfield;
	}
      else
	{
	  return false;
	}
      
      if ( sb_Utils::getSubfieldByMnem(*curfield,"Y",cursubfield ) )
	{
	  tmp_spatial.y() = *cursubfield;
	}
      else
	{
	  return false;
	}

      pnts_imp._SpatialAddress.assign( tmp_spatial );

    }

  // ATID
  if ( sb_Utils::getFieldByMnem( record,"ATID",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anAttributeID
      while ( curfield != record.end() && curfield->mnemonic() == "ATID" )
	{

	  pnts_imp._AttributeIDs.push_back( sb_AttributeID() );
	  if ( ! pnts_imp._AttributeIDs.back().assign( *curfield ) )
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


  // LNID
  if ( sb_Utils::getFieldByMnem( record,"LNID",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anLineID
      while ( curfield != record.end() && curfield->mnemonic() == "LNID" )
	{
	  
	  pnts_imp._LineIDs.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._LineIDs.back().assign( *curfield ) )
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


  // ARID
  if ( sb_Utils::getFieldByMnem( record,"ARID",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anAreaID
      while ( curfield != record.end() && curfield->mnemonic() == "ARID" )
	{

	  pnts_imp._AreaIDs.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._AreaIDs.back().assign( *curfield ) )
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

	  pnts_imp._CompositeIDs.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._CompositeIDs.back().assign( *curfield ) )
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
	  
	  pnts_imp._RepresentationModuleIDs.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._RepresentationModuleIDs.back().assign( *curfield ) )
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


  // OSAD
  if ( sb_Utils::getFieldByMnem( record,"OSAD",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anOrientationSpatialAddress
      while ( curfield != record.end() && curfield->mnemonic() == "OSAD" )
	{

	  pnts_imp._OrientationSpatialAddresss.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._OrientationSpatialAddresss.back().assign( *curfield ) )
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


  // PAID
  if ( sb_Utils::getFieldByMnem( record,"PAID",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anAttributePrimaryForeignID
      while ( curfield != record.end() && curfield->mnemonic() == "PAID" )
	{

	  pnts_imp._AttributePrimaryForeignIDs.push_back( sb_ForeignID() );
	  if ( ! pnts_imp._AttributePrimaryForeignIDs.back().assign( *curfield ) )
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


  // ATLB
  if ( sb_Utils::getFieldByMnem( record,"ATLB",curfield) )
    {
      //As this is potentially a repeating field, we just keep
      //grabbing fields until we get one that's well, not anArrtibuteLabel
      while ( curfield != record.end() && curfield->mnemonic() == "ATLB" )
	{
	  
	  pnts_imp._ArrtibuteLabels.push_back( sb_AttributeID() );
	  if ( ! pnts_imp._ArrtibuteLabels.back().assign( *curfield ) )
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



  //SSAD
  if ( sb_Utils::getFieldByMnem( record,"SSAD" ,curfield) )
    {
      sb_Spatial tmp_spatial;
      
      if ( sb_Utils::getSubfieldByMnem(*curfield,"X",cursubfield ) )
	{
	  tmp_spatial.x() = *cursubfield;
	}
      else
	{
	  return false;
	}
      
      if ( sb_Utils::getSubfieldByMnem(*curfield,"Y",cursubfield ) )
	{
	  tmp_spatial.y() = *cursubfield;
	}
      else
	{
	  return false;
	}

      pnts_imp._SymbolOrientationSpatialAddress.assign( tmp_spatial );
      
    }

  return true;

} // _ingest_record




bool
sb_Pnts::getObjectRepresentation( string & val ) const
{
  if ( _imp->_ObjectRepresentation == UNVALUED_STRING )
    return false;

  val = _imp->_ObjectRepresentation;

  return true;
} // sb_Pnts::getObjectRepresentation


bool
sb_Pnts::getSpatialAddress( sb_Spatial & val ) const
{
  if ( _imp->_SpatialAddress.x().isUnvalued()  )
    return false;

  val = _imp->_SpatialAddress;

  return true;
} // sb_Pnts::getSpatialAddress


bool
sb_Pnts::getAttributeID( std::list<std::string> & val ) const
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
} //sb_Pnts::getAttributeID(std::string& val)


bool
sb_Pnts::getAttributeID( sb_AttributeIDs & val ) const
{
  val = _imp->_AttributeIDs;
  return true;
} // sb_Pnts::getAttributeID


bool
sb_Pnts::getLineID( std::list<std::string> & val ) const
{
  if ( _imp->_LineIDs.empty() )
    return false;

  // We'll let the programmer worry about the proper maintanence of
  // the given list of strings.  HTat is, it'll be up to them to
  // clear the contents or not before invoking this member function.
  string tmp_string;

  for ( sb_ForeignIDs::const_iterator i =_imp->_LineIDs.begin(); i != _imp->_LineIDs.end();i++ )
    {
      if ( ! i->packedIdentifierString( tmp_string ) )
	{
	  return false;
	}
      val.push_back( tmp_string );
    }
  return true;
} //sb_Pnts::getLineID(std::string& val)


bool
sb_Pnts::getLineID( sb_ForeignIDs & val ) const
{
  val = _imp->_LineIDs;
  return true;
} // sb_Pnts::getLineID


bool
sb_Pnts::getAreaID( std::list<std::string> & val ) const
{
  if ( _imp->_AreaIDs.empty() )
    return false;
  
  // We'll let the programmer worry about the proper maintanence of
  // the given list of strings.  HTat is, it'll be up to them to
  // clear the contents or not before invoking this member function.
  string tmp_string;
  
  for ( sb_ForeignIDs::const_iterator i =_imp->_AreaIDs.begin(); i != _imp->_AreaIDs.end();i++ )
    {
      if ( ! i->packedIdentifierString( tmp_string ) )
	{
	  return false;
	}
      val.push_back( tmp_string );
    }
  return true;
} //sb_Pnts::getAreaID(std::string& val)


bool
sb_Pnts::getAreaID( sb_ForeignIDs & val ) const
{
  val = _imp->_AreaIDs;
  return true;
} // sb_Pnts::getAreaID


bool
sb_Pnts::getCompositeID( std::list<std::string> & val ) const
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
} //sb_Pnts::getCompositeID(std::string& val)


bool
sb_Pnts::getCompositeID( sb_ForeignIDs & val ) const
{
  val = _imp->_CompositeIDs;
  return true;
} // sb_Pnts::getCompositeID


bool
sb_Pnts::getRepresentationModuleID( std::list<std::string> & val ) const
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
} //sb_Pnts::getRepresentationModuleID(std::string& val)


bool
sb_Pnts::getRepresentationModuleID( sb_ForeignIDs & val ) const
{
  val = _imp->_RepresentationModuleIDs;
  return true;
} // sb_Pnts::getRepresentationModuleID


bool
sb_Pnts::getOrientationSpatialAddress( std::list<std::string> & val ) const
{
  if ( _imp->_OrientationSpatialAddresss.empty() )
    return false;

  // We'll let the programmer worry about the proper maintanence of
  // the given list of strings.  HTat is, it'll be up to them to
  // clear the contents or not before invoking this member function.
  string tmp_string;
  
  for ( sb_ForeignIDs::const_iterator i =_imp->_OrientationSpatialAddresss.begin(); i != _imp->_OrientationSpatialAddresss.end();i++ )
    {
      if ( ! i->packedIdentifierString( tmp_string ) )
	{
	  return false;
	}
      val.push_back( tmp_string );
    }
  return true;
} //sb_Pnts::getOrientationSpatialAddress(std::string& val)


bool
sb_Pnts::getOrientationSpatialAddress( sb_ForeignIDs & val ) const
{
  val = _imp->_OrientationSpatialAddresss;
  return true;
} // sb_Pnts::getOrientationSpatialAddress


bool
sb_Pnts::getAttributePrimaryForeignID( std::list<std::string> & val ) const
{
  if ( _imp->_AttributePrimaryForeignIDs.empty() )
    return false;

  // We'll let the programmer worry about the proper maintanence of
  // the given list of strings.  HTat is, it'll be up to them to
  // clear the contents or not before invoking this member function.
  string tmp_string;

  for ( sb_ForeignIDs::const_iterator i =_imp->_AttributePrimaryForeignIDs.begin(); i != _imp->_AttributePrimaryForeignIDs.end();i++ )
{
  if ( ! i->packedIdentifierString( tmp_string ) )
    {
      return false;
    }
  val.push_back( tmp_string );
}
  return true;
} //sb_Pnts::getAttributePrimaryForeignID(std::string& val)


bool
sb_Pnts::getAttributePrimaryForeignID( sb_ForeignIDs & val ) const
{
  val = _imp->_AttributePrimaryForeignIDs;
  return true;
} // sb_Pnts::getAttributePrimaryForeignID


bool
sb_Pnts::getArrtibuteLabel( std::list<std::string> & val ) const
{
  if ( _imp->_ArrtibuteLabels.empty() )
    return false;

  // We'll let the programmer worry about the proper maintanence of
  // the given list of strings.  HTat is, it'll be up to them to
  // clear the contents or not before invoking this member function.
  string tmp_string;

  for ( sb_AttributeIDs::const_iterator i =_imp->_ArrtibuteLabels.begin(); i != _imp->_ArrtibuteLabels.end();i++ )
    {
      if ( ! i->packedIdentifierString( tmp_string ) )
	{
	  return false;
	}
      val.push_back( tmp_string );
    }
  return true;
} //sb_Pnts::getArrtibuteLabel(std::string& val)


bool
sb_Pnts::getArrtibuteLabel( sb_AttributeIDs & val ) const
{
  val = _imp->_ArrtibuteLabels;
  return true;
} // sb_Pnts::getArrtibuteLabel


bool
sb_Pnts::getSymbolOrientationSpatialAddress( sb_Spatial & val ) const
{
  if ( _imp->_SymbolOrientationSpatialAddress.x().isUnvalued()  )
    return false;
  
  val = _imp->_SymbolOrientationSpatialAddress;
  
  return true;
} // sb_Pnts::getSymbolOrientationSpatialAddress


bool
sb_Pnts::getSchema( sio_8211Schema& schema ) const
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

} // sb_Pnts::getSchema




bool
sb_Pnts::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number
  
  sb_ForeignID tmp_foreign_id;

  record.push_back( sc_Field() );
  
  record.back().setMnemonic( "PNTS" );

  record.back().setName( "Point" );

  string tmp_str;
  double tmp_double;
  long   tmp_long;

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


  //XXX for each spatial address, add a SpatialAddress to a repeating field
  
  sb_Utils::add_field( record, "SpatialAddress","SADR" );

  {
    //just shove the sc_subfields directly onto the end of the field

    cout << "X:"<< _imp->_SpatialAddress.x() << endl;
    cout << "Y:"<<  _imp->_SpatialAddress.y() << endl;

    record.back().push_back(  _imp->_SpatialAddress.x() );
    record.back().push_back(  _imp->_SpatialAddress.y() );
  }


  for ( sb_AttributeIDs::const_iterator i = _imp->_AttributeIDs.begin();
	i != _imp->_AttributeIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_LineIDs.begin();
	i != _imp->_LineIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_AreaIDs.begin();
	i != _imp->_AreaIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_CompositeIDs.begin();
	i != _imp->_CompositeIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_RepresentationModuleIDs.begin();
	i != _imp->_RepresentationModuleIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_OrientationSpatialAddresss.begin();
	i != _imp->_OrientationSpatialAddresss.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_ForeignIDs::const_iterator i = _imp->_AttributePrimaryForeignIDs.begin();
	i != _imp->_AttributePrimaryForeignIDs.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  for ( sb_AttributeIDs::const_iterator i = _imp->_ArrtibuteLabels.begin();
	i != _imp->_ArrtibuteLabels.end(); i++ )
    {
      sb_Utils::add_foreignID ( record, *i );
    }


  //XXX for each spatial address, add a SymbolOrientationSpatialAddress to a repeating field
  
  sb_Utils::add_field( record, "SymbolOrientationSpatialAddress","SSAD" );
  
  {
    //just shove the sc_subfields directly onto the end of the field

    cout << "X:"<< _imp->_SymbolOrientationSpatialAddress.x() << endl;
    cout << "Y:"<<  _imp->_SymbolOrientationSpatialAddress.y() << endl;

    record.back().push_back(  _imp->_SymbolOrientationSpatialAddress.x() );
    record.back().push_back(  _imp->_SymbolOrientationSpatialAddress.y() );
  }

  return true;

} // Pnts::getRecord




bool
sb_Pnts::setObjectRepresentation( string const& val )
{
  _imp->_ObjectRepresentation = val;
  
  return true;
} // sb_Pnts::setObjectRepresentation


bool
sb_Pnts::setSpatialAddress( sb_Spatial const& val )
{
  _imp->_SpatialAddress = val;
  
  return true;
} // sb_Pnts::setSpatialAddress


bool
sb_Pnts::setAttributeID( sb_AttributeIDs const& val )
{
  _imp->_AttributeIDs = val;

  return true;
} // sb_Pnts::setAttributeID


bool
sb_Pnts::setLineID( sb_ForeignIDs const& val )
{
  _imp->_LineIDs = val;

  return true;
} // sb_Pnts::setLineID


bool
sb_Pnts::setAreaID( sb_ForeignIDs const& val )
{
  _imp->_AreaIDs = val;

  return true;
} // sb_Pnts::setAreaID


bool
sb_Pnts::setCompositeID( sb_ForeignIDs const& val )
{
  _imp->_CompositeIDs = val;
  
  return true;
} // sb_Pnts::setCompositeID


bool
sb_Pnts::setRepresentationModuleID( sb_ForeignIDs const& val )
{
  _imp->_RepresentationModuleIDs = val;

  return true;
} // sb_Pnts::setRepresentationModuleID


bool
sb_Pnts::setOrientationSpatialAddress( sb_ForeignIDs const& val )
{
  _imp->_OrientationSpatialAddresss = val;

  return true;
} // sb_Pnts::setOrientationSpatialAddress


bool
sb_Pnts::setAttributePrimaryForeignID( sb_ForeignIDs const& val )
{
  _imp->_AttributePrimaryForeignIDs = val;

  return true;
} // sb_Pnts::setAttributePrimaryForeignID


bool
sb_Pnts::setArrtibuteLabel( sb_AttributeIDs const& val )
{
  _imp->_ArrtibuteLabels = val;

  return true;
} // sb_Pnts::setArrtibuteLabel


bool
sb_Pnts::setSymbolOrientationSpatialAddress( sb_Spatial const& val )
{
  _imp->_SymbolOrientationSpatialAddress = val;

  return true;
} // sb_Pnts::setSymbolOrientationSpatialAddress


bool
sb_Pnts::setRecord( sc_Record const& record )
{
  _imp->reset();      // reset to new state; i.e., clear out foreign
                       // identifiers, etc.
  return _ingest_record( *this, *_imp, record );
} // sb_Pnts::setRecord




void
sb_Pnts::unDefineObjectRepresentation( )
{
  _imp->_ObjectRepresentation = UNVALUED_STRING;
} // sb_Pnts::unDefineObjectRepresentation


void
sb_Pnts::unDefineSpatialAddress( )
{
  _imp->_SpatialAddress.x().setUnvalued();
  _imp->_SpatialAddress.y().setUnvalued();
  _imp->_SpatialAddress.z().setUnvalued();
} // sb_Pnts::unDefineSpatialAddress


void
sb_Pnts::unDefineAttributeID( )
{
  _imp->_AttributeIDs.clear();
} // sb_Pnts::unDefineAttributeID


void
sb_Pnts::unDefineLineID( )
{
  _imp->_LineIDs.clear();
} // sb_Pnts::unDefineLineID


void
sb_Pnts::unDefineAreaID( )
{
  _imp->_AreaIDs.clear();
} // sb_Pnts::unDefineAreaID


void
sb_Pnts::unDefineCompositeID( )
{
  _imp->_CompositeIDs.clear();
} // sb_Pnts::unDefineCompositeID


void
sb_Pnts::unDefineRepresentationModuleID( )
{
  _imp->_RepresentationModuleIDs.clear();
} // sb_Pnts::unDefineRepresentationModuleID


void
sb_Pnts::unDefineOrientationSpatialAddress( )
{
  _imp->_OrientationSpatialAddresss.clear();
} // sb_Pnts::unDefineOrientationSpatialAddress


void
sb_Pnts::unDefineAttributePrimaryForeignID( )
{
  _imp->_AttributePrimaryForeignIDs.clear();
} // sb_Pnts::unDefineAttributePrimaryForeignID


void
sb_Pnts::unDefineArrtibuteLabel( )
{
  _imp->_ArrtibuteLabels.clear();
} // sb_Pnts::unDefineArrtibuteLabel


void
sb_Pnts::unDefineSymbolOrientationSpatialAddress( )
{
  _imp->_SymbolOrientationSpatialAddress.x().setUnvalued();
  _imp->_SymbolOrientationSpatialAddress.y().setUnvalued();
  _imp->_SymbolOrientationSpatialAddress.z().setUnvalued();
} // sb_Pnts::unDefineSymbolOrientationSpatialAddress


sio_8211Schema& 
sb_Pnts::schema_()
{
  if ( _schema.empty() )
    {
      buildSpecificSchema_();
    }

  return _schema;
} // sb_Pnts::schema_()



void
sb_Pnts::buildSpecificSchema_()
{
  _build_schema( _schema );
} // sb_Pnts::buildSpecificSchema_()

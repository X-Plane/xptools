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
// $Id: sb_Poly.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $
//

#include "builder/sb_Poly.h"


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



static const char* _ident = "$Id: sb_Poly.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Poly_Imp
{
string   _ObjectRepresentation;
sb_AttributeIDs   _AttributeIDs;
sb_ForeignIDs   _RingIDs;
sb_ForeignIDs   _ChainIDs;
sb_ForeignIDs   _CompositeIDs;
sb_ForeignIDs   _RepresentationIDs;


sb_Poly_Imp()
: _ObjectRepresentation( UNVALUED_STRING )
{}

void reset()
{
_ObjectRepresentation = UNVALUED_STRING;
_AttributeIDs.clear ();
_RingIDs.clear ();
_ChainIDs.clear ();
_CompositeIDs.clear ();
_RepresentationIDs.clear ();

}

};// struct sb_Poly_Imp


sb_Poly::sb_Poly()
 : _imp( new sb_Poly_Imp() )
{
setMnemonic("POLY");
setID( 1 );


// insert static initializers

} // Poly ctor


sb_Poly::~sb_Poly()
{
delete _imp;
} // Poly dtor




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
  field_format.setName( "Poly" );
  field_format.setTag( "POLY" );


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

  field_format.back().setLabel( "OBRP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


sb_ForeignID  foreign_id;
sb_AttributeID attribute_id;

  attribute_id.addFieldToSchema( schema, "AttributeID" , "ATID" , true );
  foreign_id.addFieldToSchema( schema, "RingID" , "RFID" , true );
  foreign_id.addFieldToSchema( schema, "ChainID" , "CHID" , true );
  foreign_id.addFieldToSchema( schema, "CompositeID" , "CPID" , true );
  foreign_id.addFieldToSchema( schema, "RepresentationID" , "RPID" , true );

} // _build_schema


static
bool
_ingest_record( sb_Poly& poly, sb_Poly_Imp &poly_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

sc_FieldCntr::const_iterator curfield;

if ( ! sb_Utils::getFieldByMnem( record,"POLY",curfield) )
{
#ifdef SDTSXX_DEBUG
cerr << "sb_Poly::sb_Poly(sc_Record const&): "
<< "Not an polygon record.";
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
poly.setMnemonic( tmp_str );
}


// RCID
if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
{
cursubfield->getI( tmp_int );
poly.setID( tmp_int );
}


// OBRP
if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
{
  cursubfield->getA( poly_imp._ObjectRepresentation);
}
else
{
  return false;
}

  // return if we've exhausted all fields in the given record
  if ( curfield == record.end () )
    { return true; }

// ATID
if ( sb_Utils::getFieldByMnem( record,"ATID",curfield) )
 	{
	//As this is potentially a repeating field, we just keep
	//grabbing fileds until we get one that's well, not anAttributeID
	  while ( curfield != record.end() && curfield->mnemonic() == "ATID" )
	    {

	      poly_imp._AttributeIDs.push_back( sb_AttributeID() );
	      if ( ! poly_imp._AttributeIDs.back().assign( *curfield ) )
		{

		  return false;
		}

	      curfield++;
	    }
	}
else
{
  //this is an optional filed, so no worries if we get here
}

// return if we've exhausted all fields in the given record
  if ( curfield == record.end () )
    { return true; }

// RFID
if ( sb_Utils::getFieldByMnem( record,"RFID",curfield) )
{
  //As this is potentially a repeating field, we just keep
  //grabbing fileds until we get one that's well, not anRingID
  while (  curfield != record.end() && curfield->mnemonic() == "RFID" )
    {

      poly_imp._RingIDs.push_back( sb_ForeignID() );
      if ( ! poly_imp._RingIDs.back().assign( *curfield ) )
	{

	  return false;
	}

      curfield++;
    }
}
else
{
  //this is an optional filed, so no worries if we get here
}

// return if we've exhausted all fields in the given record
  if ( curfield == record.end () )
    { return true; }

// CHID
if (  sb_Utils::getFieldByMnem( record,"CHID",curfield) )
{
  //As this is potentially a repeating field, we just keep
  //grabbing fileds until we get one that's well, not anChainID
  while (  curfield != record.end() && curfield->mnemonic() == "CHID" )
    {

      poly_imp._ChainIDs.push_back( sb_ForeignID() );
      if ( ! poly_imp._ChainIDs.back().assign( *curfield ) )
	{

	  return false;
	}

      curfield++;
    }
}
else
{
  //this is an optional filed, so no worries if we get here
}

// return if we've exhausted all fields in the given record
  if ( curfield == record.end () )
    { return true; }

// CPID
if ( sb_Utils::getFieldByMnem( record,"CPID",curfield) )
{
  //As this is potentially a repeating field, we just keep
  //grabbing fileds until we get one that's well, not anCompositeID
  while (  curfield != record.end() && curfield->mnemonic() == "CPID" )
    {

      poly_imp._CompositeIDs.push_back( sb_ForeignID() );
      if ( ! poly_imp._CompositeIDs.back().assign( *curfield ) )
	{

	  return false;
	}

      curfield++;
    }
}
else
{
  //this is an optional filed, so no worries if we get here
}

// return if we've exhausted all fields in the given record
  if ( curfield == record.end () )
    { return true; }

// RPID
if ( sb_Utils::getFieldByMnem( record,"RPID",curfield) )
{
  //As this is potentially a repeating field, we just keep
  //grabbing fileds until we get one that's well, not anRepresentationID
  while (  curfield != record.end() && curfield->mnemonic() == "RPID" )
    {
      
      poly_imp._RepresentationIDs.push_back( sb_ForeignID() );
      if ( ! poly_imp._RepresentationIDs.back().assign( *curfield ) )
	{

	  return false;
	}

      curfield++;
    }
}
else
{
  //this is an optional filed, so no worries if we get here
}


return true;


} // _ingest_record




bool
sb_Poly::getObjectRepresentation( string & val ) const
{
if ( _imp->_ObjectRepresentation == UNVALUED_STRING )
return false;

val = _imp->_ObjectRepresentation;

return true;

} // sb_Poly::getObjectRepresentation


bool
sb_Poly::getAttributeID( std::list<std::string> & val ) const
{
if ( _imp->_AttributeIDs.empty() )
   return false;

//We'll let the programmer worry about the proper maintanence of
//the given list of strings.  HTat is, it'll be up to them to
//clear the contents or not before invoking this member function.
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
} //sb_Poly::getAttributeID(std::string& val)


bool
sb_Poly::getAttributeID( sb_AttributeIDs & val ) const
{

   val = _imp->_AttributeIDs;
return true;

} // sb_Poly::getAttributeID


bool
sb_Poly::getRingID( std::list<std::string> & val ) const
{
if ( _imp->_RingIDs.empty() )
   return false;

//We'll let the programmer worry about the proper maintanence of
//the given list of strings.  HTat is, it'll be up to them to
//clear the contents or not before invoking this member function.
string tmp_string;

for ( sb_ForeignIDs::const_iterator i =_imp->_RingIDs.begin(); i != _imp->_RingIDs.end();i++ )
{
   if ( ! i->packedIdentifierString( tmp_string ) )
   {
   return false;
   }
   val.push_back( tmp_string );
}
return true;
} //sb_Poly::getRingID(std::string& val)


bool
sb_Poly::getRingID( sb_ForeignIDs & val ) const
{

   val = _imp->_RingIDs;
return true;

} // sb_Poly::getRingID


bool
sb_Poly::getChainID( std::list<std::string> & val ) const
{
if ( _imp->_ChainIDs.empty() )
   return false;

//We'll let the programmer worry about the proper maintanence of
//the given list of strings.  HTat is, it'll be up to them to
//clear the contents or not before invoking this member function.
string tmp_string;

for ( sb_ForeignIDs::const_iterator i =_imp->_ChainIDs.begin(); i != _imp->_ChainIDs.end();i++ )
{
   if ( ! i->packedIdentifierString( tmp_string ) )
   {
   return false;
   }
   val.push_back( tmp_string );
}
return true;
} //sb_Poly::getChainID(std::string& val)


bool
sb_Poly::getChainID( sb_ForeignIDs & val ) const
{

   val = _imp->_ChainIDs;
return true;

} // sb_Poly::getChainID


bool
sb_Poly::getCompositeID( std::list<std::string> & val ) const
{
if ( _imp->_CompositeIDs.empty() )
   return false;

//We'll let the programmer worry about the proper maintanence of
//the given list of strings.  HTat is, it'll be up to them to
//clear the contents or not before invoking this member function.
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
} //sb_Poly::getCompositeID(std::string& val)


bool
sb_Poly::getCompositeID( sb_ForeignIDs & val ) const
{

   val = _imp->_CompositeIDs;
return true;

} // sb_Poly::getCompositeID


bool
sb_Poly::getRepresentationID( std::list<std::string> & val ) const
{
if ( _imp->_RepresentationIDs.empty() )
   return false;

//We'll let the programmer worry about the proper maintanence of
//the given list of strings.  HTat is, it'll be up to them to
//clear the contents or not before invoking this member function.
string tmp_string;

for ( sb_ForeignIDs::const_iterator i =_imp->_RepresentationIDs.begin(); i != _imp->_RepresentationIDs.end();i++ )
{
   if ( ! i->packedIdentifierString( tmp_string ) )
   {
   return false;
   }
   val.push_back( tmp_string );
}
return true;
} //sb_Poly::getRepresentationID(std::string& val)


bool
sb_Poly::getRepresentationID( sb_ForeignIDs & val ) const
{

   val = _imp->_RepresentationIDs;
return true;

} // sb_Poly::getRepresentationID


bool
sb_Poly::getSchema( sio_8211Schema& schema ) const
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

} // sb_Poly::getSchema




bool
sb_Poly::getRecord( sc_Record & record ) const
{
record.clear();               // start with a clean slate

// first field, which contains module name and record number

sb_ForeignID tmp_foreign_id;

record.push_back( sc_Field() );

record.back().setMnemonic( "POLY" );

record.back().setName( "Poly" );

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

      {
for ( sb_ForeignIDs::const_iterator i = _imp->_RingIDs.begin();
      i != _imp->_RingIDs.end(); i++ )
{
sb_Utils::add_foreignID ( record, *i );
}
      }

      {
for ( sb_ForeignIDs::const_iterator i = _imp->_ChainIDs.begin();
      i != _imp->_ChainIDs.end(); i++ )
{
sb_Utils::add_foreignID ( record, *i );
}

      }

      {
for ( sb_ForeignIDs::const_iterator i = _imp->_CompositeIDs.begin();
      i != _imp->_CompositeIDs.end(); i++ )
{
sb_Utils::add_foreignID ( record, *i );
}

      }

      {
for ( sb_ForeignIDs::const_iterator i = _imp->_RepresentationIDs.begin();
      i != _imp->_RepresentationIDs.end(); i++ )
{
sb_Utils::add_foreignID ( record, *i );
}

      }

return true;


} // Poly::getRecord




bool
sb_Poly::setObjectRepresentation( string const& val )
{
_imp->_ObjectRepresentation = val;

return true;
} // sb_Poly::setObjectRepresentation


bool
sb_Poly::setAttributeID( sb_AttributeIDs const& val )
{
_imp->_AttributeIDs = val;

return true;
} // sb_Poly::setAttributeID


bool
sb_Poly::setRingID( sb_ForeignIDs const& val )
{
_imp->_RingIDs = val;

return true;
} // sb_Poly::setRingID


bool
sb_Poly::setChainID( sb_ForeignIDs const& val )
{
_imp->_ChainIDs = val;

return true;
} // sb_Poly::setChainID


bool
sb_Poly::setCompositeID( sb_ForeignIDs const& val )
{
_imp->_CompositeIDs = val;

return true;
} // sb_Poly::setCompositeID


bool
sb_Poly::setRepresentationID( sb_ForeignIDs const& val )
{
_imp->_RepresentationIDs = val;

return true;
} // sb_Poly::setRepresentationID


bool
sb_Poly::setRecord( sc_Record const& record )
{
  _imp->reset();      // reset to new state; i.e., clear out foreign
                       // identifiers, etc.
return _ingest_record( *this, *_imp, record );
} // sb_Poly::setRecord




void
sb_Poly::unDefineObjectRepresentation( )
{
_imp->_ObjectRepresentation = UNVALUED_STRING;
} // sb_Poly::unDefineObjectRepresentation


void
sb_Poly::unDefineAttributeID( )
{
_imp->_AttributeIDs.clear();
} // sb_Poly::unDefineAttributeID


void
sb_Poly::unDefineRingID( )
{
_imp->_RingIDs.clear();
} // sb_Poly::unDefineRingID


void
sb_Poly::unDefineChainID( )
{
_imp->_ChainIDs.clear();
} // sb_Poly::unDefineChainID


void
sb_Poly::unDefineCompositeID( )
{
_imp->_CompositeIDs.clear();
} // sb_Poly::unDefineCompositeID


void
sb_Poly::unDefineRepresentationID( )
{
_imp->_RepresentationIDs.clear();
} // sb_Poly::unDefineRepresentationID


sio_8211Schema& 
sb_Poly::schema_()
{
if ( _schema.empty() )
{
   buildSpecificSchema_();
}

return _schema;
} // sb_Poly::schema_()



void
sb_Poly::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Poly::buildSpecificSchema_()

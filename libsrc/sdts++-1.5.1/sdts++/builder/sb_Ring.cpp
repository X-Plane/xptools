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
// $Id: sb_Ring.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $
//

#include "builder/sb_Ring.h"


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



static const char* _ident = "$Id: sb_Ring.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Ring_Imp
{
string   _ObjectRepresentation;
sb_ForeignID   _LineorArcForeignID;
sb_ForeignID   _PolyID;


sb_Ring_Imp()
: _ObjectRepresentation( UNVALUED_STRING ),
_LineorArcForeignID( "LineorArcForeignID", "LAID" ),
_PolyID( "PolyID", "PLID" )
{}

void reset()
{
_ObjectRepresentation = UNVALUED_STRING;
_LineorArcForeignID.moduleName ( UNVALUED_STRING );
_PolyID.moduleName ( UNVALUED_STRING );

}

};// struct sb_Ring_Imp


sb_Ring::sb_Ring()
 : _imp( new sb_Ring_Imp() )
{
setMnemonic("RING");
setID( 1 );


// insert static initializers

} // Ring ctor


sb_Ring::~sb_Ring()
{
delete _imp;
} // Ring dtor




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
  field_format.setName( "Ring" );
  field_format.setTag( "RING" );


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

  foreign_id.addFieldToSchema( schema, "LineorArcForeignID" , "LAID" );
  foreign_id.addFieldToSchema( schema, "PolyID" , "PLID" );

} // _build_schema


static
bool
_ingest_record( sb_Ring& ring, sb_Ring_Imp &ring_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

sc_FieldCntr::const_iterator curfield;

if ( ! sb_Utils::getFieldByMnem( record,"RING",curfield) )
{
#ifdef SDTSXX_DEBUG
cerr << "sb_Ring::sb_Ring(sc_Record const&): "
<< "Not an ring record.";
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
ring.setMnemonic( tmp_str );
}


// RCID
if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
{
cursubfield->getI( tmp_int );
ring.setID( tmp_int );
}


// OBRP
if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
{
cursubfield->getA( ring_imp._ObjectRepresentation);
}
else
{
return false;
}




//LAID
if ( sb_Utils::getFieldByMnem( record, ring_imp._LineorArcForeignID.mnemonic(),curfield) )
{
if ( ring_imp._LineorArcForeignID.assign( *curfield ) )
	return false;
}
else
{
//XXX Add sanity checking for whenLineorArcForeignID field is required
}


//PLID
if ( sb_Utils::getFieldByMnem( record, ring_imp._PolyID.mnemonic(),curfield) )
{
if ( ring_imp._PolyID.assign( *curfield ) )
	return false;
}
else
{
//XXX Add sanity checking for whenPolyID field is required
}
return true;


} // _ingest_record




bool
sb_Ring::getObjectRepresentation( string & val ) const
{
if ( _imp->_ObjectRepresentation == UNVALUED_STRING )
return false;

val = _imp->_ObjectRepresentation;

return true;

} // sb_Ring::getObjectRepresentation


bool
sb_Ring::getLineorArcForeignID( string & val ) const
{
if ( _imp->_LineorArcForeignID.moduleName() == UNVALUED_STRING )
return false;

return _imp->_LineorArcForeignID.packedIdentifierString( val );
} // sb_Ring::getLineorArcForeignID


bool
sb_Ring::getLineorArcForeignID( sb_ForeignID& val ) const
{
if ( _imp->_LineorArcForeignID.moduleName() == UNVALUED_STRING )
return false;

val = _imp->_LineorArcForeignID;

return true;

} // sb_Ring::getLineorArcForeignID


bool
sb_Ring::getPolyID( string & val ) const
{
if ( _imp->_PolyID.moduleName() == UNVALUED_STRING )
return false;

return _imp->_PolyID.packedIdentifierString( val );
} // sb_Ring::getPolyID


bool
sb_Ring::getPolyID( sb_ForeignID& val ) const
{
if ( _imp->_PolyID.moduleName() == UNVALUED_STRING )
return false;

val = _imp->_PolyID;

return true;

} // sb_Ring::getPolyID


bool
sb_Ring::getSchema( sio_8211Schema& schema ) const
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

} // sb_Ring::getSchema




bool
sb_Ring::getRecord( sc_Record & record ) const
{
record.clear();               // start with a clean slate

// first field, which contains module name and record number

sb_ForeignID tmp_foreign_id;

record.push_back( sc_Field() );

record.back().setMnemonic( "RING" );

record.back().setName( "Ring" );

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


if ( getLineorArcForeignID(tmp_foreign_id ) )
{
sb_Utils::add_foreignID ( record, tmp_foreign_id );
}
else
{
//NOP because it's an optional field
}


if ( getPolyID(tmp_foreign_id ) )
{
sb_Utils::add_foreignID ( record, tmp_foreign_id );
}
else
{
//NOP because it's an optional field
}


return true;


} // Ring::getRecord




bool
sb_Ring::setObjectRepresentation( string const& val )
{
_imp->_ObjectRepresentation = val;

return true;
} // sb_Ring::setObjectRepresentation


bool
sb_Ring::setLineorArcForeignID( sb_ForeignID const& val )
{
_imp->_LineorArcForeignID = val;

return true;
} // sb_Ring::setLineorArcForeignID


bool
sb_Ring::setPolyID( sb_ForeignID const& val )
{
_imp->_PolyID = val;

return true;
} // sb_Ring::setPolyID


bool
sb_Ring::setRecord( sc_Record const& record )
{
_imp->reset();      // reset to new state; i.e., clear out foreign
                       // identifiers, etc.
return _ingest_record( *this, *_imp, record );
} // sb_Ring::setRecord




void
sb_Ring::unDefineObjectRepresentation( )
{
_imp->_ObjectRepresentation = UNVALUED_STRING;
} // sb_Ring::unDefineObjectRepresentation


void
sb_Ring::unDefineLineorArcForeignID( )
{
_imp->_LineorArcForeignID.moduleName( UNVALUED_STRING );
} // sb_Ring::unDefineLineorArcForeignID


void
sb_Ring::unDefinePolyID( )
{
_imp->_PolyID.moduleName( UNVALUED_STRING );
} // sb_Ring::unDefinePolyID


sio_8211Schema&
sb_Ring::schema_()
{
if ( _schema.empty() )
{
   buildSpecificSchema_();
}

return _schema;
} // sb_Ring::schema_()



void
sb_Ring::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Ring::buildSpecificSchema_()

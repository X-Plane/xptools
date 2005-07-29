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
// $Id: sb_Spdm.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $
//

#include "builder/sb_Spdm.h"


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



static const char* _ident = "$Id: sb_Spdm.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;


struct sb_Spdm_Imp
{
string   _SpatialDomainType;
string   _DomainSpatialAddressType;
string   _Comment;
sb_Spatials   _DomainSpatialAddress;


sb_Spdm_Imp()
: _SpatialDomainType( UNVALUED_STRING ),
_DomainSpatialAddressType( UNVALUED_STRING ),
_Comment( UNVALUED_STRING )
{}

void reset()
{
_SpatialDomainType = UNVALUED_STRING;
_DomainSpatialAddressType = UNVALUED_STRING;
_Comment = UNVALUED_STRING;
_DomainSpatialAddress.clear ();

}

};// struct sb_Spdm_Imp


sb_Spdm::sb_Spdm()
 : _imp( new sb_Spdm_Imp() )
{
setMnemonic("SPDM");
setID( 1 );


// insert static initializers

} // Spdm ctor


sb_Spdm::~sb_Spdm()
{
delete _imp;
} // Spdm dtor




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
  field_format.setName( "Spdm" );
  field_format.setTag( "SPDM" );


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

  field_format.back().setLabel( "DTYP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DSTP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DMSA" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );
  field_format.setIsRepeating( false );



} // _build_schema


static
bool
_ingest_record( sb_Spdm& spdm, sb_Spdm_Imp &spdm_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

sc_FieldCntr::const_iterator curfield;

if ( ! sb_Utils::getFieldByMnem( record,"SPDM",curfield) )
{
#ifdef SDTSXX_DEBUG
cerr << "sb_Spdm::sb_Spdm(sc_Record const&): "
<< "Not an spatial domain record.";
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
spdm.setMnemonic( tmp_str );
}


// RCID
if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
{
cursubfield->getI( tmp_int );
spdm.setID( tmp_int );
}


// DTYP
if (sb_Utils::getSubfieldByMnem(*curfield,"DTYP",cursubfield))
{
cursubfield->getA( spdm_imp._SpatialDomainType);
}
else
{
return false;
}


// DSTP
if (sb_Utils::getSubfieldByMnem(*curfield,"DSTP",cursubfield))
{
cursubfield->getA( spdm_imp._DomainSpatialAddressType);
}
else
{
return false;
}


// COMT
if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
{
cursubfield->getA( spdm_imp._Comment);
}
else
{
  //return false; //optional subfield
}



//DMSA
if ( sb_Utils::getFieldByMnem( record,"DMSA" ,curfield) )
{
do
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

spdm_imp._DomainSpatialAddress.push_back( tmp_spatial );

curfield++;	//increment to next field, if any

} while ( curfield != record.end() );

}
else
{
return false;
}

return true;


} // _ingest_record




bool
sb_Spdm::getSpatialDomainType( string & val ) const
{
if ( _imp->_SpatialDomainType == UNVALUED_STRING )
return false;

val = _imp->_SpatialDomainType;

return true;

} // sb_Spdm::getSpatialDomainType


bool
sb_Spdm::getDomainSpatialAddressType( string & val ) const
{
if ( _imp->_DomainSpatialAddressType == UNVALUED_STRING )
return false;

val = _imp->_DomainSpatialAddressType;

return true;

} // sb_Spdm::getDomainSpatialAddressType


bool
sb_Spdm::getComment( string & val ) const
{
if ( _imp->_Comment == UNVALUED_STRING )
return false;

val = _imp->_Comment;

return true;

} // sb_Spdm::getComment


bool
sb_Spdm::getDomainSpatialAddress( sb_Spatials & val ) const
{
if ( _imp->_DomainSpatialAddress.empty()  )
return false;

val = _imp->_DomainSpatialAddress;

return true;

} // sb_Spdm::getDomainSpatialAddress


bool
sb_Spdm::getSchema( sio_8211Schema& schema ) const
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

} // sb_Spdm::getSchema




bool
sb_Spdm::getRecord( sc_Record & record ) const
{
record.clear();               // start with a clean slate

// first field, which contains module name and record number

sb_ForeignID tmp_foreign_id;

record.push_back( sc_Field() );

record.back().setMnemonic( "SPDM" );

record.back().setName( "Spdm" );

string tmp_str;


getMnemonic( tmp_str );
sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
sb_Utils::add_subfield( record.back(), "RCID", getID() );

if ( getSpatialDomainType( tmp_str ) )
{
sb_Utils::add_subfield( record.back(),"DTYP", tmp_str );
}
else
{
sb_Utils::add_empty_subfield( record.back(), "DTYP", sc_Subfield::is_A );
}


if ( getDomainSpatialAddressType( tmp_str ) )
{
sb_Utils::add_subfield( record.back(),"DSTP", tmp_str );
}
else
{
sb_Utils::add_empty_subfield( record.back(), "DSTP", sc_Subfield::is_A );
}


if ( getComment( tmp_str ) )
{
sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
}
else
{
sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
}


//XXX for each spatial address, add a DomainSpatialAddress to a repeating field

sb_Utils::add_field( record, "DomainSpatialAddress","DMSA" );

for ( sb_Spatials::const_iterator i = _imp->_DomainSpatialAddress.begin();
      i != _imp->_DomainSpatialAddress.end(); i++ )
{
//just shove the sc_subfields directly onto the end of the field

cout << "X:"<< (*i).x() << endl;
cout << "Y:"<< (*i).x() << endl;

record.back().push_back( (*i).x() );
record.back().push_back( (*i).y() );
}


return true;


} // Spdm::getRecord




bool
sb_Spdm::setSpatialDomainType( string const& val )
{
_imp->_SpatialDomainType = val;

return true;
} // sb_Spdm::setSpatialDomainType


bool
sb_Spdm::setDomainSpatialAddressType( string const& val )
{
_imp->_DomainSpatialAddressType = val;

return true;
} // sb_Spdm::setDomainSpatialAddressType


bool
sb_Spdm::setComment( string const& val )
{
_imp->_Comment = val;

return true;
} // sb_Spdm::setComment


bool
sb_Spdm::setDomainSpatialAddress( sb_Spatials const& val )
{
_imp->_DomainSpatialAddress = val;

return true;
} // sb_Spdm::setDomainSpatialAddress


bool
sb_Spdm::setRecord( sc_Record const& record )
{
_imp->reset();      // reset to new state; i.e., clear out foreign
                       // identifiers, etc.
return _ingest_record( *this, *_imp, record );
} // sb_Spdm::setRecord




void
sb_Spdm::unDefineSpatialDomainType( )
{
_imp->_SpatialDomainType = UNVALUED_STRING;
} // sb_Spdm::unDefineSpatialDomainType


void
sb_Spdm::unDefineDomainSpatialAddressType( )
{
_imp->_DomainSpatialAddressType = UNVALUED_STRING;
} // sb_Spdm::unDefineDomainSpatialAddressType


void
sb_Spdm::unDefineComment( )
{
_imp->_Comment = UNVALUED_STRING;
} // sb_Spdm::unDefineComment


void
sb_Spdm::unDefineDomainSpatialAddress( )
{
_imp->_DomainSpatialAddress.clear();
} // sb_Spdm::unDefineDomainSpatialAddress


sio_8211Schema& 
sb_Spdm::schema_()
{
if ( _schema.empty() )
{
   buildSpecificSchema_();
}

return _schema;
} // sb_Spdm::schema_()



void
sb_Spdm::buildSpecificSchema_()
{_build_schema( _schema );
} // sb_Spdm::buildSpecificSchema_()

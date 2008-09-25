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
// $Id: sb_Clrx.cpp,v 1.4 2002/11/24 22:07:42 mcoletti Exp $
//

#include "builder/sb_Clrx.h"



#include <iostream>
#include <strstream>

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


static const char* _ident = "$Id: sb_Clrx.cpp,v 1.4 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

static const sb_ForeignID UNVALUED_FOREIGN_ID( UNVALUED_STRING,
                                               -1 );

struct sb_Clrx_Imp
{
      double   _RedComponent;
      double   _GreenComponent;
      double   _BlueComponent;
      double   _BlackComponent;


      sb_Clrx_Imp()
         :
         _RedComponent( UNVALUED_DOUBLE ),
         _GreenComponent( UNVALUED_DOUBLE ),
         _BlueComponent( UNVALUED_DOUBLE ),
         _BlackComponent( UNVALUED_DOUBLE )
      {}

};


sb_Clrx::sb_Clrx()
   : _imp( new sb_Clrx_Imp() )
{
   setMnemonic("CLRX");
   setID( 1 );


// insert static initializers

} // Clrx ctor


sb_Clrx::~sb_Clrx()
{
   delete _imp;
} // Clrx dtor




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
   field_format.setName( "Color Index" );
   field_format.setTag( "CLRX" );


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

   field_format.back().setLabel( "RED" );
   field_format.back().setType( sio_8211SubfieldFormat::R );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_R );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "GREN" );
   field_format.back().setType( sio_8211SubfieldFormat::R );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_R );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "BLUE" );
   field_format.back().setType( sio_8211SubfieldFormat::R );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_R );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "BLCK" );
   field_format.back().setType( sio_8211SubfieldFormat::R );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_R );


} // _build_schema





void
sb_Clrx::buildSpecificSchema_(  )
{
   if ( _schema.empty() )
   {
      _build_schema( _schema );
   }
} // sb_Clrx::buildSpecificSchema_(  )


sio_8211Schema &
sb_Clrx::schema_()
{
   _build_schema( _schema );

   return _schema;
} // sio_8211Schema& schema_()



static
bool
_ingest_record( sb_Clrx& clrx, sb_Clrx_Imp &clrx_imp, sc_Record const& record )
{

// Make sure we have a record from an
// External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record,"CLRX",curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Clrx::sb_Clrx(sc_Record const&): "
           << "Not an color index record.";
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
      clrx.setMnemonic( tmp_str );
   }


// RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      clrx.setID( tmp_int );
   }


// RED
   if (sb_Utils::getSubfieldByMnem(*curfield,"RED",cursubfield))
   {
      cursubfield->getR( clrx_imp._RedComponent);
   }
   else
   {
      return false;
   }


// GREN
   if (sb_Utils::getSubfieldByMnem(*curfield,"GREN",cursubfield))
   {
      cursubfield->getR( clrx_imp._GreenComponent);
   }
   else
   {
      return false;
   }


// BLUE
   if (sb_Utils::getSubfieldByMnem(*curfield,"BLUE",cursubfield))
   {
      cursubfield->getR( clrx_imp._BlueComponent);
   }
   else
   {
      return false;
   }


// BLCK
   if (sb_Utils::getSubfieldByMnem(*curfield,"BLCK",cursubfield))
   {
      cursubfield->getR( clrx_imp._BlackComponent);
   }
   else
   {
      return false;
   }


   return true;


} // _ingest_record




bool
sb_Clrx::getRedComponent( double& val ) const
{
   if ( _imp->_RedComponent == UNVALUED_DOUBLE )
      return false;

   val = _imp->_RedComponent;

   return true;
} // sb_Clrx::getRedComponent


bool
sb_Clrx::getGreenComponent( double& val ) const
{
   if ( _imp->_GreenComponent == UNVALUED_DOUBLE )
      return false;

   val = _imp->_GreenComponent;

   return true;
} // sb_Clrx::getGreenComponent


bool
sb_Clrx::getBlueComponent( double& val ) const
{
   if ( _imp->_BlueComponent == UNVALUED_DOUBLE )
      return false;

   val = _imp->_BlueComponent;

   return true;
} // sb_Clrx::getBlueComponent


bool
sb_Clrx::getBlackComponent( double& val ) const
{
   if ( _imp->_BlackComponent == UNVALUED_DOUBLE )
      return false;

   val = _imp->_BlackComponent;

   return true;
} // sb_Clrx::getBlackComponent


bool
sb_Clrx::getSchema( sio_8211Schema& schema ) const
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

} // sb_Clrx::getSchema




bool
sb_Clrx::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

// first field, which contains module name and record number

   record.push_back( sc_Field() );

   record.back().setMnemonic( "CLRX" );

   record.back().setName( "Color Index" );

   string tmp_str;
   double tmp_double;
   long   tmp_long;

   getMnemonic( tmp_str );
   sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
   sb_Utils::add_subfield( record.back(), "RCID", getID() );

   if ( getRedComponent( tmp_double ) )
   {
      sb_Utils::add_subfield( record.back(),"RED", tmp_double );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "RED", sc_Subfield::is_R );
   }


   if ( getGreenComponent( tmp_double ) )
   {
      sb_Utils::add_subfield( record.back(),"GREN", tmp_double );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "GREN", sc_Subfield::is_R );
   }


   if ( getBlueComponent( tmp_double ) )
   {
      sb_Utils::add_subfield( record.back(),"BLUE", tmp_double );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "BLUE", sc_Subfield::is_R );
   }


   if ( getBlackComponent( tmp_double ) )
   {
      sb_Utils::add_subfield( record.back(),"BLCK", tmp_double );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "BLCK", sc_Subfield::is_R );
   }


   return true;


} // Clrx::getRecord




bool
sb_Clrx::setRedComponent( double val )
{
   _imp->_RedComponent = val;

   return true;
} // sb_Clrx::setRedComponent


bool
sb_Clrx::setGreenComponent( double val )
{
   _imp->_GreenComponent = val;

   return true;
} // sb_Clrx::setGreenComponent


bool
sb_Clrx::setBlueComponent( double val )
{
   _imp->_BlueComponent = val;

   return true;
} // sb_Clrx::setBlueComponent


bool
sb_Clrx::setBlackComponent( double val )
{
   _imp->_BlackComponent = val;

   return true;
} // sb_Clrx::setBlackComponent


bool
sb_Clrx::setRecord( sc_Record const& record )
{
   return _ingest_record( *this, *_imp, record );
} // sb_Clrx::setRecord




void
sb_Clrx::unDefineRedComponent( )
{
   _imp->_RedComponent = UNVALUED_DOUBLE;
} // sb_Clrx::unDefineRedComponent


void
sb_Clrx::unDefineGreenComponent( )
{
   _imp->_GreenComponent = UNVALUED_DOUBLE;
} // sb_Clrx::unDefineGreenComponent


void
sb_Clrx::unDefineBlueComponent( )
{
   _imp->_BlueComponent = UNVALUED_DOUBLE;
} // sb_Clrx::unDefineBlueComponent


void
sb_Clrx::unDefineBlackComponent( )
{
   _imp->_BlackComponent = UNVALUED_DOUBLE;
} // sb_Clrx::unDefineBlackComponent



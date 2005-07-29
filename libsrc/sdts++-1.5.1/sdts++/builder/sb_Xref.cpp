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

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <sdts++/builder/sb_Xref.h>

#include <iostream>
#include <strstream>

#include <limits.h>

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif

#ifdef NOT_IMPLEMENTED
#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif
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


static const char* ident_ = "$Id: sb_Xref.cpp,v 1.7 2002/11/24 22:07:43 mcoletti Exp $";

// Strings and integers are initialized with these values; they're used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static set<string> RSNM_domain;
static set<string> SDAT_domain;

struct sb_Xref_Imp
{
  string       comment_;
  string       refDoc_;
  string       refSysName_;
#ifdef NOT_RASTER_PROFILE
  string       vertDatum_;
  string       soundDatum_;
#endif
  string       horizDatum_;
  string       zoneNum_;
  string       projection_;

#ifdef NOT_IMPLEMENTED
  list<sb_ForeignID> attID_;    // list of attribute ID's
#endif

  sb_Xref_Imp()
    : comment_( UNVALUED_STRING ),
      refDoc_( UNVALUED_STRING ),
      refSysName_( UNVALUED_STRING ),
#ifdef NOT_RASTER_PROFILE
      vertDatum_( UNVALUED_STRING ),
      soundDatum_( UNVALUED_STRING ),
#endif
      horizDatum_( UNVALUED_STRING ),
      zoneNum_( UNVALUED_STRING ),
      projection_( UNVALUED_STRING )
    {}

}; // struct sb_Xref_Imp



sb_Xref::sb_Xref()
  : imp_( new sb_Xref_Imp )
{
  setMnemonic( "XREF" );
  setID( 1 );

  if ( RSNM_domain.empty() )    // initialize static set if necessary
    {
      RSNM_domain.insert( "GEO" ); // these are the valid reference
      RSNM_domain.insert( "SPCS" ); // mnemonics
      RSNM_domain.insert( "UTM" );
      RSNM_domain.insert( "UPS" );
      RSNM_domain.insert( "OTHR" );
      RSNM_domain.insert( "UNSP" );
    }
#ifdef NOT_RASTER_PROFILE
  if ( SDAT_domain.empty() )
    {
      SDAT_domain.insert( "MHW" );
      SDAT_domain.insert( "MHWN" );
      SDAT_domain.insert( "MHWS" );
      SDAT_domain.insert( "MHHW" );
      SDAT_domain.insert( "MLW" );
      SDAT_domain.insert( "MLWN" );
      SDAT_domain.insert( "MLWS" );
      SDAT_domain.insert( "MLLW" );
    }
#endif
} 


sb_Xref::~sb_Xref()
{
  delete imp_;
}

static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h


static sio_8211Schema schema_; // XREF schema


sio_8211Schema&
sb_Xref::schema_()
{
  return ::schema_;
} // sb_Xref::schema_()




void
sb_Xref::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "EXTERNAL SPATIAL REFERENCE" );
  field_format.setTag( "XREF" );

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

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RDOC" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RSNM" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


#ifdef NOT_RASTER_PROFILE
  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "VDAT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SDAT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );
#endif


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "HDAT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ZONE" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PROJ" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  // XXX ATID and VATT and SATT not implemented

} // build__schema



//
// Build an sb_Xref from an sc_Record
//
static
bool
ingest_record_( sb_Xref& xref, sb_Xref_Imp& xref_imp, sc_Record const& recprox )
{

                                // Make sure we have a record from an
                                // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem(recprox,"XREF",curfield) )
    {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Xref::sb_Xref(sc_Record const&): "
           << "Not an External Spatial Reference module record.";
      cerr << endl;
#endif
      return false;
    }

  // We have a primary field from a  module. Start
  // picking it apart.

  sc_SubfieldCntr::const_iterator cursubfield;

  string tmp_str;
  long   tmp_int;

  // MODN
  if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
    {
      cursubfield->getA( tmp_str );
      xref.setMnemonic( tmp_str );
    }

  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
    {
      cursubfield->getI( tmp_int );
      xref.setID( tmp_int );
    }


  // COMT
  if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
    cursubfield->getA( xref_imp.comment_);

  // RDOC
  if (sb_Utils::getSubfieldByMnem(*curfield,"RDOC",cursubfield))
    cursubfield->getA( xref_imp.refDoc_);

  // RSNM
  if (sb_Utils::getSubfieldByMnem(*curfield,"RSNM",cursubfield))
    cursubfield->getA( xref_imp.refSysName_);

#ifdef NOT_RASTER_PROFILE
  // VDAT
  if (sb_Utils::getSubfieldByMnem(*curfield,"VDAT",cursubfield))
    cursubfield->getA( xref_imp.vertDatum_);

  // SDAT
  if (sb_Utils::getSubfieldByMnem(*curfield,"SDAT",cursubfield))
    cursubfield->getA( xref_imp.soundDatum_);
#endif

  // HDAT
  if (sb_Utils::getSubfieldByMnem(*curfield,"HDAT",cursubfield))
    cursubfield->getA( xref_imp.horizDatum_);

  // ZONE
  if (sb_Utils::getSubfieldByMnem(*curfield,"ZONE",cursubfield))
    cursubfield->getA( xref_imp.zoneNum_);

  // PROJ
  if (sb_Utils::getSubfieldByMnem(*curfield,"PROJ",cursubfield))
    cursubfield->getA( xref_imp.projection_);

  // Secondary Fields

  // Attribute ID (ATID) Field
  // XXX What about repeating fields?
#ifdef NOT_IMPLEMENTED
  if (sb_Utils::getFieldByMnem(recprox,"ATID",curfield))
    xref_imp.attID_.push_back( sb_ForeignID(*curfield) );
#endif

  return true;

} // ingest__record



bool
sb_Xref::getComment( string & val ) const
{
  if ( imp_->comment_ == UNVALUED_STRING )
    return false;

  val = imp_->comment_;

  return true;
} // sb_Xref::getComment



bool
sb_Xref::getReferenceDocumentation( string & val ) const
{
  if ( imp_->refDoc_ == UNVALUED_STRING )
    return false;

  val = imp_->refDoc_;

  return true;
} // sb_Xref::getReferenceDocumentation



bool
sb_Xref::getReferenceSystemName( string & val ) const
{
  if ( imp_->refSysName_ == UNVALUED_STRING )
    return false;

  val = imp_->refSysName_;

  return true;
} // sb_Xref::getReferenceSystemName



#ifdef NOT_RASTER_PROFILE
bool
sb_Xref::getVerticalDatum( string & val ) const
{
  if ( imp_->vertDatum_ == UNVALUED_STRING )
    return false;

  val = imp_->vertDatum_;

  return true;
} // sb_Xref::getVerticalDatum


bool
sb_Xref::getSoundingDatum( string & val ) const
{
  if ( imp_->soundDatum_ == UNVALUED_STRING )
    return false;

  val = imp_->soundDatum_;

  return true;
} // sb_Xref::getSoundingDatum
#endif


bool
sb_Xref::getHorizontalDatum( string & val ) const
{
  if ( imp_->horizDatum_ == UNVALUED_STRING )
    return false;

  val = imp_->horizDatum_;

  return true;
} // sb_Xref::getHorizontalDatum


bool
sb_Xref::getZoneReferenceNumber( string & val ) const
{
  if ( imp_->zoneNum_ == UNVALUED_STRING )
    return false;

  val = imp_->zoneNum_;

  return true;
} // sb_Xref::getZoneReferenceNumber


bool
sb_Xref::getProjection( string & val ) const
{
  if ( imp_->projection_ == UNVALUED_STRING )
    return false;

  val = imp_->projection_;

  return true;
} // sb_Xref::getProjection


#ifdef NOT_IMPLEMENTED
bool
sb_Xref::getAttID( list<sb_ForeignID> & val ) const
{
  //   if ( imp_->attID_ == UNVALUED_STRING ) XXX need way to determine
  //     return false; XXX that aid has not been given a value

  val = imp_->attID_;

  return true;
} // sb_Xref::getAttID
#endif





bool
sb_Xref::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // XREF field

  record.push_back( sc_Field() );

  record.back().setMnemonic( "XREF" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getCOMT( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
    }

  if ( getRDOC( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"RDOC", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "RDOC", sc_Subfield::is_A );
    }

  string rsnm;

  if ( ! getRSNM( rsnm ) )
    {
      return false;
    }
  else
    {
      if ( ! sb_Utils::valid_domain( rsnm, RSNM_domain ) )
        {
          return false;
        }
    }

  sb_Utils::add_subfield( record.back(),"RSNM", rsnm );


#ifdef NOT_RASTER_PROFILE
  if ( getVDAT( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"VDAT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "VDAT", sc_Subfield::is_A );
    }


  if ( getSDAT( tmp_str ) )
    {
      if ( ! sb_Utils::valid_domain( tmp_str, SDAT_domain ) )
        {
          return false;
        }
      sb_Utils::add_subfield( record.back(),"SDAT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "SDAT", sc_Subfield::is_A );
    }
#endif


  if ( getHDAT( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(),"HDAT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "HDAT", sc_Subfield::is_A );
    }


  if ( rsnm == "SPCS" ||
       rsnm == "UTM"  || 
       rsnm == "UPS" ) 
    {
      if ( ! getZONE( tmp_str ) )
        {
          return false;
        }

      sb_Utils::add_subfield( record.back(),"ZONE", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ZONE", sc_Subfield::is_A );
    }


  if ( rsnm == "OTHR" )
    {

      if ( ! getPROJ( tmp_str ) )
        {
          return false;
        }

      sb_Utils::add_subfield( record.back(),"PROJ", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "PROJ", sc_Subfield::is_A );
    }

  // XXX add support for attribute ID's

  return true;

} // sb_Xref::setRecord








bool
sb_Xref::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Xref::setRecord



bool
sb_Xref::setComment( string const & val ) 
{
  imp_->comment_ = val;
  return true;
} // sb_Xref::setComment



bool
sb_Xref::setReferenceDocumentation( string const & val ) 
{
  imp_->refDoc_ = val;
  return true;
} // sb_Xref::setReferenceDocumentation



bool
sb_Xref::setReferenceSystemName( string const & val ) 
{
  if ( ! sb_Utils::valid_domain( val, RSNM_domain ) )
    {
      return false;
    }
  imp_->refSysName_ = val;
  return true;
} // sb_Xref::setReferenceSystemName



#ifdef NOT_RASTER_PROFILE
bool
sb_Xref::setVerticalDatum( string const & val ) 
{
  imp_->vertDatum_ = val;
  return true;
} // sb_Xref::setVerticalDatum



bool
sb_Xref::setSoundingDatum( string const & val ) 
{
  if ( ! sb_Utils::valid_domain( val, SDAT_domain ) )
    {
      return false;
    }
  imp_->soundDatum_ = val;
  return true;
} // sb_Xref::setSoundingDatum
#endif


bool
sb_Xref::setHorizontalDatum( string const & val ) 
{
  imp_->horizDatum_ = val;
  return true;
} // sb_Xref::setHorizontalDatum



bool
sb_Xref::setZoneReferenceNumber( string const & val ) 
{
  imp_->zoneNum_ = val;
  return true;
} // sb_Xref::setZoneReferenceNumber


bool
sb_Xref::setZoneReferenceNumber( int val ) 
{
  strstream ss;

  ss << val;// << ends;

  if ( ! (ss >> imp_->zoneNum_) ) return false;

  return true;
} // sb_Xref::setZoneReferenceNumber



bool
sb_Xref::setProjection( string const & val ) 
{
  imp_->projection_ = val;
  return true;
} // sb_Xref::setProjection


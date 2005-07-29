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


#include "sb_Iref.h"

#include <float.h>
#include <limits.h>

#include <iostream>

#include <string>

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


static const char* ident_ = "$Id: sb_Iref.cpp,v 1.9 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they're used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

static sio_8211Schema iref_schema_; // iref module schema, which will be
                                // defined by build_iref_schema

static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;


struct 
sb_Iref_Imp
{
  string comment_;
      
  string spatialAddType_;
  string spatialAddXCompLbl_;
  string spatialAddYCompLbl_;
  string horizontalCompFmt_;
  string verticalCompFmt_;
     
  double scaleFactX_;
  double scaleFactY_;
  double scaleFactZ_;
  double xOrigin_;
  double yOrigin_; 
  double zOrigin_;
      
  double xCompHorizRes_;
  double yCompHorizRes_;
  double verticalResComp_;

#ifdef NOT_IMPLEMENTED
  vector<sb_ForeignID> dimensionID_;
#endif

  sb_Iref_Imp()
    : comment_( UNVALUED_STRING ),
      spatialAddType_( UNVALUED_STRING ),
      spatialAddXCompLbl_( UNVALUED_STRING ),
      spatialAddYCompLbl_( UNVALUED_STRING ),
      horizontalCompFmt_( UNVALUED_STRING ),
      verticalCompFmt_( UNVALUED_STRING ),
      scaleFactX_( UNVALUED_DOUBLE ),
      scaleFactY_( UNVALUED_DOUBLE ),
      scaleFactZ_( UNVALUED_DOUBLE ),
      xOrigin_( UNVALUED_DOUBLE ),
      yOrigin_( UNVALUED_DOUBLE ),
      zOrigin_( UNVALUED_DOUBLE ),
      xCompHorizRes_( UNVALUED_DOUBLE ),
      yCompHorizRes_( UNVALUED_DOUBLE ),
      verticalResComp_( UNVALUED_DOUBLE )
    {}

}; // sb_Iref_Imp






sb_Iref::sb_Iref()
  : imp_( new sb_Iref_Imp )
{
  setMnemonic( "IREF" );        // set reasonable module mnemonic and
  setID( 1 );                   // record id defaults
}


sb_Iref::~sb_Iref()
{
  delete imp_;
}



sio_8211Schema&
sb_Iref::schema_()
{
   return iref_schema_;
} // sb_Iref::schema_()



//
// builds a schema suitable for writing an SDTS IREF module
//
void
sb_Iref::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "INTERNAL SPATIAL REFERENCE" );
  field_format.setTag( "IREF" );

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

  field_format.back().setLabel( "SATP" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "XLBL" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "YLBL" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "HFMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


#ifdef NOT_RASTER_PROFILE
  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "VFMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );
#endif

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SFAX" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SFAY" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );


#ifdef NOT_RASTER_PROFILE
  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SFAZ" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );
#endif


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "XORG" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "YORG" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );



#ifdef NOT_RASTER_PROFILE
  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ZORG" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );
#endif


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "XHRS" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );



  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "YHRS" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );



#ifdef NOT_RASTER_PROFILE
  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "VRES" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );
#endif

  // DMID

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "DIMENSION ID" );
  schema_().back().setTag( "DMID" );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "MODN" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "RCID" );
  schema_().back().back().setType( sio_8211SubfieldFormat::I );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_I );



} // build__iref_schema






//
// Build an sb_Iref from an sc_Record
//

static
bool
ingest_record_( sb_Iref & iref, 
                sb_Iref_Imp & iref_imp, 
                sc_Record const & record )
{

  // Make sure we have a record from a Internal Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;
  if ( ! sb_Utils::getFieldByMnem( record, "IREF", curfield ) )
    {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Iref::sb_Iref(sc_Record const&): "
           << "Not an Internal Spatial Reference record.";
      cerr << endl;
#endif;
      return false;
    }

  // We have a primary field from a  module. Start
  // picking it apart.

  sc_SubfieldCntr::const_iterator cursubfield;

  // MODN
  if ( sb_Utils::getSubfieldByMnem( *curfield, "MODN", cursubfield ) )
    {
      string tmp;
      
      cursubfield->getA(  tmp  );
      iref.setMnemonic(  tmp  );
    }
      

  // RCID
  if ( sb_Utils::getSubfieldByMnem( *curfield, "RCID", cursubfield ) )
    {
      long tmp;
      cursubfield->getI(  tmp  );
      iref.setID(  tmp  );
    }


   // COMT
  if ( sb_Utils::getSubfieldByMnem( *curfield, "COMT", cursubfield ) )
    cursubfield->getA( iref_imp.comment_ );

   // SATP 
  if ( sb_Utils::getSubfieldByMnem( *curfield, "SATP", cursubfield ) )
    cursubfield->getA( iref_imp.spatialAddType_ );

   // XLBL
  if ( sb_Utils::getSubfieldByMnem( *curfield, "XLBL", cursubfield ) )
    cursubfield->getA( iref_imp.spatialAddXCompLbl_ );

   // YLBL
  if ( sb_Utils::getSubfieldByMnem( *curfield, "YLBL", cursubfield ) )
    cursubfield->getA( iref_imp.spatialAddYCompLbl_ );

   // HFMT
  if ( sb_Utils::getSubfieldByMnem( *curfield, "HFMT", cursubfield ) )
    cursubfield->getA( iref_imp.horizontalCompFmt_ );

#ifdef NOT_RASTER_PROFILE
   // VFMT
  if ( sb_Utils::getSubfieldByMnem( *curfield, "VFMT", cursubfield ) )
    cursubfield->getA( iref_imp.verticalCompFmt_ );
#endif

   // SFAX
  if ( sb_Utils::getSubfieldByMnem( *curfield, "SFAX", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.scaleFactX_ );

   // SFAY
  if ( sb_Utils::getSubfieldByMnem( *curfield, "SFAY", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.scaleFactY_ );

#ifdef NOT_RASTER_PROFILE
   // SFAZ
  if ( sb_Utils::getSubfieldByMnem( *curfield, "SFAZ", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.scaleFactZ_ );
#endif

   // XORG
  if ( sb_Utils::getSubfieldByMnem( *curfield, "XORG", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.xOrigin_ );

   // YORG
  if ( sb_Utils::getSubfieldByMnem( *curfield, "YORG", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.yOrigin_ );

#ifdef NOT_RASTER_PROFILE
   // ZORG
  if ( sb_Utils::getSubfieldByMnem( *curfield, "ZORG", cursubfield ) )
    sb_Utils::getDoubleFromSubfield( cursubfield, iref_imp.zOrigin_ );
#endif

  // XHRS
  if ( sb_Utils::getSubfieldByMnem( *curfield, "XHRS", cursubfield ) )
    sb_Utils::getDoubleFromSubfield(  cursubfield,  iref_imp.xCompHorizRes_ );
     
   // YHRS
  if ( sb_Utils::getSubfieldByMnem( *curfield, "YHRS", cursubfield ) )
    sb_Utils::getDoubleFromSubfield(  cursubfield, iref_imp.yCompHorizRes_ );
         
#ifdef NOT_RASTER_PROFILE
   // VRES
  if ( sb_Utils::getSubfieldByMnem( *curfield, "VRES", cursubfield ) )
    sb_Utils::getDoubleFromSubfield(  cursubfield,  iref_imp.verticalResComp_ );
#endif


  // Secondary Fields

#ifdef NOT_IMPLEMENTED
  sc_FieldCntr const& fields = record; 
  for ( curfield  = fields.begin( );
        curfield != fields.end( );
        curfield++ )
    {
      if ( curfield->getMnemonic(  ) == "DMID" )
        iref_imp.dimensionID_.push_back( sb_ForeignID( *curfield )  );
    }
#endif

   return true;

} // ingest__record




bool
sb_Iref::getComment(  string& val  ) const
{
  if ( imp_->comment_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->comment_;
  return true;
}


bool
sb_Iref::getSpatialAddressType(  string& val  ) const
{
  if ( imp_->spatialAddType_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->spatialAddType_;
  return true;
}


bool
sb_Iref::getSpatialAddressXLabel(  string& val  ) const
{
  if ( imp_->spatialAddXCompLbl_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->spatialAddXCompLbl_;
  return true;
}


bool
sb_Iref::getSpatialAddressYLabel(  string& val  ) const
{
  if ( imp_->spatialAddYCompLbl_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->spatialAddYCompLbl_;
  return true;
}


bool
sb_Iref::getHorizontalComponentFormat(  string& val  ) const
{
  if ( imp_->horizontalCompFmt_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->horizontalCompFmt_;
  return true;
}

 
#ifdef NOT_RASTER_PROFILE
bool
sb_Iref::getVerticalComponentFormat(  string& val  ) const
{
  if ( imp_->verticalCompFmt_ == UNVALUED_STRING )
    {
      return false;
    }
  val = imp_->verticalCompFmt_;
  return true;
}
#endif


bool
sb_Iref::getScaleFactorX(  double& val  ) const
{
  if ( imp_->scaleFactX_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->scaleFactX_;
  return true;
}


bool
sb_Iref::getScaleFactorY(  double& val  ) const
{
  if ( imp_->scaleFactY_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->scaleFactY_;
  return true;
}


#ifdef NOT_RASTER_PROFILE
bool
sb_Iref::getScaleFactorZ(  double& val  ) const
{
  if ( imp_->scaleFactZ_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->scaleFactZ_;
  return true;
}
#endif


bool
sb_Iref::getXOrigin(  double& val  ) const
{
  if ( imp_->xOrigin_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->xOrigin_;
  return true;
}


bool
sb_Iref::getYOrigin(  double& val  ) const
{
  if ( imp_->yOrigin_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->yOrigin_;
  return true;
}

 
#ifdef NOT_RASTER_PROFILE
bool
sb_Iref::getZOrigin(  double& val  ) const
{
  if ( imp_->zOrigin_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->zOrigin_;
  return true;
}
#endif


bool
sb_Iref::getXComponentHorizontalResolution(  double& val  ) const
{
  if ( imp_->xCompHorizRes_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->xCompHorizRes_;
  return true;
}


bool
sb_Iref::getYComponentHorizontalResolution(  double& val  ) const
{
  if ( imp_->yCompHorizRes_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->yCompHorizRes_;
  return true;
}


#ifdef NOT_RASTER_PROFILE
bool
sb_Iref::getVerticalResolutionComponent(  double& val  ) const
{
  if ( imp_->verticalResComp_ == UNVALUED_DOUBLE )
    {
      return false;
    }
  val = imp_->verticalResComp_;
  return true;
}
#endif


#ifdef NOT_IMPLEMENTED
bool
sb_Iref::getDimensionID(  vector<sb_ForeignID>& fids  ) const
{
  if (  imp_->dimensionID_.empty(  )  ) return false;

  fids = imp_->dimensionID_;

  return true;
}
#endif


bool
sb_Iref::getRecord( sc_Record & record ) const
{

  record.clear();               // start with a clean slate

  // IREF field

  record.push_back( sc_Field() );

  record.back().setMnemonic( "IREF" );

  string tmp_str;
  double tmp_double;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );


  // Add each field and subfield to the record.  Bail if a mandatory
  // subfield hasn't been set.

  if ( getCOMT( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "COMT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
    }

  set<string> SATP_domain;      // XXX this needs to be static const somewhere
  SATP_domain.insert( TWO_TUPLE );
  SATP_domain.insert( THREE_TUPLE );

  string SATP;

  if ( ! ( getSATP( SATP ) &&
           sb_Utils::valid_domain( SATP, SATP_domain ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "SATP", SATP );


  string XLBL;

  if ( ! getXLBL( XLBL ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "XLBL", XLBL );



  if ( ! getYLBL( tmp_str ) )   // some basic sanity checking
    {
      return false;
    }
  else if ( XLBL == LONGITUDE )
    {
      if ( tmp_str != LATITUDE ) return false;
    }
  else if ( XLBL == LATITUDE )
    {
      if ( tmp_str != LONGITUDE ) return false;
    }
  else if ( XLBL == EASTING )
    {
      if ( tmp_str != NORTHING ) return false;
    }
  else if ( XLBL == NORTHING )
    {
      if ( tmp_str != EASTING ) return false;
    }

  sb_Utils::add_subfield( record.back(), "YLBL", tmp_str );


  set<string> FMT_domain;
  FMT_domain.insert( "I" );
  FMT_domain.insert( "R" );
  FMT_domain.insert( "S" );
  FMT_domain.insert( "BI8" );
  FMT_domain.insert( "BI16" );
  FMT_domain.insert( "BI24" );
  FMT_domain.insert( "BI32" );
  FMT_domain.insert( "BUI" );
  FMT_domain.insert( "BUI8" );
  FMT_domain.insert( "BUI16" );
  FMT_domain.insert( "BUI24" );
  FMT_domain.insert( "BUI32" );
  FMT_domain.insert( "BFP32" );
  FMT_domain.insert( "BFP64" );


  if ( ! ( getHFMT( tmp_str )  && 
           sb_Utils::valid_domain( tmp_str, FMT_domain) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "HFMT", tmp_str );



#ifdef NOT_RASTER_PROFILE
  if ( SATP == THREE_TUPLE )
  {
    if ( ! ( getVFMT( tmp_str )   && 
             sb_Utils::valid_domain( tmp_str, FMT_domain) ) )
    {
      return false;
    }
    sb_Utils::add_subfield( record.back(), "VFMT", tmp_str );
  }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "VFMT", sc_Subfield::is_A );
    }
#endif


  if ( ! getSFAX( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "SFAX", tmp_double );



  if ( ! getSFAY( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "SFAY", tmp_double );



#ifdef NOT_RASTER_PROFILE
  if ( SATP == THREE_TUPLE )
  {
    if ( ! getSFAZ( tmp_double ) )
    {
      return false;
    }
    sb_Utils::add_subfield( record.back(), "SFAZ", tmp_double );
  }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "SFAZ", sc_Subfield::is_A );
    }
#endif


  if ( ! getXORG( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "XORG", tmp_double );



  if ( ! getYORG( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "YORG", tmp_double );


#ifdef NOT_RASTER_PROFILE
  if ( SATP == THREE_TUPLE )
  {
    if ( ! getZORG( tmp_double ) )
    {
      return false;
    }
    sb_Utils::add_subfield( record.back(), "ZORG", tmp_double );
  }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "ZORG", sc_Subfield::is_A );
    }
#endif


  if ( ! getXHRS( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "XHRS", tmp_double );



  if ( ! getYHRS( tmp_double ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "YHRS", tmp_double );



#ifdef NOT_RASTER_PROFILE
  if ( SATP == THREE_TUPLE )
  {
    if ( ! getVRES( tmp_double ) )
    {
      return false;
    }
    sb_Utils::add_subfield( record.back(), "VRES", tmp_double );

  }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "VRES", sc_Subfield::is_A );
    }
#endif

  // XXX Dimension ID


  return true;

} // sb_Iref::getRecord




void
sb_Iref::setComment( string const & val ) 
{
  imp_->comment_ = val;
} // sb_Iref::setComment


void
sb_Iref::setSpatialAddressType( string const & val ) 
{
  imp_->spatialAddType_ = val;
} // sb_Iref::setSpatialAddressType


void
sb_Iref::setSpatialAddressXLabel( string const & val ) 
{
  imp_->spatialAddXCompLbl_ = val;
} // sb_Iref::setSpatialAddressXLabel


void
sb_Iref::setSpatialAddressYLabel( string const & val ) 
{   imp_->spatialAddYCompLbl_ = val;
} // sb_Iref::setSpatialAddressYLabel



void
sb_Iref::setHorizontalComponentFormat( string const & val ) 
{   imp_->horizontalCompFmt_ = val;
} // sb_Iref::setHorizontalComponentFormat



#ifdef NOT_RASTER_PROFILE
void
sb_Iref::setVerticalComponentFormat( string const & val ) 
{   imp_->verticalCompFmt_ = val;
} // sb_Iref::setVerticalComponentFormat
#endif

       
void
sb_Iref::setScaleFactorX( double const & val ) 
{   imp_->scaleFactX_ = val;
} // sb_Iref::setScaleFactorX



void
sb_Iref::setScaleFactorY( double const & val ) 
{   imp_->scaleFactY_ = val;
} // sb_Iref::setScaleFactorY



#ifdef NOT_RASTER_PROFILE
void
sb_Iref::setScaleFactorZ( double const & val ) 
{   imp_->scaleFactZ_ = val;
} // sb_Iref::setScaleFactorZ
#endif


void
sb_Iref::setXOrigin( double const & val ) 
{   imp_->xOrigin_ = val;
} // sb_Iref::setXOrigin



void
sb_Iref::setYOrigin( double const & val ) 
{   imp_->yOrigin_ = val;
} // sb_Iref::setYOrigin



#ifdef NOT_RASTER_PROFILE
void
sb_Iref::setZOrigin( double const & val ) 
{   imp_->zOrigin_ = val;
} // sb_Iref::setZOrigin
#endif


      
void
sb_Iref::setXComponentHorizontalResolution( double const & val ) 
{   imp_->xCompHorizRes_ = val;
} // sb_Iref::setXComponentHorizontalResolution



void
sb_Iref::setYComponentHorizontalResolution( double const & val ) 
{   imp_->yCompHorizRes_ = val;
} // sb_Iref::setYComponentHorizontalResolution



#ifdef NOT_RASTER_PROFILE
void
sb_Iref::setVerticalResolutionComponent( double const & val ) 
{   imp_->verticalResComp_ = val;
} // sb_Iref::setVerticalResolutionComponent
#endif



#ifdef NOT_IMPLEMENTED
void
sb_Iref::setDimensionID( vector<sb_ForeignID> const & val ) 
{   imp_->dimensionID_ = val;
} // sb_Iref::setDimensionID
#endif





bool
sb_Iref::setRecord( sc_Record const & record )
{
  return ingest_record_( *this, *(this->imp_), record );
} // sb_Iref::setRecord




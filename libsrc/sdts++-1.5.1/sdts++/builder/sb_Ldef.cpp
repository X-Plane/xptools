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
// $Id: sb_Ldef.cpp,v 1.8 2002/11/24 22:07:42 mcoletti Exp $
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <sdts++/builder/sb_Ldef.h>

#include <iostream>
#include <strstream>

#include <limits.h>
#include <float.h>

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
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


static const char* ident_ = "$Id: sb_Ldef.cpp,v 1.8 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Ldef_Imp
{
  string   CellModuleName_;
  string   LayerLabel_;
  string   CellCode_;
  string   Bitmask_;
  long     NumberRows_;
  long     NumberColumns_;
  long     NumberPlanes_;
  long     ScanOriginRow_;
  long     ScanOriginColumn_;
  long     ScanOriginPlane_;
  long     RowOffsetOrigin_;
  long     ColumnOffsetOrigin_;
  long     PlaneOffsetOrigin_;
  string   IntracellReferenceLocation_;
  string   Comment_;


  sb_Ldef_Imp()
    : 
    CellModuleName_( UNVALUED_STRING ),
    LayerLabel_( UNVALUED_STRING ),
    CellCode_( UNVALUED_STRING ),
    Bitmask_( UNVALUED_STRING ),
    NumberRows_( UNVALUED_LONG ),
    NumberColumns_( UNVALUED_LONG ),
    NumberPlanes_( UNVALUED_LONG ),
    ScanOriginRow_( UNVALUED_LONG ),
    ScanOriginColumn_( UNVALUED_LONG ),
    ScanOriginPlane_( UNVALUED_LONG ),
    RowOffsetOrigin_( UNVALUED_LONG ),
    ColumnOffsetOrigin_( UNVALUED_LONG ),
    PlaneOffsetOrigin_( UNVALUED_LONG ),
    IntracellReferenceLocation_( UNVALUED_STRING ),
    Comment_( UNVALUED_STRING )
  {}

};


sb_Ldef::sb_Ldef()
  : imp_( new sb_Ldef_Imp() )
{
  setMnemonic("LDEF");
  setID( 1 );


  // insert static initializers

} // Ldef ctor


sb_Ldef::~sb_Ldef()
{
  delete imp_;
} // Ldef dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema schema_; // module specific schema


sio_8211Schema&
sb_Ldef::schema_()
{
   return ::schema_;
} // sb_Ldef::schema_()



void
sb_Ldef::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Layer Definition" );
  field_format.setTag( "LDEF" );


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

  field_format.back().setLabel( "CMNM" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "LLBL" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "CODE" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "BMSK" );
//  field_format.back().setType( sio_8211SubfieldFormat::C );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_C );

//  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NROW" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NCOL" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "NPLA" );
//  field_format.back().setType( sio_8211SubfieldFormat::I );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_I );

//  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SORI" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SOCI" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "SOPI" );
//  field_format.back().setType( sio_8211SubfieldFormat::I );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_I );

//  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RWOO" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "CLOO" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "PLOO" );
//  field_format.back().setType( sio_8211SubfieldFormat::I );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_I );

//  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "INTR" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


//  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "COMT" );
//  field_format.back().setType( sio_8211SubfieldFormat::A );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_A );


} // build__schema


static
bool
ingest_record_( sb_Ldef& ldef, sb_Ldef_Imp &ldef_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"LDEF",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Ldef::sb_Ldef(sc_Record const&): "
	 << "Not an layer definition record.";
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
    ldef.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    ldef.setID( tmp_int );
  }


  // CMNM
  if (sb_Utils::getSubfieldByMnem(*curfield,"CMNM",cursubfield))
  {
    cursubfield->getA( ldef_imp.CellModuleName_);
  }

  // LLBL
  if (sb_Utils::getSubfieldByMnem(*curfield,"LLBL",cursubfield))
  {
    cursubfield->getA( ldef_imp.LayerLabel_);
  }

  // CODE
  if (sb_Utils::getSubfieldByMnem(*curfield,"CODE",cursubfield))
  {
    cursubfield->getA( ldef_imp.CellCode_);
  }

  // BMSK
  // if (sb_Utils::getSubfieldByMnem(*curfield,"BMSK",cursubfield))
  // {
  //  cursubfield->getC( ldef_imp.Bitmask_);
  // }

  // NROW
  if (sb_Utils::getSubfieldByMnem(*curfield,"NROW",cursubfield))
  {
    cursubfield->getI( ldef_imp.NumberRows_);
  }

  // NCOL
  if (sb_Utils::getSubfieldByMnem(*curfield,"NCOL",cursubfield))
  {
    cursubfield->getI( ldef_imp.NumberColumns_);
  }

  // NPLA
  // if (sb_Utils::getSubfieldByMnem(*curfield,"NPLA",cursubfield))
  // {
  //  cursubfield->getI( ldef_imp.NumberPlanes_);
  // }

  // SORI
  if (sb_Utils::getSubfieldByMnem(*curfield,"SORI",cursubfield))
  {
    cursubfield->getI( ldef_imp.ScanOriginRow_);
  }

  // SOCI
  if (sb_Utils::getSubfieldByMnem(*curfield,"SOCI",cursubfield))
  {
    cursubfield->getI( ldef_imp.ScanOriginColumn_);
  }

  // SOPI
  // if (sb_Utils::getSubfieldByMnem(*curfield,"SOPI",cursubfield))
  // {
  //  cursubfield->getI( ldef_imp.ScanOriginPlane_);
  // }

  // RWOO
  if (sb_Utils::getSubfieldByMnem(*curfield,"RWOO",cursubfield))
  {
    cursubfield->getI( ldef_imp.RowOffsetOrigin_);
  }

  // CLOO
  if (sb_Utils::getSubfieldByMnem(*curfield,"CLOO",cursubfield))
  {
    cursubfield->getI( ldef_imp.ColumnOffsetOrigin_);
  }

  // PLOO
  // if (sb_Utils::getSubfieldByMnem(*curfield,"PLOO",cursubfield))
  // {
  //  cursubfield->getI( ldef_imp.PlaneOffsetOrigin_);
  // }

  // INTR
  if (sb_Utils::getSubfieldByMnem(*curfield,"INTR",cursubfield))
  {
    cursubfield->getA( ldef_imp.IntracellReferenceLocation_);
  }

  // COMT
  // if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
  // {
  //   cursubfield->getA( ldef_imp.Comment_);
  // }

  return true;


} // ingest__record




bool
sb_Ldef::getCellModuleName( string& val ) const
{
  if ( imp_->CellModuleName_ == UNVALUED_STRING )
    return false;

  val = imp_->CellModuleName_;

  return true;
} // sb_Ldef::getCellModuleName


bool
sb_Ldef::getLayerLabel( string& val ) const
{
  if ( imp_->LayerLabel_ == UNVALUED_STRING )
    return false;

  val = imp_->LayerLabel_;

  return true;
} // sb_Ldef::getLayerLabel


bool
sb_Ldef::getCellCode( string& val ) const
{
  if ( imp_->CellCode_ == UNVALUED_STRING )
    return false;

  val = imp_->CellCode_;

  return true;
} // sb_Ldef::getCellCode


// bool
// sb_Ldef::getBitmask( string& val ) const
// {
//  if ( imp_->Bitmask_ == UNVALUED_STRING )
//    return false;
//
//  val = imp_->Bitmask_;
//
//  return true;
// } // sb_Ldef::getBitmask


bool
sb_Ldef::getNumberRows( long& val ) const
{
  if ( imp_->NumberRows_ == UNVALUED_LONG )
    return false;

  val = imp_->NumberRows_;

  return true;
} // sb_Ldef::getNumberRows


bool
sb_Ldef::getNumberColumns( long& val ) const
{
  if ( imp_->NumberColumns_ == UNVALUED_LONG )
    return false;

  val = imp_->NumberColumns_;

  return true;
} // sb_Ldef::getNumberColumns


// bool
// sb_Ldef::getNumberPlanes( long& val ) const
// {
//  if ( imp_->NumberPlanes_ == UNVALUED_LONG )
//    return false;
//
//  val = imp_->NumberPlanes_;
//
//  return true;
// } // sb_Ldef::getNumberPlanes


bool
sb_Ldef::getScanOriginRow( long& val ) const
{
  if ( imp_->ScanOriginRow_ == UNVALUED_LONG )
    return false;

  val = imp_->ScanOriginRow_;

  return true;
} // sb_Ldef::getScanOriginRow


bool
sb_Ldef::getScanOriginColumn( long& val ) const
{
  if ( imp_->ScanOriginColumn_ == UNVALUED_LONG )
    return false;

  val = imp_->ScanOriginColumn_;

  return true;
} // sb_Ldef::getScanOriginColumn


// bool
// sb_Ldef::getScanOriginPlane( long& val ) const
// {
//  if ( imp_->ScanOriginPlane_ == UNVALUED_LONG )
//    return false;
//
//  val = imp_->ScanOriginPlane_;
//
//  return true;
// } // sb_Ldef::getScanOriginPlane


bool
sb_Ldef::getRowOffsetOrigin( long& val ) const
{
  if ( imp_->RowOffsetOrigin_ == UNVALUED_LONG )
    return false;

  val = imp_->RowOffsetOrigin_;

  return true;
} // sb_Ldef::getRowOffsetOrigin


bool
sb_Ldef::getColumnOffsetOrigin( long& val ) const
{
  if ( imp_->ColumnOffsetOrigin_ == UNVALUED_LONG )
    return false;

  val = imp_->ColumnOffsetOrigin_;

  return true;
} // sb_Ldef::getColumnOffsetOrigin


// bool
// sb_Ldef::getPlaneOffsetOrigin( long& val ) const
// {
//  if ( imp_->PlaneOffsetOrigin_ == UNVALUED_LONG )
//    return false;
//
//  val = imp_->PlaneOffsetOrigin_;
//
//  return true;
// } // sb_Ldef::getPlaneOffsetOrigin


bool
sb_Ldef::getIntracellReferenceLocation( string& val ) const
{
  if ( imp_->IntracellReferenceLocation_ == UNVALUED_STRING )
    return false;

  val = imp_->IntracellReferenceLocation_;

  return true;
} // sb_Ldef::getIntracellReferenceLocation


// bool
// sb_Ldef::getComment( string& val ) const
// {
//  if ( imp_->Comment_ == UNVALUED_STRING )
//    return false;
//
//  val = imp_->Comment_;
//
//  return true;
// } // sb_Ldef::getComment






bool
sb_Ldef::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "LDEF" );

  record.back().setName( "Layer Definition" );

  string tmp_str;
  long   tmp_long;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getCellModuleName( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"CMNM", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "CMNM", sc_Subfield::is_A );
  }


  if ( getLayerLabel( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"LLBL", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "LLBL", sc_Subfield::is_A );
  }


  if ( getCellCode( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"CODE", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "CODE", sc_Subfield::is_A );
  }


//  if ( getBitmask( tmp_str ) )
//  {
//    sb_Utils::add_subfield( record.back(),"BMSK", tmp_str );
//  }
//  else
//  {
//    sb_Utils::add_empty_subfield( record.back(), "BMSK", sc_Subfield::is_C );
//  }


  if ( getNumberRows( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"NROW", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "NROW", sc_Subfield::is_I );
  }


  if ( getNumberColumns( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"NCOL", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "NCOL", sc_Subfield::is_I );
  }


//  if ( getNumberPlanes( tmp_long ) )
//  {
//    sb_Utils::add_subfield( record.back(),"NPLA", tmp_long );
//  }
//  else
//  {
//    sb_Utils::add_empty_subfield( record.back(), "NPLA", sc_Subfield::is_I );
//  }


  if ( getScanOriginRow( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"SORI", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "SORI", sc_Subfield::is_I );
  }


  if ( getScanOriginColumn( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"SOCI", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "SOCI", sc_Subfield::is_I );
  }


  // if ( getScanOriginPlane( tmp_long ) )
  // {
  //  sb_Utils::add_subfield( record.back(),"SOPI", tmp_long );
  // }
  // else
  // {
  //  sb_Utils::add_empty_subfield( record.back(), "SOPI", sc_Subfield::is_I );
  // }


  if ( getRowOffsetOrigin( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"RWOO", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "RWOO", sc_Subfield::is_I );
  }


  if ( getColumnOffsetOrigin( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"CLOO", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "CLOO", sc_Subfield::is_I );
  }


  // if ( getPlaneOffsetOrigin( tmp_long ) )
  // {
  //   sb_Utils::add_subfield( record.back(),"PLOO", tmp_long );
  // }
  // else
  // {
  //   sb_Utils::add_empty_subfield( record.back(), "PLOO", sc_Subfield::is_I );
  // }


  if ( getIntracellReferenceLocation( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"INTR", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "INTR", sc_Subfield::is_A );
  }


  // if ( getComment( tmp_str ) )
  // {
  //  sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  // }
  // else
  // {
  //   sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  // }


  return true;


} // Ldef::getRecord




bool
sb_Ldef::setCellModuleName( string const& val )
{
  imp_->CellModuleName_ = val;

  return true;
} // sb_Ldef::setCellModuleName


bool
sb_Ldef::setLayerLabel( string const& val )
{
  imp_->LayerLabel_ = val;

  return true;
} // sb_Ldef::setLayerLabel


bool
sb_Ldef::setCellCode( string const& val )
{
  imp_->CellCode_ = val;

  return true;
} // sb_Ldef::setCellCode


// bool
// sb_Ldef::setBitmask( string const& val )
// {
// imp_->Bitmask_ = val;
//
//  return true;
// } // sb_Ldef::setBitmask


bool
sb_Ldef::setNumberRows( long val )
{
  imp_->NumberRows_ = val;

  return true;
} // sb_Ldef::setNumberRows


bool
sb_Ldef::setNumberColumns( long val )
{
  imp_->NumberColumns_ = val;

  return true;
} // sb_Ldef::setNumberColumns


// bool
// sb_Ldef::setNumberPlanes( long val )
// {
//  imp_->NumberPlanes_ = val;
//
//  return true;
// } // sb_Ldef::setNumberPlanes


bool
sb_Ldef::setScanOriginRow( long val )
{
  imp_->ScanOriginRow_ = val;

  return true;
} // sb_Ldef::setScanOriginRow


bool
sb_Ldef::setScanOriginColumn( long val )
{
  imp_->ScanOriginColumn_ = val;

  return true;
} // sb_Ldef::setScanOriginColumn


// bool
// sb_Ldef::setScanOriginPlane( long val )
// {
//   imp_->ScanOriginPlane_ = val;
//
//   return true;
// } // sb_Ldef::setScanOriginPlane


bool
sb_Ldef::setRowOffsetOrigin( long val )
{
  imp_->RowOffsetOrigin_ = val;

  return true;
} // sb_Ldef::setRowOffsetOrigin


bool
sb_Ldef::setColumnOffsetOrigin( long val )
{
  imp_->ColumnOffsetOrigin_ = val;

  return true;
} // sb_Ldef::setColumnOffsetOrigin


// bool
// sb_Ldef::setPlaneOffsetOrigin( long val )
// {
//  imp_->PlaneOffsetOrigin_ = val;
//
//  return true;
// } // sb_Ldef::setPlaneOffsetOrigin


bool
sb_Ldef::setIntracellReferenceLocation( string const& val )
{
  imp_->IntracellReferenceLocation_ = val;

  return true;
} // sb_Ldef::setIntracellReferenceLocation


// bool
// sb_Ldef::setComment( string const& val )
// {
//   imp_->Comment_ = val;
//
//   return true;
// } // sb_Ldef::setComment


bool
sb_Ldef::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Ldef::setRecord




void
sb_Ldef::unDefineCellModuleName( )
{
  imp_->CellModuleName_ = UNVALUED_STRING;
} // sb_Ldef::unDefineCellModuleName


void
sb_Ldef::unDefineLayerLabel( )
{
  imp_->LayerLabel_ = UNVALUED_STRING;
} // sb_Ldef::unDefineLayerLabel


void
sb_Ldef::unDefineCellCode( )
{
  imp_->CellCode_ = UNVALUED_STRING;
} // sb_Ldef::unDefineCellCode


// void
// sb_Ldef::unDefineBitmask( )
// {
//  imp_->Bitmask_ = UNVALUED_STRING;
// } // sb_Ldef::unDefineBitmask


void
sb_Ldef::unDefineNumberRows( )
{
  imp_->NumberRows_ = UNVALUED_LONG;
} // sb_Ldef::unDefineNumberRows


void
sb_Ldef::unDefineNumberColumns( )
{
  imp_->NumberColumns_ = UNVALUED_LONG;
} // sb_Ldef::unDefineNumberColumns


// void
// sb_Ldef::unDefineNumberPlanes( )
// {
//  imp_->NumberPlanes_ = UNVALUED_LONG;
// } // sb_Ldef::unDefineNumberPlanes


void
sb_Ldef::unDefineScanOriginRow( )
{
  imp_->ScanOriginRow_ = UNVALUED_LONG;
} // sb_Ldef::unDefineScanOriginRow


void
sb_Ldef::unDefineScanOriginColumn( )
{
  imp_->ScanOriginColumn_ = UNVALUED_LONG;
} // sb_Ldef::unDefineScanOriginColumn


// void
// sb_Ldef::unDefineScanOriginPlane( )
// {
//   imp_->ScanOriginPlane_ = UNVALUED_LONG;
// } // sb_Ldef::unDefineScanOriginPlane


void
sb_Ldef::unDefineRowOffsetOrigin( )
{
  imp_->RowOffsetOrigin_ = UNVALUED_LONG;
} // sb_Ldef::unDefineRowOffsetOrigin


void
sb_Ldef::unDefineColumnOffsetOrigin( )
{
  imp_->ColumnOffsetOrigin_ = UNVALUED_LONG;
} // sb_Ldef::unDefineColumnOffsetOrigin


// void
// sb_Ldef::unDefinePlaneOffsetOrigin( )
// {
//   imp_->PlaneOffsetOrigin_ = UNVALUED_LONG;
// } // sb_Ldef::unDefinePlaneOffsetOrigin


void
sb_Ldef::unDefineIntracellReferenceLocation( )
{
  imp_->IntracellReferenceLocation_ = UNVALUED_STRING;
} // sb_Ldef::unDefineIntracellReferenceLocation


// void
// sb_Ldef::unDefineComment( )
// {
//   imp_->Comment_ = UNVALUED_STRING;
// } // sb_Ldef::unDefineComment



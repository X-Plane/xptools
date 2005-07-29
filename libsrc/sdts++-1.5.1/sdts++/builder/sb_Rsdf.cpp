//   PHY - attempt to add support for other fields
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
// $Id: sb_Rsdf.cpp,v 1.11 2002/11/24 22:07:42 mcoletti Exp $
//

#include <sdts++/builder/sb_Rsdf.h>



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



static const char* ident_ = "$Id: sb_Rsdf.cpp,v 1.11 2002/11/24 22:07:42 mcoletti Exp $";

// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Rsdf_Imp
{
  string   ObjectRepresentation_;
  string   CellSequencingCode_;
  string   AcquisitionDeviceMethod_;
  string   AcquisitionDate_;
  string   Comments_;
  string   DefaultImplementation_;
  string   Compression_;
  string   CodingMethod_;
  long     RowExtent_;
  long     ColumnExtent_;
  // long     PlaneExtent_;  // Phy - cut out unneeded stuff
  string   ScanOrigin_;
  string   ScanPattern_;
  string   TesseralIndexing_; 
  // string   TesseralIndexFormat_;   -- Phy - cut out unneeded stuff
  // string   TesseralIndexingDescription_; -- Phy - cut out unneeded stuff
  long     NumberLinesAlternation_;
  string   FirstScanDirection_;
  double   AspectRatio_;
  long     NumberLayers_;
  double   SADR__X;
  double   SADR__Y;
 
  sb_ForeignID isid_;           // internal spatial foreign ID
  sb_ForeignIDs  lyids_;          // layer ID foreign ID container
  sb_ForeignIDs  ratps_;          // raster attribute foreign ID container

  sb_Rsdf_Imp()
    :
    ObjectRepresentation_( UNVALUED_STRING ),
    CellSequencingCode_( UNVALUED_STRING ),
    AcquisitionDeviceMethod_( UNVALUED_STRING ),
    AcquisitionDate_( UNVALUED_STRING ),
    Comments_( UNVALUED_STRING ),
    DefaultImplementation_( UNVALUED_STRING ),
    Compression_( UNVALUED_STRING ),
    CodingMethod_( UNVALUED_STRING ),
    RowExtent_( UNVALUED_LONG ),
    ColumnExtent_( UNVALUED_LONG ),
  //  PlaneExtent_( UNVALUED_LONG ),  Phy - cut out unneeded stuff
    ScanOrigin_( UNVALUED_STRING ),
    ScanPattern_( UNVALUED_STRING ),
    TesseralIndexing_( UNVALUED_STRING ), 
  //  TesseralIndexFormat_( UNVALUED_STRING ),  Phy - cut out unneeded stuff
  //  TesseralIndexingDescription_( UNVALUED_STRING ), Phy - cut out unneeded stuff
    NumberLinesAlternation_( UNVALUED_LONG ),
    FirstScanDirection_( UNVALUED_STRING ),
    AspectRatio_( UNVALUED_DOUBLE ),
    NumberLayers_( UNVALUED_LONG ),
    SADR__X( UNVALUED_DOUBLE ),
    SADR__Y( UNVALUED_DOUBLE )
  {}

}; // sb_Rsdf_Imp


sb_Rsdf::sb_Rsdf()
  : imp_( new sb_Rsdf_Imp() )
{
  setMnemonic("RSDF");
  setID( 1 );


  // insert static initializers

} // Rsdf ctor


sb_Rsdf::~sb_Rsdf()
{
  delete imp_ ;
} // Rsdf dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;

static sio_8211Schema schema_; // module specific schema



sio_8211Schema&
sb_Rsdf::schema_()
{
   return ::schema_;
} // sb_Rsdf::schema_()



void
sb_Rsdf::buildSpecificSchema_( )
{
  schema_().push_back( sio_8211FieldFormat() );

  // Phy - throughout here I added in missing required subfields, and excluded
  //        unneeded subfields and entire fields.

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Raster Definition" );
  field_format.setTag( "RSDF" );


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


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "CSCD" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "AQMD" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "AQDT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DEFI" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "CMPR" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "METH" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RWXT" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "CLXT" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );

// Phy - cut out all unneeded stuff
//  field_format.push_back( sio_8211SubfieldFormat() );
//
//  field_format.back().setLabel( "PLXT" );
//  field_format.back().setType( sio_8211SubfieldFormat::I );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SCOR" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SCPT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "TIDX" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

// Phy - cut out unneeded stuff
//  field_format.push_back( sio_8211SubfieldFormat() );
//
//  field_format.back().setLabel( "TIFT" );
//  field_format.back().setType( sio_8211SubfieldFormat::A );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_A );
//
//
//  field_format.push_back( sio_8211SubfieldFormat() );
//
//  field_format.back().setLabel( "TIDS" );
//  field_format.back().setType( sio_8211SubfieldFormat::A );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ALTN" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "FSCN" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "ASPR" );
  field_format.back().setType( sio_8211SubfieldFormat::R );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_R );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "NLAY" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );


				// ISID

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::vector );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "Internal Spatial Id" );
  schema_().back().setTag( "ISID" );


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


// Phy - cut out all unneeded stuff for now
//
//
				// RDXT
//
//  schema_().push_back( sio_8211FieldFormat() );
//
//  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
//  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
//  schema_().back().setName( "Raster Dimension Extent" );
//  schema_().back().setTag( "RDXT" );
//
//
//  schema_().back().push_back( sio_8211SubfieldFormat() );
//
//  schema_().back().back().setLabel( "DEXT" );
//  schema_().back().back().setType( sio_8211SubfieldFormat::I );
//  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
//  schema_().back().back().setConverter( &converter_I );
//
//
//  schema_().back().push_back( sio_8211SubfieldFormat() );
//
//  schema_().back().back().setLabel( "DSCO" );
//  schema_().back().back().setType( sio_8211SubfieldFormat::A );
//  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
//  schema_().back().back().setConverter( &converter_A );


				// SADR

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::vector );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "Spatial Address" );
  schema_().back().setTag( "SADR" );


				// XXX Need finer grain control of
				// XXX coordinate format; arbitrarily
				// XXX using 'R'

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "X" );
  schema_().back().back().setType( sio_8211SubfieldFormat::R );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_R );


  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "Y" );
  schema_().back().back().setType( sio_8211SubfieldFormat::R );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_R );



#ifdef NOT_IMPLEMENTED

				// XXLB

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "X-Axis Label" );
  schema_().back().setTag( "XXLB" );


				// YXLB

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "Y-Axis Label" );
  schema_().back().setTag( "YXLB" );

#endif

				// LYID

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "Layer ID" );
  schema_().back().setTag( "LYID" );


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



			// RATP

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "Raster Attribute ID" );
  schema_().back().setTag( "RATP" );


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


// Phy - cut out all unneeded stuff for now
//
				// CPID
//
//  schema_().push_back( sio_8211FieldFormat() );
//
//  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
//  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
//  schema_().back().setName( "Composite ID" );
//  schema_().back().setTag( "CPID" );
//
//
//  schema_().back().push_back( sio_8211SubfieldFormat() );
//
//  schema_().back().back().setLabel( "MODN" );
//  schema_().back().back().setType( sio_8211SubfieldFormat::A );
//  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
//  schema_().back().back().setConverter( &converter_A );
//
//
//  schema_().back().push_back( sio_8211SubfieldFormat() );
//
//  schema_().back().back().setLabel( "RCID" );
//  schema_().back().back().setType( sio_8211SubfieldFormat::I );
//  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
//  schema_().back().back().setConverter( &converter_I );

} // build__schema





//
// NOTE THAT THIS ONLY GETS THE FIRST FIELD!!
// Phy -- attempt to add support for other fields
//
static
bool
ingest_record_( sb_Rsdf& rsdf, sb_Rsdf_Imp &rsdf_imp, sc_Record const& record )
{

  // Make sure we have a record from an
  // External Spatial Reference module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"RSDF",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Rsdf::sb_Rsdf(sc_Record const&): "
	 << "Not an raster definition record.";
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
    rsdf.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    rsdf.setID( tmp_int );
  }


  // OBRP
  if (sb_Utils::getSubfieldByMnem(*curfield,"OBRP",cursubfield))
  {
    cursubfield->getA( rsdf_imp.ObjectRepresentation_);
  }

  // CSCD
  if (sb_Utils::getSubfieldByMnem(*curfield,"CSCD",cursubfield))
  {
    cursubfield->getA( rsdf_imp.CellSequencingCode_);
  }
//    else
//    {
//      return false;
//    }


  // Phy -- subfields CMPR and METH moved to after DEFI

  // AQMD
  if (sb_Utils::getSubfieldByMnem(*curfield,"AQMD",cursubfield))
  {
    cursubfield->getA( rsdf_imp.AcquisitionDeviceMethod_);
  }

  // AQDT
  if (sb_Utils::getSubfieldByMnem(*curfield,"AQDT",cursubfield))
  {
    cursubfield->getA( rsdf_imp.AcquisitionDate_);
  }

  // COMT
  if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
  {
    cursubfield->getA( rsdf_imp.Comments_);
  }

  // DEFI
  if (sb_Utils::getSubfieldByMnem(*curfield,"DEFI",cursubfield))
  {
    cursubfield->getA( rsdf_imp.DefaultImplementation_);
  }

  // CMPR -- Phy - moved this after DEFI
  if (sb_Utils::getSubfieldByMnem(*curfield,"CMPR",cursubfield))
  {
    cursubfield->getA( rsdf_imp.Compression_);
  }

  // METH -- Phy move this after CMPR
  if (sb_Utils::getSubfieldByMnem(*curfield,"METH",cursubfield))
  {
    cursubfield->getA( rsdf_imp.CodingMethod_);  // Phy - fix name
  }

  // RWXT
  if (sb_Utils::getSubfieldByMnem(*curfield,"RWXT",cursubfield))
  {
    cursubfield->getI( rsdf_imp.RowExtent_); //Phy - fix spelling
  }

  // CLXT
  if (sb_Utils::getSubfieldByMnem(*curfield,"CLXT",cursubfield))
  {
    cursubfield->getI( rsdf_imp.ColumnExtent_);
  }

  // PLXT -- Phy - cutout unneeded stuff
  //if (sb_Utils::getSubfieldByMnem(*curfield,"PLXT",cursubfield))
  // {
  //  cursubfield->getI( rsdf_imp.PlaneExtent_);
  //}
  //else
  //{
  //  return false;
  //}


  // SCOR
  if (sb_Utils::getSubfieldByMnem(*curfield,"SCOR",cursubfield))
  {
    cursubfield->getA( rsdf_imp.ScanOrigin_);
  }

   // SCPT - Phy - moved to after SCOR
  if (sb_Utils::getSubfieldByMnem(*curfield,"SCPT",cursubfield))
  {
    cursubfield->getA( rsdf_imp.ScanPattern_);
  }

  // TIDX
  if (sb_Utils::getSubfieldByMnem(*curfield,"TIDX",cursubfield))
  {
    cursubfield->getA( rsdf_imp.TesseralIndexing_);
  }

  // TIFT -- Phy - cut out unneeded stuff
  //if (sb_Utils::getSubfieldByMnem(*curfield,"TIFT",cursubfield))
  //{
  //  cursubfield->getA( rsdf_imp.TesseralIndexFormat_);
  //}
  //else
  //{
  //  return false;
  // }


  // TIDS -- Phy - cut out unneeded stuff
  //if (sb_Utils::getSubfieldByMnem(*curfield,"TIDS",cursubfield))
  //{
  //  cursubfield->getA( rsdf_imp.TesseralIndexingDescription_);
  //}
  //else
  //{
  //  return false;
  //}


  // ALTN
  if (sb_Utils::getSubfieldByMnem(*curfield,"ALTN",cursubfield))
  {
    cursubfield->getI( rsdf_imp.NumberLinesAlternation_);
  }

  // FSCN
  if (sb_Utils::getSubfieldByMnem(*curfield,"FSCN",cursubfield))
  {
    cursubfield->getA( rsdf_imp.FirstScanDirection_);
  }

  // ASPR
  if (sb_Utils::getSubfieldByMnem(*curfield,"ASPR",cursubfield))
  {
    cursubfield->getR( rsdf_imp.AspectRatio_); //Phy - fix spelling
  }

  // NLAY
  if (sb_Utils::getSubfieldByMnem(*curfield,"NLAY",cursubfield))
  {
    cursubfield->getI( rsdf_imp.NumberLayers_);
  }

  // Phy ??? -- Add support for secondary field ISID

  if ( ! sb_Utils::getFieldByMnem( record,"ISID",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Rsdf::sb_Rsdf(sc_Record const&): "
	 << "No Secondary ISID field.";
    cerr << endl;
#endif
    return false;
  }


  // We have a secondary ISID field. Start picking it apart.

  // Phy ??? - will Mnem MODN work here or does it need to be different to avoid confusion ???
  // ISID_MODN

  string tmp_string;

  if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
  {
    cursubfield->getA( tmp_string );
    rsdf_imp.isid_.moduleName( tmp_string );
  }

  // ISID_RCID

  long tmp_long;

  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_long );
    rsdf_imp.isid_.recordID( tmp_long );
  }

  // Phy ??? -- Add support for secondary field SADR

  if ( ! sb_Utils::getFieldByMnem( record,"SADR",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Rsdf::sb_Rsdf(sc_Record const&): "
	 << "No Secondary SADR field.";
    cerr << endl;
#endif
    return false;
  }


  // We have a secondary SADR field. Start picking it apart.

  // SADR__X
  if (sb_Utils::getSubfieldByMnem(*curfield,"X",cursubfield))
  {
	  // Phy - ??? - didn't check on this call name ..
    cursubfield->getR( rsdf_imp.SADR__X);
  }
  // SADR__Y
  if (sb_Utils::getSubfieldByMnem(*curfield,"Y",cursubfield))
  {
    cursubfield->getR( rsdf_imp.SADR__Y);
  }

  // Phy ??? -- Add support for secondary field LYID     ??? REPEATING FIELD ???

  // XXX This still needs to be done.  The getFieldByMnem mechanism
  // XXX breaks down in the case of repeating fields.  Either something
  // XXX will have to be added to sb_Utils to handle this case, or we'll
  // XXX have to unravel the record by hand here.  Since we're not needing to
  // XXX read Raster records yet , I'll put off implementing this for now.
 
  if ( ! sb_Utils::getFieldByMnem( record,"ISID",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Rsdf::sb_Rsdf(sc_Record const&): "
	 << "No Secondary ISID field.";
    cerr << endl;
#endif
    return false;
  }


#ifdef NOT_IMPLEMENTED

  // Phy ??? - will Mnem MODN work here or does it need to be different to avoid confusion ???
  // LYID_MODN
  if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
  {
    cursubfield->getA( rsdf_imp.LYID__MODN);
  }
  else
  {
    return false;
  }

  // LYID_RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( rsdf_imp.LYID__RCID);
  }
  else
  {
    return false;
  }


  // Phy ??? -- Add support for secondary field RATP  ??? REPEATING FIELD ???

  if ( ! sb_Utils::getFieldByMnem( record,"RATP",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Rsdf::sb_Rsdf(sc_Record const&): "
	 << "No Secondary RATP field.";
    cerr << endl;
#endif
    return false;
  }


  // Phy ??? - will Mnem MODN work here or does it need to be
  // different to avoid confusion ???

  // XXX The same thing applies to this field as what's down for the
  // XXX previous one.

  // RATP_MODN
  if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
  {
    cursubfield->getA( rsdf_imp.RATP__MODN);
  }
  else
  {
    return false;
  }

  // RATP_RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( rsdf_imp.RATP__RCID);
  }
  else
  {
    return false;
  }

#endif // NOT_IMPLEMENTED


  return true;


} // ingest__record




bool
sb_Rsdf::getObjectRepresentation( string& val ) const
{
  if ( imp_->ObjectRepresentation_ == UNVALUED_STRING )
    return false;

  val = imp_->ObjectRepresentation_;

  return true;
} // sb_Rsdf::getObjectRepresentation


bool
sb_Rsdf::getCellSequencingCode( string& val ) const
{
  if ( imp_->CellSequencingCode_ == UNVALUED_STRING )
    return false;

  val = imp_->CellSequencingCode_;

  return true;
} // sb_Rsdf::getCellSequencingCode


bool
sb_Rsdf::getEncodingMethod( string& val ) const
{
  if ( imp_->CodingMethod_ == UNVALUED_STRING )
    return false;

  val = imp_->CodingMethod_;

  return true;
} // sb_Rsdf::getEncodingMethod


bool
sb_Rsdf::getCompression( string& val ) const
{
  if ( imp_->Compression_ == UNVALUED_STRING )
    return false;

  val = imp_->Compression_;

  return true;
} // sb_Rsdf::getCompression


bool
sb_Rsdf::getAcquisitionDeviceMethod( string& val ) const
{
  if ( imp_->AcquisitionDeviceMethod_ == UNVALUED_STRING )
    return false;

  val = imp_->AcquisitionDeviceMethod_;

  return true;
} // sb_Rsdf::getAcquisitionDeviceMethod


bool
sb_Rsdf::getAcquisitionDate( string& val ) const
{
  if ( imp_->AcquisitionDate_ == UNVALUED_STRING )
    return false;

  val = imp_->AcquisitionDate_;

  return true;
} // sb_Rsdf::getAcquisitionDate


bool
sb_Rsdf::getComments( string& val ) const
{
  if ( imp_->Comments_ == UNVALUED_STRING )
    return false;

  val = imp_->Comments_;

  return true;
} // sb_Rsdf::getComments


bool
sb_Rsdf::getDefaultImplementation( string& val ) const
{
  if ( imp_->DefaultImplementation_ == UNVALUED_STRING )
    return false;

  val = imp_->DefaultImplementation_;

  return true;
} // sb_Rsdf::getDefaultImplementation


bool
sb_Rsdf::getRowExtent( long& val ) const
{
  if ( imp_->RowExtent_ == UNVALUED_LONG )
    return false;

  val = imp_->RowExtent_;

  return true;
} // sb_Rsdf::getRowExtent


bool
sb_Rsdf::getColumnExtent( long& val ) const
{
  if ( imp_->ColumnExtent_ == UNVALUED_LONG )
    return false;

  val = imp_->ColumnExtent_;

  return true;
} // sb_Rsdf::getColumnExtent


// Phy -- cut out unneeded stuff
//bool
//sb_Rsdf::getPlaneExtent( long& val ) const
//{
//  if ( imp_->PlaneExtent_ == UNVALUED_LONG )
//    return false;
//
//  val = imp_->PlaneExtent_;
//
//  return true;
//} // sb_Rsdf::getPlaneExtent


bool
sb_Rsdf::getScanOrigin( string& val ) const
{
  if ( imp_->ScanOrigin_ == UNVALUED_STRING )
    return false;

  val = imp_->ScanOrigin_;

  return true;
} // sb_Rsdf::getScanOrigin


bool
sb_Rsdf::getTesseralIndexing( string& val ) const
{
  if ( imp_->TesseralIndexing_ == UNVALUED_STRING )
    return false;

  val = imp_->TesseralIndexing_;

  return true;
} // sb_Rsdf::getTesseralIndexing


bool
sb_Rsdf::getScanPattern( string& val ) const
{
  if ( imp_->ScanPattern_ == UNVALUED_STRING )
    return false;

  val = imp_->ScanPattern_;

  return true;
} // sb_Rsdf::getScanPattern


// Phy - cut out unneeded stuff
//bool
//sb_Rsdf::getTesseralIndexFormat( string& val ) const
//{
//  if ( imp_->TesseralIndexFormat_ == UNVALUED_STRING )
//    return false;
//
//  val = imp_->TesseralIndexFormat_;
//
//  return true;
//} // sb_Rsdf::getTesseralIndexFormat
//
//
//bool
//sb_Rsdf::getTesseralIndexingDescription( string& val ) const
//{
// if ( imp_->TesseralIndexingDescription_ == UNVALUED_STRING )
//    return false;
//
//  val = imp_->TesseralIndexingDescription_;
//
//  return true;
//} // sb_Rsdf::getTesseralIndexingDescription


bool
sb_Rsdf::getNumberLinesAlternation( long& val ) const
{
  if ( imp_->NumberLinesAlternation_ == UNVALUED_LONG )
    return false;

  val = imp_->NumberLinesAlternation_;

  return true;
} // sb_Rsdf::getNumberLinesAlternation


bool
sb_Rsdf::getFirstScanDirection( string& val ) const
{
  if ( imp_->FirstScanDirection_ == UNVALUED_STRING )
    return false;

  val = imp_->FirstScanDirection_;

  return true;
} // sb_Rsdf::getFirstScanDirection


bool
sb_Rsdf::getAspectRation( double& val ) const
{
  if ( imp_->AspectRatio_ == UNVALUED_DOUBLE )
    return false;

  val = imp_->AspectRatio_;

  return true;
} // sb_Rsdf::getAspectRation


bool
sb_Rsdf::getNumberLayers( long& val ) const
{
  if ( imp_->NumberLayers_ == UNVALUED_LONG )
    return false;

  val = imp_->NumberLayers_;

  return true;
} // sb_Rsdf::getNumberLayers



bool
sb_Rsdf::getSpatialAddress( double& x, double& y ) const
{
  if ( imp_->SADR__X == UNVALUED_DOUBLE || imp_->SADR__Y == UNVALUED_DOUBLE )
    return false;

  x = imp_->SADR__X;
  y = imp_->SADR__Y;

  return true;
} // sb_Rsdf::getSpatialAddress


bool
sb_Rsdf::getInternalSpatialId( sb_ForeignID & fid ) const
{
  if ( imp_->isid_.moduleName() == "" || imp_->isid_.recordID() == 0 )
    {
      return false;
    }

  fid = imp_->isid_;

  return true;
} 


bool
sb_Rsdf::getLayerIds( sb_ForeignIDs& fids ) const
{
  if ( imp_->lyids_.empty() )
    {
      return false;
    }

  fids = imp_->lyids_;

  return true;
} 



bool
sb_Rsdf::getRasterAttributeIds( sb_ForeignIDs& fids ) const
{
  if ( imp_->ratps_.empty() )
    {
      return false;
  }

  fids = imp_->ratps_;

  return true;
} 





bool
sb_Rsdf::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "RSDF" );

  record.back().setName( "Raster Definition" );

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
    return false;
  }


  if ( getCellSequencingCode( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"CSCD", tmp_str );
  }
  else
  {
    return false;
  }


  if ( getAcquisitionDeviceMethod( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"AQMD", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "AQMD", sc_Subfield::is_A );
  }


  if ( getAcquisitionDate( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"AQDT", tmp_str );
  }
  else
  {
    // Phy - change to not make date mandatory 
    sb_Utils::add_empty_subfield( record.back(), "AQDT", sc_Subfield::is_A );
  }


  if ( getComments( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }


  if ( getDefaultImplementation( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"DEFI", tmp_str );
  }
  else
  {
    return false;
  }


  if ( getCompression( tmp_str ) ) // XXX Mandatory?
  {
    sb_Utils::add_subfield( record.back(),"CMPR", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "CMPR", sc_Subfield::is_A );
  }


  if ( getEncodingMethod( tmp_str ) ) // XXX Mandatory?
  {
    sb_Utils::add_subfield( record.back(),"METH", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "METH", sc_Subfield::is_A );
  }



  if ( getRowExtent( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"RWXT", tmp_long );
  }
  else
  {
    return false;
  }


  if ( getColumnExtent( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"CLXT", tmp_long );
  }
  else
  {
    return false;
  }

  
  // Phy - cut out unneeded stuff
  //if ( getPlaneExtent( tmp_long ) )
  //{
  // sb_Utils::add_subfield( record.back(),"PLXT", tmp_long );
  //}
  //else
  //{
  //  sb_Utils::add_empty_subfield( record.back(), "PLXT", sc_Subfield::is_I );
  //}


  if ( getScanOrigin( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"SCOR", tmp_str );
  }
  else
  {
    return false;
  }


  if ( getScanPattern( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"SCPT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "SCPT", sc_Subfield::is_A );
  }


  if ( getTesseralIndexing( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"TIDX", tmp_str );
  }
  else
  {
    return false;
  }



// Phy - cut out unneeded stuff
//  if ( getTesseralIndexFormat( tmp_str ) )
//  {
//    sb_Utils::add_subfield( record.back(),"TIFT", tmp_str );
//  }
//  else
//  {
//    sb_Utils::add_empty_subfield( record.back(), "TIFT", sc_Subfield::is_A );
//  }
//
//
//  if ( getTesseralIndexingDescription( tmp_str ) )
//  {
//    sb_Utils::add_subfield( record.back(),"TIDS", tmp_str );
//  }
//  else
//  {
//    sb_Utils::add_empty_subfield( record.back(), "TIDS", sc_Subfield::is_A );
//  }


  if ( getNumberLinesAlternation( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"ALTN", tmp_long );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "ALTN", sc_Subfield::is_I );
  }


  if ( getFirstScanDirection( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"FSCN", tmp_str );
  }
  else
  {
    return false;
  }


  if ( getAspectRation( tmp_double ) )
  {
    sb_Utils::add_subfield( record.back(),"ASPR", tmp_double );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "ASPR", sc_Subfield::is_R );
  }


  if ( getNumberLayers( tmp_long ) )
  {
    sb_Utils::add_subfield( record.back(),"NLAY", tmp_long );
  }
  else
  {
    return false;
  }



  // PHY -- Field ISID

  record.push_back( sc_Field() );

  record.back().setMnemonic( "ISID" );

  record.back().setName( "Internal Spatial Id" );


  sb_ForeignID isid;

  if ( getISID( isid ) )
  {
    sb_Utils::add_subfield( record.back(),"MODN", isid.moduleName() );
    sb_Utils::add_subfield( record.back(),"RCID", isid.recordID() );
  }
  else
  {
    return false;
  }



  // Field SADR 

  record.push_back( sc_Field() );

  record.back().setMnemonic( "SADR" );

  record.back().setName( "Spatial Address" );


  double x, y;

  if ( getSADR( x, y ) )
  {
    sb_Utils::add_subfield( record.back(),"X", x );
    sb_Utils::add_subfield( record.back(),"Y", y );
  }
  else
  {
    return false;
  }


  // For each LYID field, add a LYID field to the record.

  for ( sb_ForeignIDs::const_iterator lyid_itr = imp_->lyids_.begin();
        lyid_itr != imp_->lyids_.end();
        lyid_itr++ )
    {

      record.push_back( sc_Field() );

      record.back().setMnemonic( "LYID" );

      record.back().setName( "Layer Id" );

      sb_Utils::add_subfield( record.back(),"MODN", (*lyid_itr).moduleName() );
      sb_Utils::add_subfield( record.back(),"RCID", (*lyid_itr).recordID() );
    }


  // For each RATP field, add a RAPT field to the record.

  for ( sb_ForeignIDs::const_iterator ratp_itr = imp_->ratps_.begin();
        ratp_itr != imp_->ratps_.end();
        ratp_itr++ )
    {
      record.push_back( sc_Field() );

      record.back().setMnemonic( "RATP" );

      record.back().setName( "Raster Attribute Id" );

      sb_Utils::add_subfield( record.back(),"MODN", (*ratp_itr).moduleName() );
      sb_Utils::add_subfield( record.back(),"RCID", (*ratp_itr).recordID() );
    }

  return true;


} // Rsdf::getRecord




bool
sb_Rsdf::setObjectRepresentation( string const& val )
{
  imp_->ObjectRepresentation_ = val;

  return true;
} // sb_Rsdf::setObjectRepresentation


bool
sb_Rsdf::setCellSequencingCode( string const& val )
{
  imp_->CellSequencingCode_ = val;

  return true;
} // sb_Rsdf::setCellSequencingCode


bool
sb_Rsdf::setCompression( string const& val )
{
  imp_->Compression_ = val;

  return true;
} // sb_Rsdf::setCompression


bool
sb_Rsdf::setEncodingMethod( string const& val )
{
  imp_->CodingMethod_ = val;

  return true;
} // sb_Rsdf::setEncodingMethod


bool
sb_Rsdf::setAcquisitionDeviceMethod( string const& val )
{
  imp_->AcquisitionDeviceMethod_ = val;

  return true;
} // sb_Rsdf::setAcquisitionDeviceMethod


bool
sb_Rsdf::setAcquisitionDate( string const& val )
{
  imp_->AcquisitionDate_ = val;

  return true;
} // sb_Rsdf::setAcquisitionDate


bool
sb_Rsdf::setComments( string const& val )
{
  imp_->Comments_ = val;

  return true;
} // sb_Rsdf::setComments


bool
sb_Rsdf::setDefaultImplementation( string const& val )
{
  imp_->DefaultImplementation_ = val;

  return true;
} // sb_Rsdf::setDefaultImplementation


bool
sb_Rsdf::setRowExtant( long val )
{
  imp_->RowExtent_ = val;

  return true;
} // sb_Rsdf::setRowExtant


bool
sb_Rsdf::setColumnExtent( long val )
{
  imp_->ColumnExtent_ = val;

  return true;
} // sb_Rsdf::setColumnExtent


// Phy - cut out unneeded stuff
//bool
//sb_Rsdf::setPlaneExtent( long val )
//{
//  imp_->PlaneExtent_ = val;
//
//  return true;
//} // sb_Rsdf::setPlaneExtent


bool
sb_Rsdf::setScanOrigin( string const& val )
{
  imp_->ScanOrigin_ = val;

  return true;
} // sb_Rsdf::setScanOrigin


bool
sb_Rsdf::setTesseralIndexing( string const& val )
{
  imp_->TesseralIndexing_ = val;

  return true;
} // sb_Rsdf::setTesseralIndexing


bool
sb_Rsdf::setScanPattern( string const& val )
{
  imp_->ScanPattern_ = val;

  return true;
} // sb_Rsdf::setScanPattern

// Phy - cut out unneeded stuff
//bool
//sb_Rsdf::setTesseralIndexFormat( string const& val )
//{
//  imp_->TesseralIndexFormat_ = val;
//
//  return true;
//} // sb_Rsdf::setTesseralIndexFormat
//
//
//bool
//sb_Rsdf::setTesseralIndexingDescription( string const& val )
//{
//  imp_->TesseralIndexingDescription_ = val;
//
//  return true;
//} // sb_Rsdf::setTesseralIndexingDescription


bool
sb_Rsdf::setNumberLinesAlternation( long val )
{
  imp_->NumberLinesAlternation_ = val;

  return true;
} // sb_Rsdf::setNumberLinesAlternation


bool
sb_Rsdf::setFirstScanDirection( string const& val )
{
  imp_->FirstScanDirection_ = val;

  return true;
} // sb_Rsdf::setFirstScanDirection


bool
sb_Rsdf::setAspectRation( double val )
{
  imp_->AspectRatio_ = val;

  return true;
} // sb_Rsdf::setAspectRation


bool
sb_Rsdf::setNumberLayers( long val )
{
  imp_->NumberLayers_ = val;

  return true;
} // sb_Rsdf::setNumberLayers


bool
sb_Rsdf::setSpatialAddress( double x, double y )
{
  imp_->SADR__X = x;
  imp_->SADR__Y = y;

  return true;
} // sb_Rsdf::setSpatialAddress


// Phy - Add set for field ISID
bool
sb_Rsdf::setInternalSpatialId( sb_ForeignID const & fid )
{
  imp_->isid_ = fid;
  return true;
}


bool
sb_Rsdf::setLayerId( sb_ForeignIDs const& fids )
{
  imp_->lyids_ = fids;
  return true;
}

// Phy - ??? Add set for field RATP
bool
sb_Rsdf::setRasterAttributeId( sb_ForeignIDs const& fids )
{
  imp_->ratps_ = fids;
  return true;
}


bool
sb_Rsdf::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Rsdf::setRecord




void
sb_Rsdf::unDefineObjectRepresentation( )
{
  imp_->ObjectRepresentation_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineObjectRepresentation


void
sb_Rsdf::unDefineCellSequencingCode( )
{
  imp_->CellSequencingCode_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineCellSequencingCode


void
sb_Rsdf::unDefineCompression( )
{
   imp_->Compression_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineCompression


void
sb_Rsdf::unDefineEncodingMethod( )
{
   imp_->CodingMethod_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineEncodingMethod


void
sb_Rsdf::unDefineAcquisitionDeviceMethod( )
{
  imp_->AcquisitionDeviceMethod_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineAcquisitionDeviceMethod


void
sb_Rsdf::unDefineAcquisitionDate( )
{
  imp_->AcquisitionDate_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineAcquisitionDate


void
sb_Rsdf::unDefineComments( )
{
  imp_->Comments_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineComments


void
sb_Rsdf::unDefineDefaultImplementation( )
{
  imp_->DefaultImplementation_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineDefaultImplementation


void
sb_Rsdf::unDefineRowExtant( )
{
  imp_->RowExtent_ = UNVALUED_LONG; // Phy - fixed spelling
} // sb_Rsdf::unDefineRowExtant


void
sb_Rsdf::unDefineColumnExtent( )
{
  imp_->ColumnExtent_ = UNVALUED_LONG;
} // sb_Rsdf::unDefineColumnExtent

// Phy - unneeded
//void
//sb_Rsdf::unDefinePlaneExtent( )
//{
//  imp_->PlaneExtent_ = UNVALUED_LONG;
//} // sb_Rsdf::unDefinePlaneExtent


void
sb_Rsdf::unDefineScanOrigin( )
{
  imp_->ScanOrigin_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineScanOrigin


void
sb_Rsdf::unDefineTesseralIndexing( )
{
  imp_->TesseralIndexing_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineTesseralIndexing


void
sb_Rsdf::unDefineScanPattern( )
{
  imp_->ScanPattern_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineScanPattern

// Phy - unneeded
//void
//sb_Rsdf::unDefineTesseralIndexFormat( )
//{
//  imp_->TesseralIndexFormat_ = UNVALUED_STRING;
//} // sb_Rsdf::unDefineTesseralIndexFormat
//
//
//void
//sb_Rsdf::unDefineTesseralIndexingDescription( )
//{
//  imp_->TesseralIndexingDescription_ = UNVALUED_STRING;
//} // sb_Rsdf::unDefineTesseralIndexingDescription


void
sb_Rsdf::unDefineNumberLinesAlternation( )
{
  imp_->NumberLinesAlternation_ = UNVALUED_LONG;
} // sb_Rsdf::unDefineNumberLinesAlternation


void
sb_Rsdf::unDefineFirstScanDirection( )
{
  imp_->FirstScanDirection_ = UNVALUED_STRING;
} // sb_Rsdf::unDefineFirstScanDirection


void
sb_Rsdf::unDefineAspectRation( )
{
  imp_->AspectRatio_ = UNVALUED_DOUBLE;  // Phy - fixed spelling
} // sb_Rsdf::unDefineAspectRation


void
sb_Rsdf::unDefineNumberLayers( )
{
  imp_->NumberLayers_ = UNVALUED_LONG;
} // sb_Rsdf::unDefineNumberLayers


void
sb_Rsdf::unDefineSpatialAddress()
{
  imp_->SADR__X = UNVALUED_DOUBLE;
  imp_->SADR__Y = UNVALUED_DOUBLE;
} // sb_RsDF::unDefineSpatialAddress


// Phy - Field ISID
void
sb_Rsdf::unDefineInternalSpatialId()
{
  imp_->isid_.moduleName( "" ); // reset to null state
  imp_->isid_.recordID( 0 );
}

// Phy - ??? Field LYID
void
sb_Rsdf::unDefineLayerIds()
{
  imp_->lyids_.clear();
}

// Phy - ??? Field RATP
void
sb_Rsdf::unDefineRasterAttributeIds()
{
  imp_->ratps_.clear();
}

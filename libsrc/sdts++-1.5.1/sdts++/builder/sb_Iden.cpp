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

// $Id: sb_Iden.cpp,v 1.7 2002/11/24 22:07:42 mcoletti Exp $


#include <iostream>


#include <limits.h>

#include <sdts++/builder/sb_Iden.h>


#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif


#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif


#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif


using namespace std;


static const char* iden_ = "$Id: sb_Iden.cpp,v 1.7 2002/11/24 22:07:42 mcoletti Exp $";


// Strings and integers are initialized with these values; they're used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static sio_8211Schema iden__schema; // iden module schema, which will be
                                // defined by build_iden_schema

static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h



sio_8211Schema&
sb_Iden::schema_()
{
   return iden__schema;
} // sb_Iden::schema_




//
// builds a schema suitable for writing an SDTS IDEN module
//
void
sb_Iden::buildSpecificSchema_(  )
{



  // IDENIFICATION field

  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "IDENIFICATION" );
  field_format.setTag( "IDEN" );

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

  field_format.back().setLabel( "STID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "STVS" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DOCU" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PRID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PRVS" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PDOC" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "TITL" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DAID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DAST" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MPDT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DCDT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SCAL" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_I );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );
   


  // CONFORMANCE field

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::vector );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "CONFORMANCE" );
  schema_().back().setTag( "CONF" );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "FFYN" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "VGYN" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "GTYN" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "RCYN" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "EXSP" );
  schema_().back().back().setType( sio_8211SubfieldFormat::I );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_I );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "FTLV" );
  schema_().back().back().setType( sio_8211SubfieldFormat::I );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_I );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "CDLV" );
  schema_().back().back().setType( sio_8211SubfieldFormat::I );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_I );

  schema_().back().push_back( sio_8211SubfieldFormat() );

  schema_().back().back().setLabel( "NGDM" );
  schema_().back().back().setType( sio_8211SubfieldFormat::A );
  schema_().back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema_().back().back().setConverter( &converter_A );


  // ATTRIBUTE ID field

  schema_().push_back( sio_8211FieldFormat() );

  schema_().back().setDataStructCode( sio_8211FieldFormat::array );
  schema_().back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema_().back().setName( "ATTRIBUTE ID" );
  schema_().back().setTag( "ATID" );

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

} // build_iden_schema





struct sb_Iden_Imp
{
  string standardIden;
  string standardVer;
  string standardDocRef;
  string profileIden;
  string profileVer;
  string profileDocRef;
  string title;
  string dataID;
  string dataStruct;
  string mapDate;
  string dataSetCreationDate;
  long   scale;
  string comment;

  string composites;
  string vectorGeom;
  string vectorTopol;
  string raster;
  long   externSpatRef;
  long   featuresLevel;
  long   codingLevel;
  string nongeoDimension;


#ifdef NOT_IMPLEMENTED
  sb_ForeignID attrID;          // XXX need valued indicator here, too
#endif

  bool   bad;                   // true if in bad state

  sb_Iden_Imp() : 
    bad( false ),
    standardIden( UNVALUED_STRING ),
    standardVer( UNVALUED_STRING ),
    standardDocRef( UNVALUED_STRING ),
    profileIden( UNVALUED_STRING ),
    profileVer( UNVALUED_STRING ),
    profileDocRef( UNVALUED_STRING ),
    title( UNVALUED_STRING ),
    dataID( UNVALUED_STRING ),
    dataStruct( UNVALUED_STRING ),
    mapDate( UNVALUED_STRING ),
    dataSetCreationDate( UNVALUED_STRING ),
    scale( UNVALUED_LONG ),
    comment( UNVALUED_STRING ),
    composites( UNVALUED_STRING ),
    vectorGeom( UNVALUED_STRING ),
    vectorTopol( UNVALUED_STRING ),
    raster( UNVALUED_STRING ),
    externSpatRef( UNVALUED_LONG ),
    featuresLevel( UNVALUED_LONG ),
    codingLevel( UNVALUED_LONG ),
    nongeoDimension( UNVALUED_STRING )
    {}

}; // struct sb_Iden_Imp



sb_Iden::sb_Iden() : imp_( new sb_Iden_Imp )
{
  setMnemonic( "IDEN" );        // set reasonable module mnemonic and
  setID( 0 );                   // record id defaults
}



sb_Iden::~sb_Iden()
{
  delete imp_;
}


// populate an iden object from the given sc_Record that (hopefully)
// contains an IDEN module record
static
bool
ingest_record_( sb_Iden & iden, 
                sb_Iden_Imp & iden_imp, 
                sc_Record const & record )
{

   // Make sure we have a record from a Idenification module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record, "IDEN", curfield) )
      {
#ifdef SDTSXX_DEBUG
         cerr << "sb_Iden::sb_Iden(sc_Record const&): Not a IDEN record.";
         cerr << endl;
#endif
         return false;
      }

   // We have a primary field from a Idenification module. Start picking 
   // it apart.

   sc_SubfieldCntr::const_iterator cursubfield;

   // MODN
   if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
     {
       string tmp;
      
       cursubfield->getA( tmp );
       iden.setMnemonic( tmp );
     }
      

   // RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
     {
       long tmp;
       cursubfield->getI( tmp );
       iden.setID( tmp );
     }

   // STID
   if (sb_Utils::getSubfieldByMnem(*curfield,"STID",cursubfield))
      cursubfield->getA(iden_imp.standardIden);

   // STVS
   if (sb_Utils::getSubfieldByMnem(*curfield,"STVS",cursubfield))
      cursubfield->getA(iden_imp.standardVer);

   // DOCU
   if (sb_Utils::getSubfieldByMnem(*curfield,"DOCU",cursubfield))
      cursubfield->getA(iden_imp.standardDocRef);

   // PRID
   if (sb_Utils::getSubfieldByMnem(*curfield,"PRID",cursubfield))
      cursubfield->getA(iden_imp.profileIden);
 
   // PRVS
   if (sb_Utils::getSubfieldByMnem(*curfield,"PRVS",cursubfield))
      cursubfield->getA(iden_imp.profileVer);

   // PDOC
   if (sb_Utils::getSubfieldByMnem(*curfield,"PDOC",cursubfield))
      cursubfield->getA(iden_imp.profileDocRef);

   // TITL
   if (sb_Utils::getSubfieldByMnem(*curfield,"TITL",cursubfield))
      cursubfield->getA(iden_imp.title);

   // DAID
   if (sb_Utils::getSubfieldByMnem(*curfield,"DAID",cursubfield))
      cursubfield->getA(iden_imp.dataID);

   // DAST
   if (sb_Utils::getSubfieldByMnem(*curfield,"DAST",cursubfield))
      cursubfield->getA(iden_imp.dataStruct);

   // MPDT
   if (sb_Utils::getSubfieldByMnem(*curfield,"MPDT",cursubfield))
      cursubfield->getA(iden_imp.mapDate);

   // DCDT
   if (sb_Utils::getSubfieldByMnem(*curfield,"DCDT",cursubfield))
      cursubfield->getA(iden_imp.dataSetCreationDate);

   // SCAL
   if (sb_Utils::getSubfieldByMnem(*curfield,"SCAL",cursubfield))
      cursubfield->getI(iden_imp.scale);

   // COMT
   if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
      cursubfield->getA(iden_imp.comment);

   // Secondary Fields

   // Conformance (CONF) Field
   if (!sb_Utils::getFieldByMnem(record,"CONF",curfield))
      {
#ifdef SDTSXX_DEBUG
         cerr << "sb_Iden::sb_Iden(sc_Record const&): Not a Conformance "
              << "field to be found.";
         cerr << endl;
#endif
         return false;
      }

   // We have a secondary field from a Idenification module. Start picking 
   // it apart.

   // FFYN 
   if (sb_Utils::getSubfieldByMnem(*curfield,"FFYN",cursubfield))
      cursubfield->getA(iden_imp.composites);

   // VGYN
   if (sb_Utils::getSubfieldByMnem(*curfield,"VGYN",cursubfield))
      cursubfield->getA(iden_imp.vectorGeom);

   // GTYN
   if (sb_Utils::getSubfieldByMnem(*curfield,"GTYN",cursubfield))
      cursubfield->getA(iden_imp.vectorTopol);

   // RCYN
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCYN",cursubfield))
      cursubfield->getA(iden_imp.raster);

   // EXSP
   if (sb_Utils::getSubfieldByMnem(*curfield,"EXSP",cursubfield))
      cursubfield->getI(iden_imp.externSpatRef);

   // FTLV
   if (sb_Utils::getSubfieldByMnem(*curfield,"FTLV",cursubfield))
      cursubfield->getI(iden_imp.featuresLevel);

   // CDLV - Coding level (I)
   if (sb_Utils::getSubfieldByMnem(*curfield,"CDLV",cursubfield))
      cursubfield->getI(iden_imp.codingLevel);

   // NGDM = nongeospatila deminsions (A)
   if (sb_Utils::getSubfieldByMnem(*curfield,"NGDM",cursubfield))
      cursubfield->getA(iden_imp.nongeoDimension);

#ifdef NOT_IMPLEMENTED
   // Attribute ID (ATID) Field
   if (sb_Utils::getFieldByMnem(record,"ATID",curfield))
      iden_imp.attrID = sb_ForeignID(*curfield);
#endif

   return true;

}; // ingest__record



sb_Iden::sb_Iden( sc_Record const & record )
  : imp_( new sb_Iden_Imp )
{
   // Build an sb_Iden from an sc_Record.
  if ( ! ingest_record_( *this, *imp_, record ) )
    {
      imp_->bad = true;
    }
} // sb_Iden ctor






bool
sb_Iden::getStandardIdentification( string& str ) const
{
  if (  imp_->standardIden == UNVALUED_STRING )
    return false;

  str =  imp_->standardIden;

  return true;
}


bool
sb_Iden::getStandardVersion( string& str ) const
{
  if (  imp_->standardVer == UNVALUED_STRING )
    return false;

  str =  imp_->standardVer;

  return true;
}


bool
sb_Iden::getStandardDocumentationReference( string& str ) const
{
  if (  imp_->standardDocRef == UNVALUED_STRING )
    return false;

  str =  imp_->standardDocRef;

  return true;
}


bool
sb_Iden::getProfileIdentification( string& str  ) const
{  
  if (  imp_->profileIden == UNVALUED_STRING )
    return false;

  str =  imp_->profileIden;

  return true;
}


bool
sb_Iden::getProfileVersion( string& str ) const
{
  if (  imp_->profileVer == UNVALUED_STRING )
    return false;

  str =  imp_->profileVer;

  return true;
}

 
bool
sb_Iden::getProfileDocumentationReference( string& str ) const
{
  if (  imp_->profileDocRef == UNVALUED_STRING )
    return false;

  str =  imp_->profileDocRef;

  return true;
}


bool
sb_Iden::getTitle( string& str ) const
{
  if (  imp_->title == UNVALUED_STRING )
    return false;

  str =  imp_->title;

  return true;
}


bool
sb_Iden::getDataID( string& str ) const
{
  if (  imp_->dataID == UNVALUED_STRING )
    return false;

  str =  imp_->dataID;

  return true;
}


bool
sb_Iden::getDataStructure( string& str ) const
{
  if (  imp_->dataStruct == UNVALUED_STRING )
    return false;

  str =  imp_->dataStruct;

  return true;
}


bool
sb_Iden::getMapDate( string& str ) const
{
  if (  imp_->mapDate == UNVALUED_STRING )
    return false;

  str =  imp_->mapDate;

  return true;
}


bool
sb_Iden::getDataSetCreationDate( string& str ) const
{
  if (  imp_->dataSetCreationDate == UNVALUED_STRING )
    return false;

  str =  imp_->dataSetCreationDate;

  return true;
}

 
bool
sb_Iden::getScale( long & num ) const
{
  if (  imp_->scale == UNVALUED_LONG )
    return false;

  num =  imp_->scale;

  return true;
}


bool
sb_Iden::getComment( string& str ) const
{
  if (  imp_->comment == UNVALUED_STRING )
    return false;

  str =  imp_->comment;

  return true;
}


bool
sb_Iden::getComposites( string& str ) const
{
  if (  imp_->composites == UNVALUED_STRING )
    return false;

  str =  imp_->composites;

  return true;
}


bool
sb_Iden::getVectorGeometry( string& str ) const
{
  if (  imp_->vectorGeom == UNVALUED_STRING )
    return false;

  str =  imp_->vectorGeom;

  return true;
}


bool
sb_Iden::getVectorTopology( string& str ) const
{
  if (  imp_->vectorTopol == UNVALUED_STRING )
    return false;

  str =  imp_->vectorTopol;

  return true;
}

 
bool
sb_Iden::getRaster( string& str ) const
{
  if (  imp_->raster == UNVALUED_STRING )
    return false;

  str =  imp_->raster;

  return true;
}


bool
sb_Iden::getExternalSpatialReference( long& num ) const
{
  if (  imp_->externSpatRef == UNVALUED_LONG )
    return false;

  num =  imp_->externSpatRef;

  return true;
}


bool
sb_Iden::getFeaturesLevel( long & num ) const
{
  if (  imp_->featuresLevel == UNVALUED_LONG )
    return false;

  num =  imp_->featuresLevel;

  return true;
}


bool
sb_Iden::getCodingLevel( long & num ) const
{
  if (  imp_->codingLevel == UNVALUED_LONG )
    return false;

  num =  imp_->codingLevel;

  return true;
}
      


bool
sb_Iden::getNonGeoSpatialDimensions( string& str ) const
{
  if (  imp_->nongeoDimension == UNVALUED_STRING )
    return false;

  str =  imp_->nongeoDimension;

  return true;
}
      

#ifdef NOT_IMPLEMENTED
bool
sb_Iden::getAttributeID( sb_ForeignID& fid ) const
{
  fid = imp_->attrID;           // XXX need to check for unset state
  return true;
}
#endif





bool
sb_Iden::getRecord( sc_Record& record ) const
{

  record.clear();               // start with a clean slate

  // IDEN field

  record.push_back( sc_Field() );

  record.back().setMnemonic( "IDEN" );

  string tmp_str;
  long   tmp_long;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  // Add each field and subfield to the record.  Bail if a mandatory
  // subfield hasn't been set.

  if ( ! getStandardIdentification( tmp_str ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "STID", tmp_str );

  if ( ! getStandardVersion( tmp_str ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "STVS", tmp_str );


  if ( getDOCU( tmp_str ) )     // DOCU optional, so only add if set
    {
      sb_Utils::add_subfield( record.back(), "DOCU", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "DOCU", sc_Subfield::is_A );
    }


  if ( ! getPRID( tmp_str ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "PRID", tmp_str );

  if ( ! getPRVS( tmp_str ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "PRVS", tmp_str );


  if ( getPDOC( tmp_str ) ) // optional subfield; add if have value
    {
      sb_Utils::add_subfield( record.back(), "PDOC", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "PDOC", sc_Subfield::is_A );
    }


  if ( ! getTitle( tmp_str ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "TITL", tmp_str );

  if ( getDataID( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "DAID", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "DAID", sc_Subfield::is_A );
    }



                                // The next five subfields are
                                // optional, so they'll only be set if
                                // they've got values.

  if ( getDataStructure( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "DAST", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "DAST", sc_Subfield::is_A );
    }


  if ( getMapDate( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "MPDT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "MPDT", sc_Subfield::is_A );
    }


  if ( getDataSetCreationDate( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "DCDT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "DCDT", sc_Subfield::is_A );
    }


  if ( getScale( tmp_long ) )
    {
      sb_Utils::add_subfield( record.back(), "SCAL", tmp_long );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "SCAL", sc_Subfield::is_I );
    }

  if ( getComment( tmp_str ) )
    {
      sb_Utils::add_subfield( record.back(), "COMT", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
    }


  // Conformance field



  record.push_back( sc_Field() );

  record.back().setMnemonic( "CONF" );
   

  if ( ! ( getComposites( tmp_str ) &&
           sb_Utils::valid_domain( tmp_str, "YN" ) ) )
    {
      return false;
    }

  sb_Utils::add_subfield( record.back(), "FFYN", tmp_str );

  if ( ! ( getVGYN( tmp_str ) &&
           sb_Utils::valid_domain( tmp_str, "YN" ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "VGYN", tmp_str );

  if ( ! ( getGTYN( tmp_str ) && 
           sb_Utils::valid_domain( tmp_str, "YN" ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "GTYN", tmp_str );

  if ( ! ( getRaster( tmp_str ) && sb_Utils::valid_domain( tmp_str, "YN" ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "RCYN", tmp_str );


  set<long> EXSP_domain;
  EXSP_domain.insert( 1 );
  EXSP_domain.insert( 2 );
  EXSP_domain.insert( 3 );

  // set.insert( NULL );  XXX What is the NULL value?

  if ( ! ( getEXSP( tmp_long ) &&
           sb_Utils::valid_domain( tmp_long, EXSP_domain ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "EXSP", tmp_long );

  set<long> FTLV_domain;
  FTLV_domain.insert( 1 );
  FTLV_domain.insert( 2 );
  FTLV_domain.insert( 3 );
  FTLV_domain.insert( 4 );

  // set.insert( NULL );  XXX What is the NULL value?


  if ( ! ( getFeaturesLevel( tmp_long ) &&
           sb_Utils::valid_domain( tmp_long, FTLV_domain ) ) )
    {
      return false;
    }
  sb_Utils::add_subfield( record.back(), "FTLV", tmp_long );


  if ( getCodingLevel( tmp_long ) ) // this subfield is optional
    {
      set<long> CDLV_domain;
      CDLV_domain.insert( 0 );
      CDLV_domain.insert( 1 );
      CDLV_domain.insert( 2 );
      
      if ( ! sb_Utils::valid_domain( tmp_long, CDLV_domain ) )
        {
          return false;
        }

      sb_Utils::add_subfield( record.back(), "CDLV", tmp_long );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "CDLV", sc_Subfield::is_I );
    }

  if ( getNGDM( tmp_str ) )     // this subfield is optional
    {
      if ( ! sb_Utils::valid_domain( tmp_str, "YN" ) )
        {
          return false;
        }

      sb_Utils::add_subfield( record.back(), "NGDM", tmp_str );
    }
  else
    {
      sb_Utils::add_empty_subfield( record.back(), "NGDM", sc_Subfield::is_A );
    }

  return true;

} // sb_Iden::getRecord




void
sb_Iden::setStandardIdentification( string const & str )
{
  imp_->standardIden = str;
}


void
sb_Iden::setStandardVersion( string const & str )
{
  imp_->standardVer = str;
}


void
sb_Iden::setStandardDocumentationReference( string const & str )
{
  imp_->standardDocRef = str;
}



void
sb_Iden::setProfileIdentification( string const & str )
{
  imp_->profileIden = str;
}


void
sb_Iden::setProfileVersion( string const & str )
{
  imp_->profileVer = str;
}


void
sb_Iden::setProfileDocumentationReference( string const & str )
{
  imp_->profileDocRef = str;
}


void
sb_Iden::setTitle( string const & str )
{
  imp_->title = str;
}


void
sb_Iden::setDataID( string const & str )
{
  imp_->dataID = str;
}


void
sb_Iden::setDataStructure( string const & str )
{
  imp_->dataStruct = str;
}


void
sb_Iden::setMapDate( string const & str )
{
  imp_->mapDate = str;
}


void
sb_Iden::setDataSetCreationDate( string const & str )
{
  imp_->dataSetCreationDate = str;
}


void
sb_Iden::setScale( long val )
{
  imp_->scale = val;
}


void
sb_Iden::setComment( string const & str )
{
  imp_->comment = str;
}



void
sb_Iden::setComposites( string const & str )
{
  imp_->composites = str;
}


void
sb_Iden::setVectorGeometry( string const & str )
{
  imp_->vectorGeom = str;
}


void
sb_Iden::setVectorTopology( string const & str )
{
  imp_->vectorTopol = str;
}


void
sb_Iden::setRaster( string const & str )
{
  imp_->raster = str;
}


void
sb_Iden::setExternalSpatialReference( long val )
{
  imp_->externSpatRef = val;
}


void
sb_Iden::setFeaturesLevel( long val )
{
  imp_->featuresLevel = val;
}


void
sb_Iden::setCodingLevel( long val )
{
  imp_->codingLevel = val;
}


void
sb_Iden::setNonGeoSpatialDimensions( string const & str )
{
  imp_->nongeoDimension = str;
}


      
#ifdef NOT_IMPLEMENTED
void
sb_Iden::setAttributeID( sb_ForeignID const & fid )
{
  imp_->attrID = fid;
}
#endif



bool
sb_Iden::setRecord( sc_Record const& record )
{
   // Build an sb_Iden from an sc_Record.
  if ( ! ingest_record_( *this, *imp_, record ) )
    {
      imp_->bad = true;
      return false;
    }
  return true;
} // sb_Iden::setRecord

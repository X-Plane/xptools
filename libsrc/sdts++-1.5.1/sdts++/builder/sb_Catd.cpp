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
// $Id: sb_Catd.cpp,v 1.16 2002/11/24 22:07:42 mcoletti Exp $
//

#include <sdts++/builder/sb_Catd.h>


#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

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

#include <sdts++/io/sio_Reader.h>



using namespace std;



static const char* ident_ = 
  "$Id: sb_Catd.cpp,v 1.16 2002/11/24 22:07:42 mcoletti Exp $";



// Strings and integers are initialized with these values; they are used
// to indicate whether a given module value has been assigned a value or not.

// (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)

static const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

static const long    UNVALUED_LONG   = INT_MIN;

static const double  UNVALUED_DOUBLE = DBL_MAX;

struct sb_Catd_Imp
{
  string   Name_;
  string   Type_;
  string   Volume_;
  string   File_;
  string   Record_;
  string   External_;
  string   ModuleVersion_;
  string   Comment_;


  sb_Catd_Imp()
    : 
    Name_( UNVALUED_STRING ),
    Type_( UNVALUED_STRING ),
    Volume_( UNVALUED_STRING ),
    File_( UNVALUED_STRING ),
    Record_( UNVALUED_STRING ),
    External_( UNVALUED_STRING ),
    ModuleVersion_( UNVALUED_STRING ),
    Comment_( UNVALUED_STRING )
  {}

};


sb_Catd::sb_Catd()
  : imp_( new sb_Catd_Imp() )
{
  setMnemonic( "CATD" );
  setID( 1 );
} // Catd ctor



sb_Catd::sb_Catd( sb_Catd const & rhs )
  : imp_( new sb_Catd_Imp( *rhs.imp_ ) )
{
  setMnemonic( "CATD" );
  setID( 1 );
} // Catd ctor



sb_Catd const & 
sb_Catd::operator=( sb_Catd const & rhs )
{
   if ( &rhs == this ) return *this;

   *imp_ = *rhs.imp_;

   setMnemonic( "CATD" );
   setID( 1 );

   return *this;
} // sb_Catd::operator=




sb_Catd::~sb_Catd()
{
  delete imp_;
} // Catd dtor




static sio_8211Converter_I converter_I; // XXX should define these in
static sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
static sio_8211Converter_R converter_R;
static sio_8211Converter_C converter_C;

static sio_8211Schema schema_; // module specific schema


sio_8211Schema&
sb_Catd::schema_()
{
   return ::schema_;
} // sb_Catd::schema_()



void
sb_Catd::buildSpecificSchema_(  )
{
  schema_().push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema_().back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "Catalog/Directory" );
  field_format.setTag( "CATD" );


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

  field_format.back().setLabel( "NAME" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "TYPE" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


//  field_format.push_back( sio_8211SubfieldFormat() );

//  field_format.back().setLabel( "VOLM" );
//  field_format.back().setType( sio_8211SubfieldFormat::A );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "FILE" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


#ifdef NOT_SUPPORTED
  field_format.push_back( sio_8211SubfieldFormat() );
  field_format.back().setLabel( "RECD" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );
#endif

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "EXTR" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MVER" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( &converter_A );


//  field_format.push_back( sio_8211SubfieldFormat() );
//
//  field_format.back().setLabel( "COMT" );
//  field_format.back().setType( sio_8211SubfieldFormat::A );
//  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
//  field_format.back().setConverter( &converter_A );


} // sb_Catd::buildSpecificSchema()




static
bool
ingest_record_( sb_Catd& catd, sb_Catd_Imp &catd_imp, sc_Record const& record )
{

  // Make sure we have a record from an catalog module.

  sc_FieldCntr::const_iterator curfield;

  if ( ! sb_Utils::getFieldByMnem( record,"CATD",curfield) )
  {
#ifdef SDTSXX_DEBUG
    cerr << "sb_Catd::sb_Catd(sc_Record const&): "
         << "Not an catalog/directory record.";
    cerr << endl;
#endif
    return false;
  }


  // We have a primary field from a  module. Start picking it apart.

  sc_SubfieldCntr::const_iterator cursubfield;

  string tmp_str;
  long   tmp_int;


  // MODN
  if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
  {
    cursubfield->getA( tmp_str );
    catd.setMnemonic( tmp_str );
  }


  // RCID
  if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
  {
    cursubfield->getI( tmp_int );
    catd.setID( tmp_int );
  }


  // NAME
  if (sb_Utils::getSubfieldByMnem(*curfield,"NAME",cursubfield))
  {
    cursubfield->getA( catd_imp.Name_);
  }


  // TYPE
  if (sb_Utils::getSubfieldByMnem(*curfield,"TYPE",cursubfield))
  {
    cursubfield->getA( catd_imp.Type_);
  }


  // VOLM
//  if (sb_Utils::getSubfieldByMnem(*curfield,"VOLM",cursubfield))
//  {
//    cursubfield->getA( catd_imp.Volume_);
//  }
//  else
//  {
//    return false;
//  }


  // FILE
  if (sb_Utils::getSubfieldByMnem(*curfield,"FILE",cursubfield))
  {
    cursubfield->getA( catd_imp.File_);
  }


#ifdef NOT_SUPPORTED
 // RECD
  if (sb_Utils::getSubfieldByMnem(*curfield,"RECD",cursubfield))
  {
     cursubfield->getA( catd_imp.Record_);
  }
  else
  {
     return false;
  }
#endif


  // EXTR
  if (sb_Utils::getSubfieldByMnem(*curfield,"EXTR",cursubfield))
  {
    cursubfield->getA( catd_imp.External_);
  }


  // MVER
  if (sb_Utils::getSubfieldByMnem(*curfield,"MVER",cursubfield))
  {
    cursubfield->getA( catd_imp.ModuleVersion_);
  }


  // COMT
//  if (sb_Utils::getSubfieldByMnem(*curfield,"COMT",cursubfield))
//  {
//    cursubfield->getA( catd_imp.Comment_);
//  }
//  else
//  {
//    return false;
//  }


  return true;


} // ingest_record_




bool
sb_Catd::getName( string& val ) const
{
  if ( imp_->Name_ == UNVALUED_STRING )
    return false;

  val = imp_->Name_;

  return true;
} // sb_Catd::getName


bool
sb_Catd::getType( string& val ) const
{
  if ( imp_->Type_ == UNVALUED_STRING )
    return false;

  val = imp_->Type_;

  return true;
} // sb_Catd::getType


bool
sb_Catd::getVolume( string& val ) const
{
  if ( imp_->Volume_ == UNVALUED_STRING )
    return false;

  val = imp_->Volume_;

  return true;
} // sb_Catd::getVolume


bool
sb_Catd::getFile( string& val ) const
{
  if ( imp_->File_ == UNVALUED_STRING )
    return false;

  val = imp_->File_;

  return true;
} // sb_Catd::getFile


#ifdef NOT_SUPPORTED
bool
sb_Catd::getRecord( string& val ) const
{
  if ( imp_->Record_ == UNVALUED_STRING )
    return false;

  val = imp_->Record_;

  return true;
} // sb_Catd::getRecord
#endif


bool
sb_Catd::getExternal( string& val ) const
{
  if ( imp_->External_ == UNVALUED_STRING )
    return false;

  val = imp_->External_;

  return true;
} // sb_Catd::getExternal


bool
sb_Catd::getModuleVersion( string& val ) const
{
  if ( imp_->ModuleVersion_ == UNVALUED_STRING )
    return false;

  val = imp_->ModuleVersion_;

  return true;
} // sb_Catd::getModuleVersion


bool
sb_Catd::getComment( string& val ) const
{
  if ( imp_->Comment_ == UNVALUED_STRING )
    return false;

  val = imp_->Comment_;

  return true;
} // sb_Catd::getComment





bool
sb_Catd::getRecord( sc_Record & record ) const
{
  record.clear();               // start with a clean slate

  // first field, which contains module name and record number

  record.push_back( sc_Field() );

  record.back().setMnemonic( "CATD" );

  record.back().setName( "Catalog/Directory" );

  string tmp_str;

  getMnemonic( tmp_str );
  sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
  sb_Utils::add_subfield( record.back(), "RCID", getID() );

  if ( getName( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"NAME", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "NAME", sc_Subfield::is_A );
  }


  if ( getType( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"TYPE", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "TYPE", sc_Subfield::is_A );
  }

/*
  if ( getVolume( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"VOLM", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "VOLM", sc_Subfield::is_A );
  }
*/

  if ( getFile( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"FILE", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "FILE", sc_Subfield::is_A );
  }


#ifdef NOT_SUPPORTED
  if ( getRecord( tmp_str ) )
  {
     sb_Utils::add_subfield( record.back(),"RECD", tmp_str );
  }
  else
  {
     sb_Utils::add_empty_subfield( record.back(), "RECD", sc_Subfield::is_A );
  }
#endif


  if ( getExternal( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"EXTR", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "EXTR", sc_Subfield::is_A );
  }


  if ( getModuleVersion( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"MVER", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "MVER", sc_Subfield::is_A );
  }

/*
  if ( getComment( tmp_str ) )
  {
    sb_Utils::add_subfield( record.back(),"COMT", tmp_str );
  }
  else
  {
    sb_Utils::add_empty_subfield( record.back(), "COMT", sc_Subfield::is_A );
  }
*/

  return true;


} // Catd::getRecord




bool
sb_Catd::setName( string const& val )
{
  imp_->Name_ = val;

  return true;
} // sb_Catd::setName


bool
sb_Catd::setType( string const& val )
{
  imp_->Type_ = val;

  return true;
} // sb_Catd::setType


bool
sb_Catd::setVolume( string const& val )
{
  imp_->Volume_ = val;

  return true;
} // sb_Catd::setVolume


bool
sb_Catd::setFile( string const& val )
{
  imp_->File_ = val;

  return true;
} // sb_Catd::setFile


#ifdef NOT_SUPPORTED
bool
sb_Catd::setRecord( string const& val )
{
  imp_->Record_ = val;

  return true;
} // sb_Catd::setRecord
#endif


bool
sb_Catd::setExternal( string const& val )
{
  imp_->External_ = val;

  return true;
} // sb_Catd::setExternal


bool
sb_Catd::setModuleVersion( string const& val )
{
  imp_->ModuleVersion_ = val;

  return true;
} // sb_Catd::setModuleVersion


bool
sb_Catd::setComment( string const& val )
{
  imp_->Comment_ = val;

  return true;
} // sb_Catd::setComment


bool
sb_Catd::setRecord( sc_Record const& record )
{
  return ingest_record_( *this, *imp_, record );
} // sb_Catd::setRecord




void
sb_Catd::unDefineName( )
{
  imp_->Name_ = UNVALUED_STRING;
} // sb_Catd::unDefineName


void
sb_Catd::unDefineType( )
{
  imp_->Type_ = UNVALUED_STRING;
} // sb_Catd::unDefineType


void
sb_Catd::unDefineVolume( )
{
  imp_->Volume_ = UNVALUED_STRING;
} // sb_Catd::unDefineVolume


void
sb_Catd::unDefineFile( )
{
  imp_->File_ = UNVALUED_STRING;
} // sb_Catd::unDefineFile


#ifdef NOT_SUPPORTED
void
sb_Catd::unDefineRecord( )
{
  imp_->Record_ = UNVALUED_STRING;
} // sb_Catd::unDefineRecord
#endif


void
sb_Catd::unDefineExternal( )
{
  imp_->External_ = UNVALUED_STRING;
} // sb_Catd::unDefineExternal


void
sb_Catd::unDefineModuleVersion( )
{
  imp_->ModuleVersion_ = UNVALUED_STRING;
} // sb_Catd::unDefineModuleVersion


void
sb_Catd::unDefineComment( )
{
  imp_->Comment_ = UNVALUED_STRING;
} // sb_Catd::unDefineComment




//
// sb_Directory
//


struct sb_Directory::Imp
{
      // file name of the CATD module that we'll be doing look-ups
      // from
      string catd_filename;

      // associative container that maps SDTS module NAMES to sb_Catd
      // objects
      map<string,sb_Catd> directory;


      Imp() {}

      Imp( string const & catd_fn )
         : catd_filename( catd_fn )
      {}

      ~Imp() {}

      /// create this->directory from the CATD module found in catd_filename
      bool createDirectory();

} ; // struct sb_Directory::Imp



/// Set up an sb_Directory 
/**

   Open the CATD module referenced in directory_imp.catd_filename and
   read each record.  For each record, create an entry in
   directory_imp.directory mapping a sb_Catd record to the CATD::FILE.

 */
bool
sb_Directory::Imp::createDirectory( )
{
   // open the CATD module

   ifstream catd_file( catd_filename.c_str() );

   if ( ! catd_file )
   {                            // try giving us a valid file next time, ok?
      return false;
   }

   // start grinding through the CATD records

   sio_8211Reader reader( catd_file ); // CATD module reader

   sio_8211ForwardIterator i( reader ); // used to grind through CATD

   sc_Record record;            // current CATD record

   string catd_name;            // CATD::NAME values

   sb_Catd catd_module_record;


   while ( i )
   {
      i.get( record );

      if ( ! catd_module_record.setRecord( record ) )
      {
         return false;          // we have a non CATD module, so bail
      }

      if ( ! catd_module_record.getNAME( catd_name ) )
      {
         return false;          // if we can't get CATD::NAME, something's
                                // seriously wrong because ALL CATD modules
                                // have this subfield
      }

      directory[ catd_name ] = catd_module_record;

      ++i;

   } // grind to next CATD module record

   return true;

} // createDirectory


sb_Directory::~sb_Directory()
{
   if ( imp_ ) delete imp_;
} // sb_Directory ctor


sb_Directory::sb_Directory()
   : imp_( new sb_Directory::Imp )
{
} // sb_Directory ctor



sb_Directory::sb_Directory( string const & catd_filename )
   : imp_( new sb_Directory::Imp(catd_filename) )
{
   imp_->createDirectory( );
} // sb_Directory ctor



string const &
sb_Directory::catdFilename() const
{
   return imp_->catd_filename;
} // sb_Directory::catdFilename() 



bool
sb_Directory::catdFilename( string const & catd_fn ) 
{
   imp_->catd_filename = catd_fn;

   return imp_->createDirectory( );
} // sb_Directory::catdFilename() 



bool
sb_Directory::find( std::string const & module_name,
                    sb_Catd & module_info ) const
{
   if ( imp_->directory.empty() )
   {
      return false;             // we haven't read a CATD module yet
   }

   map<string,sb_Catd>::const_iterator catd_entry;

   catd_entry = imp_->directory.find( module_name );

   if ( catd_entry == imp_->directory.end() )
   {
      return false;             // NAME doesn't exist
   }

   module_info = catd_entry->second;

   return true;
} // sb_Directory::find

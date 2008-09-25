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

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <string>
#include <fstream>

#include <cstring>

#include <algorithm>
#include <functional>

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif

#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Catd.h>
#include <sdts++/builder/sb_Ddsh.h>

#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/io/sio_Reader.h>

#include "sysutils/fileutils.h"
#include "sysutils/stringutils.h"

using namespace std;


static const char* ident_ =
"$Id: sb_Utils.cpp,v 1.16 2002/03/27 23:47:46 mcoletti Exp $";


//
// Support Functions for sb_Utils
//


// Generic utility things.
//
// XXX You better believe__ this is temporary.

// This algorithm is used in find_if calls to locate fields
// or subfields that have a given mnemonic.

// Q.v., sc_Record:getRecordID() for an example of its use.

template< class T >
struct equalMnemonic : binary_function<T, const string, bool>
{
  bool operator()( const T& x, const string& y ) const
    { return x.getMnemonic() == y; }

};
// comparison class equalMnemonic

template< class T >
struct equalName : binary_function<T, const string, bool>
{
  bool operator()( const T& x, const string& y ) const
    { return x.getName() == y; }

};
// comparison class equalName

// This algorithm is used in find_if calls to locate modules with a
// given module type.

template< class T >
struct equalModuleType : binary_function<T, const string, bool>
{
  bool operator()( const T& x, const string& y ) const
    { return x.getModuleType() == y; }

};
// comparison class equalModuleType



/*
 * Implementation of the sb_Utils functions.
 */

bool
sb_Utils::getFieldByMnem(sc_Record const& rec,
                         string const& mnemonic,
                         sc_Record::const_iterator& thefield)
{
  thefield = find_if( rec.begin(), rec.end(),
                      bind2nd(equalMnemonic<sc_Field>(), mnemonic) );
  if ( thefield == rec.end() )
    return false;

  return true;
}




bool
sb_Utils::getSubfieldByMnem( sc_Field const &           field,
                             string const &             mnemonic,
                             sc_Field::const_iterator & thesubf )
{
  thesubf = find_if( field.begin(), field.end(),
                     bind2nd(equalMnemonic<sc_Subfield>(), mnemonic) );

  if ( thesubf == field.end() )
  { return false; }

  return true;

} // sb_Utils::getSubfieldByMnem




bool
sb_Utils::getSubfieldByName( sc_Field const& field,
                             string const& name,
                             sc_Field::const_iterator& thesubf )
{
  thesubf = find_if( field.begin(), field.end(),
                     bind2nd(equalName<sc_Subfield>(), name) );

  if ( thesubf == field.end() )
  { return false; }

  return true;

} // sb_Utils::getSubfieldByName



// Tries to convert a subfield into a double.
// If it succeeds returns True, else False.
// Place the convert value into the dataTo passed in parameter
bool
sb_Utils::getDoubleFromSubfield(sc_SubfieldCntr::const_iterator const& subf, //sc_Subfield const& subf,
                                double& dataOut )
{

  bool rc;
  unsigned long tempULong;
  long tempLong;
  sc_Subfield::SubfieldType sType = subf->getSubfieldType();

  switch(sType) {

    // Not supported at this time.
    // convert string class to double
  case(sc_Subfield::is_A):
  case(sc_Subfield::is_C):
    return false;
    break;

    // convert long to double
  case(sc_Subfield::is_I):
    rc = subf->getI( tempLong );
    dataOut = double(tempLong);
    break;
  case(sc_Subfield::is_BI8):
    rc = subf->getBI8( tempLong );
    dataOut = double(tempLong);
    break;
  case(sc_Subfield::is_BI16):
    rc = subf->getBI16( tempLong );
    dataOut = double(tempLong);
    break;
  case(sc_Subfield::is_BI24):
    rc = subf->getBI24( tempLong );
    dataOut = double(tempLong);
    break;
  case(sc_Subfield::is_BI32):
    rc = subf->getBI32( tempLong );
    dataOut = double(tempLong);
    break;

    // Convert unsigned long to double
  case(sc_Subfield::is_BUI8):
    rc = subf->getBUI8( tempULong );
    dataOut = double(tempULong );
    break;
  case(sc_Subfield::is_BUI16):
    rc = subf->getBUI16( tempULong );
    dataOut = double(tempULong );
    break;
  case(sc_Subfield::is_BUI24):
    rc = subf->getBUI24( tempULong );
    dataOut = double(tempULong );
    break;
  case(sc_Subfield::is_BUI32):
    rc = subf->getBUI32( tempULong );
    dataOut = double(tempULong );
    break;

    //convert double to double
  case(sc_Subfield::is_R):
    rc = subf->getR( dataOut );
    break;
  case(sc_Subfield::is_S):
    rc = subf->getS( dataOut );
    break;

    // non-implimented return false
  case(sc_Subfield::is_B):
  case(sc_Subfield::is_BUI):
  case(sc_Subfield::is_BFP32):
  case(sc_Subfield::is_BFP64):
    rc = false;
    break;

    // Shouldn't get here, except in an error situation
  default:
    rc = false;
  }

  return rc;


}
/* Numberic -> optionalSign Digits FrationalPart ExponentPart
 */
/*
 *
 *  <number>    ->  [ <sign> ] <digit> { <digit> } [ <deciaml> ] [ <exponent> ]
 *
 *  <sign>       ->  '+' | '-'
 *
 *  <decimal>   ->  '.' { <digit> }
 *
 *  <exponent>  ->  'E' [ <sign> ] <digit> { <digit> }
 *
 *  <digits>    ->  1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0
 *
 */

// bool String2double(sc_SubfieldCntr::const_iterator const& subf, double &dataOut );

inline
bool
String2double( sc_SubfieldCntr::const_iterator const& subf,
               double &dataOut )
{
  sc_Subfield::SubfieldType sType = subf->getSubfieldType();
  string tempString;
  bool rc;
  switch(sType){
  case(sc_Subfield::is_A):
    rc = subf->getA( tempString );
    break;
  case(sc_Subfield::is_C):
    rc = subf->getC( tempString );
    break;
  default:
    // Error!
    // Illegal subfield type passed to function.
    return false;
  }
  if ( rc == false ) // get on subfield failed pass it on.
    return false;
  // Verfiy string is only digits,
  // convert
  return true;
}



void
sb_Utils::add_field( sc_Record &         record,
                     std::string const & name,
                     std::string const & mnemonic )
{

   record.push_back( sc_Field() );

   record.back().setName( name );

   record.back().setMnemonic( mnemonic );

} // add_field



void
sb_Utils::add_foreignID( sc_Record & record,
                         sb_ForeignID const & frid )
{
   // first add the field
   sb_Utils::add_field( record, frid.name(), frid.mnemonic() );

   // now add the relevent subfields

   sb_Utils::add_subfield( record.back(),
                           "MODN",
                           frid.moduleName() );

   sb_Utils::add_subfield( record.back(),
                           "RCID",
                           frid.recordID() );

   // XXX add support for optional usage modifier

} // sb_Utils::add_foreignID( sc_Record & record,



/// XXX Should sc_Field have an addSubfield that does this instead?
void
sb_Utils::add_subfield( sc_Field& field,
                        string const& mnemonic,
                        string const& value )
{ field.push_back( sc_Subfield() );

 field.back().setMnemonic( mnemonic );
 field.back().setA( value );
} // add_subfield



void
sb_Utils::add_subfield( sc_Field& field, string const& mnemonic, int value )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );
  field.back().setI( (long) value );
} // add_subfield



void
sb_Utils::add_subfield( sc_Field& field, string const& mnemonic, long value )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );
  field.back().setI( value );
} // add_subfield



void
sb_Utils::add_subfield( sc_Field& field, string const& mnemonic, double value )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );
  field.back().setR( value );
} // add_subfield




void
sb_Utils::add_empty_subfield( sc_Field& field,
			       string const& mnemonic,
			       sc_Subfield::SubfieldType type )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );

				// XXX this switch statement could be
				// XXX eliminated if the setUnvalued()
				// XXX took a type parameter, or was
				// XXX defined for each subfield type
  switch ( type )
  {
  case sc_Subfield::is_A :
    field.back().setA( "" );
    break;
  case sc_Subfield::is_I :
    field.back().setI( (long) 0 );
    break;
  case sc_Subfield::is_R :
    field.back().setR( 0.0 );
    break;
  case sc_Subfield::is_S :
    field.back().setS( 0.0 );
    break;
  default :
    // non of the other fields can be variable, so this function is irrelevent
    // (or they're inherently bogus, like the 'C' type subfields
    break;
  }

  field.back().setUnvalued();	// explicitly state that it ain't got nothin'

} // add_subfield





bool
sb_Utils::valid_domain( string const & str, string const & values )
{
  if ( str.length() != 1 ) return false; // single character comparison only

  if ( strpbrk( str.c_str(), values.c_str() ) == NULL )
    return false;

  return true;
} // valid_domain




// XXX These two could probably be made a template function of some sort.

bool
sb_Utils::valid_domain( long val, set<long> const & domain_values )
{
  set<long> tmp_set;
  tmp_set.insert( val );
  return includes( domain_values.begin(), domain_values.end(),
                   tmp_set.begin(), tmp_set.end() );
} // valid_domain






bool
sb_Utils::valid_domain( string const& val, set<string> const & domain_values )
{
   for ( set<string>::const_iterator i = domain_values.begin();
         i != domain_values.end();
         i++ )
   {
      // I had to use std::lexicographical_compare because "val" was
      // getting set into a weird state in sb_Ddsh.cpp; NOTHING I did
      // seemed to correct the problem.  E.g., "ATPR" compared to
      // "ATPR" would indicate they WEREN'T equal.
      if ( lexicographical_compare( i->begin(), i->end(),
                                    val.begin(), val.end() ) )
      { return true; }
   }

   return false;

} // valid_domain



void
sb_Utils::find( sc_Module::const_iterator begin,
                sc_Module::const_iterator end,
                string const & field_name,
                sc_Subfield const & subfield,
                sc_Module & matches )
{
   sc_Record::const_iterator matched_field;
   sc_Field::const_iterator  matched_subfield;

   for ( sc_Module::const_iterator current_record = begin;
         current_record != end;
         ++current_record )
   {
      if ( getFieldByMnem( *current_record,
                           field_name,
                           matched_field ) )
      {
         if ( getSubfieldByMnem( *matched_field,
                                 subfield.mnemonic(),
                                 matched_subfield ) )
         {
            if ( *matched_subfield == subfield )
            {
               matches.push_back( *current_record );
            }
         }
      }
      else
      {
         // If the field doesn't exist in one record, it's not going
         // to magically appear in the rest of the module SDTS records
         // because these records are homogenous; so why waste time
         // grinding through the rest of them?  So, leave immediately
         // if the field isn't found in this module record.
         return;
      }
   }
} // sb_Utils::find




/// Returns true if ``type'' is one of the 8211 binary types; otherwise false.
/**
   It does so by the simplest form of checking: since ALL the binary
   types begin with a 'B' (or 'b'), then check that the first
   character is that; if so, then it's a binary type, otherwise it
   can't be.  Obviously this will miss malformed binary types strings;
   e.g., "BAZ".  But the sio_ConverterFactory will just return NULL if
   you try to use such a malformed string, so we needn't worry about
   being pedantic here.
 */
static
bool
isBinaryType_( string const & type )
{
   if ( type[0] == 'b' || type[0] == 'B' )
   {
      return true;
   }

   return false;
} // isBinaryType_



bool
sb_Utils::addConverter( sb_Iref const & iref,
                        sio_8211_converter_dictionary & dictionary )
{
   string hfmt;                 // a dataset's horizontal format description

   if ( ! iref.getHFMT( hfmt ) )
   {
      return false;
   }

   // if it's not a binary type, we don't have to worry about making a
   // special converter for it; since this isn't exactly an error,
   // we'll return ``true''.  (It's not an error because there are
   // many instances where the HFMT is non-binary for perfectly
   // legitimate datasets.)
   if ( ! isBinaryType_( hfmt ) )
   {
      return true;
   }

   // Ok, so now we've got the HFMT; we merely ask the factory to give
   // us an appropriate converter and we add that to the dictionary.
   // Done!

   sio_8211Converter * converter;

   if ( ( converter = sio_ConverterFactory::instance()->get( hfmt ) )  )
   {
      // all horizontal format subfield values use "X" and "Y"
      // mnemonics (I think.  At least from what I've seen so far.)

      dictionary[ "X" ] = converter;
      dictionary[ "Y" ] = converter;

      return true;
   }

   // We got a binary type but the factory was unable to properly
   // parse the binary type string, so it's more than likely
   // malformed, so complain.

   return false;

} // sb_Utils:: addConverter



bool
sb_Utils::addConverter( sb_Ddsh const & ddsh,
                        sio_8211_converter_dictionary & dictionary )
{
   string fmt;                 // a format description

   if ( ! ddsh.getFMT( fmt ) )
   {
      return false;
   }

   // if it's not a binary type, we don't have to worry about making a
   // special converter for it; since this isn't exactly an error,
   // we'll return ``true''.  (It's not an error because there are
   // many instances where the FMT is non-binary for perfectly
   // legitimate datasets.)
   if ( ! isBinaryType_( fmt ) )
   {
      return true;
   }

   // Ok, so now we've got the FMT; we merely ask the factory to give
   // us an appropriate converter and we add that to the dictionary.
   // Done!

   sio_8211Converter * converter;

   if ( ( converter = sio_ConverterFactory::instance()->get( fmt ) )  )
   {
      // the binary converter will be keyed by the attribute name

      string attribute_name;

      if ( ! ddsh.getATLB( attribute_name ) )
      {                         // Hmm, no attribute?  No converter for you!
         return false;
      }

      stringutils::chomp( attribute_name ); // nuke any trailing whitespace

      dictionary[ attribute_name ] = converter;

      return true;
   }

   // We got a binary type but the factory was unable to properly
   // parse the binary type string, so it's more than likely
   // malformed, so complain.

   return false;

} // sb_Utils::addConverter( sb_Ddsh const & ddsh,




static
bool
addConvertersFromIREF_( string const & iref_filename,
                        sio_8211_converter_dictionary & dictionary )
{
   // open the IREF module

   ifstream iref_file( iref_filename.c_str() );

   if ( ! iref_file )
   {                            // try giving us a valid file next time, ok?
      return false;
   }

   // Now grind through IREF records, calling addConverter() for @ one
   // (There SHOULD only be ONE IREF record, so the while is
   // admittedly superfluous; but it's here in case, well, there's
   // more than one.)

   sio_8211Reader reader( iref_file ); // IREF module reader

   sio_8211ForwardIterator i( reader ); // used to grind through IREF

   sc_Record record;            // current IREF record

   sb_Iref   iref_module_record;

   while ( i )
   {
      i.get( record );

      if ( ! iref_module_record.setRecord( record ) )
      {
         return false;          // unable to translate IREF record, so leave
      }

      if ( ! sb_Utils::addConverter( iref_module_record, dictionary ) )
      {
         return false;
      }

      ++i;

   } // grind to next IREF module record

   return true;

} // addConvertersFromIREF_




static
bool
addConvertersFromDDSH_( string const & ddsh_filename,
                        sio_8211_converter_dictionary & dictionary )
{
   // open the DDSH module

   ifstream ddsh_file( ddsh_filename.c_str() );

   if ( ! ddsh_file )
   {                            // try giving us a valid file next time, ok?
      return false;
   }

   // start grinding through the records; for each record call
   // addConverter()

   sio_8211Reader reader( ddsh_file ); // DDSH module reader

   sio_8211ForwardIterator i( reader ); // used to grind through DDSH

   sc_Record record;            // current DDSH record

   sb_Ddsh   ddsh_module_record;

   while ( i )
   {
      i.get( record );

      if ( ! ddsh_module_record.setRecord( record ) )
      {
         return false;          // unable to translate DDSH record, so leave
      }

      if ( ! sb_Utils::addConverter( ddsh_module_record, dictionary ) )
      {
         return false;
      }

      ++i;

   } // grind to next DDSH module record

   return true;

} // addConvertersFromDDSH_




bool
sb_Utils::addConverters( string const & catd_filename,
                         sio_8211_converter_dictionary & dictionary )
{

   // save the dir path because we'll need to prepend that to file
   // names later

   string dir_path( fileutils::dirname( catd_filename ) );

   dir_path += '/';


   // open the CATD module

   ifstream catd_file( catd_filename.c_str() );

   if ( ! catd_file )
   {                            // try giving us a valid file next time, ok?
      return false;
   }

   // start grinding through the records; for each IREF or DDSH hit,
   // open up _that_ module and start calling addConverter() for @
   // record

   sio_8211Reader reader( catd_file ); // CATD module reader

   sio_8211ForwardIterator i( reader ); // used to grind through CATD

   sc_Record record;            // current CATD record

   string catd_name;            // CATD::NAME values

   sb_Catd catd_module_record;

   string file_name;            // file name for IREF or DDSH module

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

      if ( ! catd_module_record.getFILE( file_name ) )
      {
         return false;          // dittor for CATD::FILE
      }

      string full_path_name;    // fully qualified file name; needed because
                                // these module may not be in $CWD

      full_path_name = dir_path + file_name;

      if ( "IREF" == catd_name )
      {
         if ( ! addConvertersFromIREF_( full_path_name, dictionary ) )
         {
            return false;
         }
      }
      else if  ( "DDSH" == catd_name )
      {
         if ( ! addConvertersFromDDSH_( full_path_name, dictionary ) )
         {
            return false;
         }
      }

      ++i;

   } // grind to next CATD module record

   return true;

} // sb_Utils::addConverters

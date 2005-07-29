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

#include <sdts++/builder/sb_ForeignID.h>


#ifndef INCLUDED_SIO_CONVERTERFACTORY_H
#include "io/sio_ConverterFactory.h"
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include "io/sio_8211Converter.h"
#endif

#ifndef INCLUDED_SC_FIELD_H
#include "container/sc_Field.h"
#endif

#ifndef INCLUDED_SB_UTILS_H
#include "sb_Utils.h"
#endif


#include <strstream>


using namespace std;


static const char* ident_ = 
  "$Id: sb_ForeignID.cpp,v 1.14 2002/11/24 22:07:42 mcoletti Exp $";




static
char usage_type_codes_[9] =
{ 0x0, 'S', 'E', 'L', 'R', 'F', 'B', 'I', 'X' };


/** 
    maps a usage modifer to its character representation according to
    the SDTS specification 
*/
static
char
usageModifierToChar_( sb_ForeignID::usage_t ut )
{
   return usage_type_codes_[ ut ];
} // usageModifierToChar_




bool
sb_ForeignID::packedIdentifierString( string& fid ) const
{
  if ( moduleName_.empty() || recordID_ < 0 )
    {
      return false;
    }

  strstream tmp_ss;

                                // XXX Should add sanity check to see
                                // XXX that usage modifier is in the
                                // XXX proper domain if it actually has a
                                // XXX value.

  tmp_ss << moduleName_ << "#" << recordID_;

  if ( sb_ForeignID::none != usageModifier_ )
  {
     tmp_ss << usageModifierToChar_( usageModifier_ );
  }

  getline( tmp_ss, fid );

  return true;
} // sb_ForeignID::get( string& fid )



/// These are the strings used by default for sb_ForeignID, but can be
/// over-ridden by the user.
static std::string const foreign_id_long_str = "FOREIGN ID";
static std::string const foreign_id_mnemonic = "FRID";


sb_ForeignID::sb_ForeignID() 
   : recordID_( 1 ),      // most module records start at one, not zero
     usageModifier_( sb_ForeignID::none ),
     name_( foreign_id_long_str ),
     mnemonic_( foreign_id_mnemonic )
{}


sb_ForeignID::sb_ForeignID( std::string const& name,
                            std::string const& mnemonic)
   : recordID_( 1 ),            // most module records start at one, not zero
     usageModifier_( sb_ForeignID::none ),
     name_( name ),
     mnemonic_( mnemonic )
{}


sb_ForeignID::sb_ForeignID( std::string const& mn,
                            long id,
                            sb_ForeignID::usage_t um )
   : moduleName_( mn ), 
     recordID_( id ), 
     usageModifier_( um ),
     name_( foreign_id_long_str ),
     mnemonic_( foreign_id_mnemonic )
{}




void
sb_ForeignID::addFieldToSchema( sio_8211Schema& schema,
                                std::string const & name,
                                std::string const & mnemonic,
                                bool isRepeating ) const
{
   schema.push_back( sio_8211FieldFormat() );

   sio_8211FieldFormat& field_format = schema.back();

   field_format.setDataStructCode( sio_8211FieldFormat::array );
   field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );

   // XXX probably a better way to do this; suggestive of API flaw
   if ( "" == name )
   {
      field_format.setName( sb_ForeignID::name() );
   }
   else
   {
      field_format.setName( name );
   }

   if ( "" == mnemonic )
   {
      field_format.setTag( sb_ForeignID::mnemonic() );
   }
   else
   {
      field_format.setTag( mnemonic );
   }

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "MODN" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

   field_format.back().setLabel( "RCID" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

   // XXX add usage modifier field?

   if ( isRepeating )
   {
      field_format.setIsRepeating( true );
   }

} // sb_ForeignID::addFieldToSchema( sio_8211Schema& schema )



bool
sb_ForeignID::assign( sc_Field const & field )
{
   mnemonic( field.getMnemonic() );
   name( field.getName() );

   // now pull in the module and record values

   sc_SubfieldCntr::const_iterator cursubfield;

   string tmp_str;

   if ( sb_Utils::getSubfieldByMnem( field, "MODN", cursubfield ) )
   {
      cursubfield->getA( tmp_str );
      moduleName( tmp_str );
   }
   else                         // maybe a packed id?
   {
      return false;
   }


   if ( sb_Utils::getSubfieldByMnem( field, "RCID", cursubfield ) )
   {
      long tmp_long;
      cursubfield->getI( tmp_long );
      recordID( tmp_long );
   }
   else                         // maybe a packed id?
   {
      return false;
   }

   return true;

} // sb_ForeignID::assign




static std::string const attribute_id_long_str = "ATTRIBUTE ID";
static std::string const attribute_id_mnemonic = "ATID";


sb_AttributeID::sb_AttributeID() 
   : sb_ForeignID( attribute_id_long_str, attribute_id_mnemonic )
{}



sb_AttributeID::sb_AttributeID( std::string const& mn,
                                long id,
                                sb_ForeignID::usage_t um )
   : sb_ForeignID( mn, id, um )
{}




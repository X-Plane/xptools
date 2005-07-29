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
// XXX Need to add comprehensive suite of checks instead of just the
// XXX most recent added functions.



#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>


using namespace std;


#include <cassert>

#include <sdts++/container/sc_Module.h>

#include <sdts++/builder/sb_Utils.h>
#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Ddsh.h>

#include <sdts++/io/sio_8211FieldFormat.h>



void
buildModule( sc_Module & module )
{
   sc_Record   record;
   sc_Field    field;
   sc_Subfield subfield;

   // first bogus record is FVAL::SVAL::FOO

   subfield.name( "SUBFIELD" );
   subfield.mnemonic( "SVAL" );
   subfield.setA( "FOO" );

   field.name( "FIELD" );
   field.mnemonic( "FVAL" );

   field.push_back( subfield );
   record.push_back( field );
   module.push_back( record );


   // first bogus record is FFVAL::SVAL::BAR
   field.name( "ANOTHERFIELD" );
   field.mnemonic( "FFVAL" );

   field.front().setA( "BAR" );

   record.clear();
   record.push_back( field );
   module.push_back( record );

   // module should now have two records:
   // FVAL::SVAL::FOO
   // FFVAL::SVAL::BAR

} // buildModule




int
main( int argc, char** argv )
{

   // sb_Utils::find() check

   // 1. build a fake sc_Module, replete with fake records, fields,
   //    and subfields

   sc_Module module;

   buildModule( module );

   cout << "bogus module:\n";
   cout << module << endl;

   // 2. use find() to locate records we KNOW do NOT exist and verify
   //    this is so

   sc_Module results_module;

   sc_Subfield subfield;

   subfield.name( "SUBFIELD" );
   subfield.mnemonic( "SVAL" );
   subfield.setA( "FOO" );


   sb_Utils::find( module.begin(), module.end(),
                   "BOGUSFIELDMNEMONIC",
                   subfield,
                   results_module );

   assert( results_module.empty() );


   // 3. use find() to locate records we KNOW DO exist and verify
   //    this is so



   sb_Utils::find( module.begin(), module.end(),
                   "FVAL",
                   subfield,
                   results_module );

   assert( 1 == results_module.size() );


   {

      // now we test the sb_Utils::addConverter() functions

      sb_Iref iref;             // fake IREF module object

      iref.setHFMT( "R" );      // set the horizontal format to a non-
                                // binary type

      sio_8211_converter_dictionary converters;

                                // should return true since 'R' is a
                                // valid type

      assert( sb_Utils::addConverter( iref, converters ) );

                                // however, the converter dictionary
                                // shouldn't have anything in it
                                // because 'R' is non-binary

      assert( converters.empty() );


      // now set the binary type to something intentionally bogus to see
      // if addConverters() fails as it should

      iref.setHFMT( "BAZ" );    // set the horizontal format to a bogus non-
                                // binary type


                                // should return false since "BAZ" is
                                // an invalid binary type

      assert( ! sb_Utils::addConverter( iref, converters ) );

                                // the converter dictionary should
                                // STILL be empty since "BAZ" is
                                // invalid

      assert( converters.empty() );

      // the final test would be that it works for a proper binary type


      iref.setHFMT( "BI16" );   // set the horizontal format to a good
                                // binary type


                                // should return true since "BI16" is
                                // a valid binary type

      assert( sb_Utils::addConverter( iref, converters ) );

                                // the converter dictionary should NOT
                                // be empty since "BI16" is invalid;
                                // moreover, it should have two items,
                                // one for "X" and another for "Y"
                                // subfield mnemonics

      assert( 2 == converters.size() );
   
                                // in fact, I'm so paranoid, I'll make
                                // sure those are set.

      assert( converters[ "X" ] && converters[ "Y" ] );

   }


   // Alas, there's _another_ addConverter(), this time one for the
   // DDSH; it's very similar, though, so the tests are going to look
   // a lot like the one for the IREF module.

   {

      // now we test the sb_Utils::addConverter() functions

      sb_Ddsh ddsh;             // fake DDSH module object

      ddsh.setATLB( "FOO" );    // set fake attribute label

      ddsh.setFMT( "R" );       // set the horizontal format to a non-
                                // binary type

      sio_8211_converter_dictionary converters;

                                // should return true since 'R' is a
                                // valid type

      assert( sb_Utils::addConverter( ddsh, converters ) );

                                // however, the converter dictionary
                                // shouldn't have anything in it
                                // because 'R' is non-binary

      assert( converters.empty() );


      // now set the binary type to something intentionally bogus to see
      // if addConverters() fails as it should

      ddsh.setFMT( "BAZ" );     // set the horizontal format to a bogus non-
                                // binary type


                                // should return false since "BAZ" is
                                // an invalid binary type

      assert( ! sb_Utils::addConverter( ddsh, converters ) );

                                // the converter dictionary should
                                // STILL be empty since "BAZ" is
                                // invalid

      assert( converters.empty() );

      // the final test would be that it works for a proper binary type


      ddsh.setFMT( "BI16" );    // set the horizontal format to a good
                                // binary type


                                // should return true since "BI16" is
                                // a valid binary type

      assert( sb_Utils::addConverter( ddsh, converters ) );

                                // the converter dictionary should NOT
                                // be empty since "BI16" is invalid;
                                // moreover, it should have one and
                                // only one item associated with the attribute

      assert( 1 == converters.size() );
   
                                // in fact, I'm so paranoid, I'll make
                                // sure it is set.

      assert( converters[ "FOO" ] );

   }


   { // test sb_Utils::add_field()

      sc_Record record;

      sb_Utils::add_field( record, "NAME", "MNEMONIC" );

      assert( "NAME" == record.front().name() );
      assert( "MNEMONIC" == record.front().mnemonic() );

   }

   exit( 0 );

}



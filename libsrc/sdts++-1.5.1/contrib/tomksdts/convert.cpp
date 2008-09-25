//
// convert.cpp
//

#include "convert.h"

#include <iostream>
#include <fstream>
#include <strstream>
#include <iomanip>

#include <string>
#include <algorithm>
#include <functional>

using namespace std;


#include <sdts++/container/sc_Record.h>
#include <sdts++/io/sio_Reader.h>



static const char * const ident_ =
    "$Id: convert.cpp,v 1.3 2001/05/18 23:08:36 mcoletti Exp $";



static
void
emitSubfieldSchema_( sc_Subfield const & subfield )
{
   cout << subfield.mnemonic() << " : ";

   switch( subfield.getSubfieldType() )
   {
      case sc_Subfield::is_A :
         cout << "A";
         break;
      case sc_Subfield::is_I :
         cout << "I";
         break;
      case sc_Subfield::is_R :
         cout << "R";
         break;
      case sc_Subfield::is_S :
         cout << "S";
         break;
      case sc_Subfield::is_C :
         cout << "C";
         break;
      case sc_Subfield::is_B :
         cout << "B";
         break;
      case sc_Subfield::is_BI8 :
         cout << "BI8";
         break;
      case sc_Subfield::is_BI16 :
         cout << "BI16";
         break;
      case sc_Subfield::is_BI24 :
         cout << "BI24";
         break;
      case sc_Subfield::is_BI32 :
         cout << "BI32";
         break;
      case sc_Subfield::is_BUI :
         cout << "BUI";
         break;
      case sc_Subfield::is_BUI8 :
         cout << "BUI8";
         break;
      case sc_Subfield::is_BUI16 :
         cout << "BUI16";
         break;
      case sc_Subfield::is_BUI24 :
         cout << "BUI24";
         break;
      case sc_Subfield::is_BUI32 :
         cout << "BUI32";
         break;
      case sc_Subfield::is_BFP32 :
         cout << "BFP32";
         break;
      case sc_Subfield::is_BFP64 :
         cout << "BFP64";
         break;
   }
} // emitSubfieldSchema




static
void
emitFieldSchema_( sc_Field const & field )
{
   string buffer;

   buffer = field.name( );

   cout << "field \"" << buffer << "\" ";

   buffer = field.mnemonic( );

   cout << buffer << "\n";
   cout << "{\n   ";

   sc_Field::const_iterator current_subfield = field.begin();

   do
   {
      // these two subfields are automatically handled by mksdts
      // so we don't need to emit them
      if ( "MODN" == current_subfield->mnemonic() )
      {
         current_subfield++;
         continue;
      }
      else if ( "RCID" == current_subfield->mnemonic() )
      {
         current_subfield++;
         continue;
      }

      emitSubfieldSchema_( *current_subfield );

      current_subfield ++;

      if ( field.end() != current_subfield )
      {
         cout << ", ";
      }
   } while ( current_subfield != field.end() );

   cout << "\n}\n\n";
} // emitFieldSchema_




static
void
emitSchema_( sc_Record const & record )
{
   // the first field should have information about the module, like
   // it's mnemonic and name

   string buffer = record.front().getMnemonic( ) ;

   cout << "module " << buffer << "\n\n";

                                // This assumes, of course, that the
                                // module is homogenous; i.e., it's
                                // possible that there exist fields
                                // defined in the DDR that won't
                                // necessarily show up in the first
                                // record; though this CAN happen,
                                // I've never seen that to be the case.

   for ( sc_Record::const_iterator i = record.begin() ;
         i != record.end();
         i++ )
   {
      emitFieldSchema_( *i );
   }

} // emitSchema_




static
void
emitField_( sc_Field const & field )
{
   cout << "\t" << field.mnemonic() << "\n";
   cout << "\t" << "{\n\t   ";

   sc_Field::const_iterator i = field.begin();

   do
   {
      // these two subfields are automatically handled by mksdts
      // so we don't need to emit them
      if ( "MODN" == i->mnemonic() )
      {
         i++;
         continue;
      }
      else if ( "RCID" == i->mnemonic() )
      {
         i++;
         continue;
      }

      if ( sc_Subfield::is_A == i->getSubfieldType() )
      {
         string value;
         i->getA( value );

         cout << "\""
              << value
              << "\"";
      }
      else
      {
         float f_val;
         int   i_val;

         if ( i->getFloat( f_val ) )
         {
            cout << f_val;
         }
         else if ( i->getInt( i_val ) )
         {
            cout << i_val;
         }
         else
         {
            cerr << "unknown or unhandled subfield type\n";
            exit( 7 );
         }
      }

      i++;

      if ( i != field.end() )
      {
         cout << ", ";
      }

   } while ( i != field.end() );

   cout << "\n\t}\n";

} // emitField




static
void
emitRecord_( sc_Record const & record )
{
   cout << "record\n";
   cout << "{\n";

   for ( sc_Record::const_iterator current_field = record.begin();
         current_field != record.end();
         current_field++ )
   {
      emitField_( *current_field );
   }

   cout << "\n}\n\n";

} // emitRecord_



void
convertToSDL( ifstream & module_stream )
{
   sio_8211Reader reader( module_stream ); // XXX we need to specify
                                //  binary converters

   sio_8211ForwardIterator i( reader );

   sc_Record record;

   if ( ! i.get( record ) )
   {
      cerr << "unable to read from module\n";
      exit( 5 );
   }

   // first, emit the schema section
   emitSchema_( record );


   // then blat out the records

   do
   {
      emitRecord_( record );

      ++i;

      if ( i ) { i.get( record ) ; }

   } while( i );

} // convertToSDL( ifstream module_stream )


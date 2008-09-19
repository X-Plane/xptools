//
// sb_line_t.cpp
//


#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>
#include <cassert>


using namespace std;


#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Line.h>



int
main( int argc, char** argv )
{


   if ( ! argv[1] ) 
   {
      cerr << "usage: "
           << argv[0] << " line module " << endl;
      exit( 1 );
   }

#ifdef WIN32
   ifstream ddf( argv[1], ios::binary );
#else
   ifstream ddf( argv[1] );
#endif

   if ( ! ddf ) 
   {
      cerr << "couldn't open " << argv[1] << "\n";
      exit( 2 );
   }


   sio_8211_converter_dictionary converters;

   // by default, spatial coordinates are signed 32-bit binary integers

   converters["X"] = sio_ConverterFactory::instance()->get( "BI32" );
   converters["Y"] = sio_ConverterFactory::instance()->get( "BI32" );

	
   sio_8211Reader  reader( ddf, &converters );

   sc_Record record;

   sb_Line sb_line;

   for( sio_8211ForwardIterator i( reader );
        ! i.done();
        ++i )
   {
      i.get( record );

     
      cout << "what we read in, uninterpreted:\n" 
           << record 
           << "\n";


      assert ( sb_line.setRecord( record ) );

      cout << "what the COMP builder object interpeted it as:\n";
      cout << sb_line
           << "\n";

      assert ( sb_line.getRecord( record ) );

      cout << "translated back into a raw record:\n";
      cout << record
           << "\n";

   } 

   exit( 0 );
}

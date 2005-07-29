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
//


#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>

using namespace std;

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Poly.h>

sio_8211Converter_BI8   converter_bi8;
sio_8211Converter_BI16	converter_bi16;
sio_8211Converter_BI24  converter_bi24;
sio_8211Converter_BI32	converter_bi32;
sio_8211Converter_BUI8	converter_bui8;
sio_8211Converter_BUI16	converter_bui16;
sio_8211Converter_BUI24	converter_bui24;
sio_8211Converter_BUI32	converter_bui32;
sio_8211Converter_BFP32	converter_bfp32;
sio_8211Converter_BFP64 converter_bfp64;

int
main( int argc, char** argv )
{

  if ( ! argv[1] ) 
    {
      cerr << "usage: " << argv[0] << " POLY module " << endl;
      exit( 1 );
    }

#ifdef WIN32
  ifstream ddf( argv[1], ios::binary );
#else
  ifstream ddf( argv[1] );
#endif

  if ( ! ddf ) 
    {
      cerr << "couldn't open " << argv[1] << endl;
      exit( 2 );
    }

   sio_8211_converter_dictionary converters;

  // XXX need to be more dynamic about this

  converters["X"] = &converter_bi32;
  converters["Y"] = &converter_bi32;

	
  sio_8211Reader  reader( ddf, &converters );
 
  //  sio_8211Reader  reader( ddf );
  sc_Record record;
  sb_Poly sb_poly;

  for( sio_8211ForwardIterator i( reader );
        ! i.done();
        ++i )
   {
     i.get( record );
     
     cout << "what we read in:" << endl;
     cout << record << endl;
      
     if ( ! sb_poly.setRecord( record ) ) 
       {
	 cerr << " sb_poly::setRecord() failed" << endl;
	 abort();
       }
     else
       {
         
	 sb_poly.setRecord( record );

	 cout << "\nand what the POLY object says it is:" << endl;
	 cout << sb_poly << endl;

     
       }
  }

  exit(0);
}

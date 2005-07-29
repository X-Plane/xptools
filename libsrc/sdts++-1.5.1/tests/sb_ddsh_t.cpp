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
// $Id: sb_ddsh_t.cpp,v 1.3 2002/11/27 00:21:34 mcoletti Exp $
//


#include <iostream>
#include <fstream>


using namespace std;


#include <cassert>

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Ddsh.h>


int
main( int argc, char** argv )
{

  if ( ! argv[1] ) 
    {
      cerr << "usage: " << argv[0] << " DDSH module " << endl;
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

  sio_8211Reader  reader( ddf );
  sc_Record record;
  sio_8211ForwardIterator i( reader );

  {

    sb_Ddsh ddsh;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

        ddsh.setRecord( record );

        cout << "\nand what the DDSH object says it is:\n";
        cout << ddsh << endl;

        cout << "\nand what the record built from the DDSH object says it is:\n";
        ddsh.getRecord( record );

        cout << record << endl;

        ++i;
      }
  }

  // test building a record from scratch
  {
    sb_Ddsh ddsh;

    assert( ddsh.setNAME( "Name" ) );

    assert( ddsh.setTYPE( "CELL" ) );

    assert( ddsh.setETLB( "EntityLabel" ) );

    assert( ddsh.setEUTH( "EntityAuthority" ) );

    assert( ddsh.setATLB( "AttributeLabel" ) );

    assert( ddsh.setAUTH( "AttributeAuthority" ) );

    assert( ddsh.setFMT( "Format" ) );

    assert( ddsh.setUNIT( "Unit" ) );

    assert( ddsh.setPREC( 8 ) );

    assert( ddsh.setMXLN( 9 ) );

    assert( ddsh.setKEY( "PKEY" ) );

    cout << ddsh << endl;
  }

  exit(0);
}

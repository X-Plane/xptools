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
// $Id: sb_ddom_t.cpp,v 1.3 2002/11/27 00:21:34 mcoletti Exp $
//


#include <iostream>
#include <fstream>


using namespace std;


#include <cassert>

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Ddom.h>


int
main( int argc, char** argv )
{

  if ( ! argv[1] )
    {
      cerr << "usage: " << argv[0] << " DDOM module " << endl;
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

    sb_Ddom ddom;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

        ddom.setRecord( record );

        cout << "\nand what the DDOM object says it is:\n";
        cout << ddom << endl;

        cout << "\nand what the record built from the DDOM object says it is:\n";
        ddom.getRecord( record );

        cout << record << endl;

        ++i;
      }
  }

  // test building a record from scratch
  {
    sb_Ddom ddom;

    assert( ddom.setNAME( "Name" ) );

    assert( ddom.setTYPE( "Type" ) );

    assert( ddom.setATLB( "AttributeLabel" ) );

    assert( ddom.setAUTH( "AttributeAuthority" ) );

    assert( ddom.setATYP( "AttributeDomainType" ) );

    sc_Subfield subfield;

    subfield.setA( "ADVF" );

    assert( ddom.setADVF( sc_Subfield::is_A ) );

    assert( ddom.setADMU( "AttributeDomainValueMeasurementUnit" ) );

    assert( ddom.setRAVA( "RangeOrValue" ) );

    subfield.setA( "DomainValue" );

    assert( ddom.setDVAL( subfield  ) );

    assert( ddom.setDVDF( "DomainValueDefinition" ) );

    cout << ddom << endl;
  }

  exit(0);
}

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
#include "../Windows/getopt.h"
#pragma warning( disable : 4786 )
#else
#include <cstdlib>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif


#include <iostream>
#include <fstream>
#include <vector>

#ifdef WIN32
using namespace std;
#endif

#include <cassert>
#include <cstdlib>


// stooopid IRIX has non-standard getopt() behavior; it returns -1
// instead of EOF when the last command line argument is processed

#ifdef _SGIAPI
#include <getopt.h>
#else
const int GETOPTDONE = EOF;
#endif



#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Cell.h>



void
assign_converter( sio_8211_converter_dictionary & cd,
                  string const & mnemonic,
                  string const & converter_type )
{
  if ( mnemonic.empty() )
    {
      cerr << "binary type without a subfield\n";
    }

  cd[ mnemonic ] = sio_ConverterFactory::instance()->get( converter_type );

  if ( ! cd[ mnemonic ] )
  {
     cerr << converter_type << " unknown binary type\n";
     exit( -9 );
  }

} // assign_converter





void
usage( const char * name )
{
  cerr << name << ": [flags] cell-module\n"
       << "flags:\n"
       << "\t-s\tmnemonic -b (b(u)?i(8|16|24|32))|(bfp(32|64))\n"
       << "\t\tassign binary converter to subfield mnemonic\n";
} // usage()





int
main( int argc, char** argv )
{

  string last_mnemonic;
  string subfield_mnemonic;

  sio_8211_converter_dictionary converters; // hints for reader for binary data

  extern char *optarg;
  extern int   optind;

  int ch;

  char * ifs_name = "-";

  // use BI16 converter by default
  converters["ELEVATION"] = sio_ConverterFactory::instance()->get( "BI16" );

  while((ch = getopt(argc, argv, "s:b:h")) != GETOPTDONE)
    switch(ch)
      {
      case 's':
	converters[optarg] = '\0'; // Set to null
	last_mnemonic = optarg;
	break;

      case 'b':
	assign_converter( converters, last_mnemonic, optarg );
	break;

      case 'h':
	usage( argv[0] );
        break;

      case '?':
      default:
        break;
      }


  if (optind == argc )
    {
      cerr << "must specify an SDTS module file name\n";
      exit(1);
    }

  ifs_name = argv[optind];

  ifstream ifs;                 // module to be read from


#ifdef WIN32
  ifs.open( ifs_name, ios::binary );
#else
  ifs.open( ifs_name );
#endif


  if ( ! ifs )
    {
      cerr << "unable to open " << ifs_name << "\n";
      exit(-1);
    }


  sio_8211Reader          reader( ifs, &converters );
  sio_8211ForwardIterator i( reader );

  sc_Record record;

  vector<int> elevations;

  {

    sb_Cell< vector<int> >  cell;

    while ( i )
      {
        if ( ! i.get( record ) )
          {
            cerr << "unable to read from " << ifs_name << "\n";
            exit( 42 );
          }

        cell.setRecord( record );

        cout << "\nand what the CELL object says it is:\n";
        cout << cell << endl;

        cout << "\nand what the record build from the CELL object says it is:\n";
        cell.getRecord( record );
        cout << record << endl;

        ++i;
      }
  }

#ifdef NOT_IMPLEMENTED_YET
                                // test building an CELL from scratch
  {
    sb_Cell cell;

    assert( cell.setCOMT( "This is not a comment." ) );

    assert( cell.setRDOC("Reference Documentation") );

    assert( cell.setRSNM("UTM") );

    //    assert( cell.setVDAT("Vertical Datum") );

    // assert( cell.setSDAT("MLLW") );

    assert( cell.setHDAT("Horizontal Datum") );

    assert( cell.setZONE("UTM Zone") );

    assert( cell.setPROJ("Projection") );

    cout << cell << endl;
  }
#endif

  exit( 0 );

}

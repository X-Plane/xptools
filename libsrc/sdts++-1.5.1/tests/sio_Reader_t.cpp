//
// $Id: sio_Reader_t.cpp,v 1.3 2002/11/27 00:21:34 mcoletti Exp $
//

#include <iostream>
#include <fstream>
#include <cstdlib>


#ifdef _MSC_VER
#include "../../Windows/getopt.h"
#pragma warning( disable : 4786 )
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif


using namespace std;



// stooopid IRIX has non-standard getopt() behavior; it returns -1
// instead of EOF when the last command line argument is processed

#ifdef _SGIAPI
#include <getopt.h>
#else
const int GETOPTDONE = EOF;
#endif


#include <sdts++/io/sio_Reader.h>
#include <sdts++/io/sio_ConverterFactory.h>

#include <sdts++/container/sc_Record.h>




bool totals = false;		// true if user just wants to see how many
                                // records there are in the given module



extern int EF_ALIGNMENT;
extern int EF_PROTECT_BELOW;
extern int EF_PROTECT_FREE;





void
assign_converter( sio_8211_converter_dictionary & cd,                  
                  string const & mnemonic,
                  string const & converter_type )
{
  if ( mnemonic.empty() )  
    {    
      cerr << "binary type without a subfield\n";  
    }  

  cd[ mnemonic ] = sio_ConverterFactory::instance()->get( mnemonic );

  if ( ! cd[ mnemonic ] )
  {
     cerr << converter_type << " unknown binary type\n";
     exit( -9 );
  }
} // assign_converter



void
usage( const char * name )
{
  cerr << name << ": [flags] sdts-module\n"       
       << "flags:\n"
       << "\t-t\tjust print totals\n"       
       << "\t-s\tmnemonic -b (b(u)?i(8|16|24|32))|(bfp(32|64))\n"       
       << "\t\tassign binary converter to subfield mnemonic\n";
} // usage()



int
main( int argc, char** argv )
{
// electric fence specific stuff
//     EF_PROTECT_BELOW = 1;
//     EF_ALIGNMENT = 1;
//     EF_PROTECT_FREE = 1;


  string last_mnemonic;
  string subfield_mnemonic;

  sio_8211_converter_dictionary converters; // hints for reader for binary data

  extern char *optarg;
  extern int   optind;

  int ch;

  char * ifs_name = "-";

  
  // set up meaningful default converters for these subfields

  converters["X"] = sio_ConverterFactory::instance()->get( "BI32" );
  converters["Y"] = sio_ConverterFactory::instance()->get( "BI32" );
  converters["ELEVATION"] = sio_ConverterFactory::instance()->get( "BI16" );

  while((ch = getopt(argc, argv, "ts:b:h")) != GETOPTDONE)
    switch(ch)
      {
      case 't':
        totals = true;
	break;

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


  int records = 0;

  while ( i )
    {
      i.get( record );

      if ( ! totals )
        {
          cout << record << "\n";
        }

      ++i;
      ++records;
    }

  if ( totals )
    {
      const char* singular = " record";
      const char* plural   = " records";
      const char* message = (records > 1 || 0 == records ) ? plural : singular;

      cout << ifs_name << " has " << records << message << "\n";
    }

                                // if we prematurely stopped reading,
                                // then something bad happened, so
                                // tell the user
  if ( ! ifs.eof() )
    {
      cerr << "\n" << argv[0] << ": bad data\n";
      exit( 1 );
    }
               
  ifs.close();

  {
     // Now let's test the OTHER method for using the sio_Reader.
     // That is, using the sio_Reader::attach() mechansim.

     cout << " *** now using attach to open same module\n";

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

     sc_Record record;

     sio_8211Reader reader;

     reader.attach( ifs, &converters );

     for ( sio_8211ForwardIterator i = reader.begin();
           ! i.done();
           ++i )
     {
        i.get( record );
        cout << record << "\n";
     }

     
  }


  exit( 0 );
}

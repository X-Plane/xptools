//
// $Id: mkstat.cpp,v 1.5 2003/02/13 23:37:46 mcoletti Exp $
//
// Utility for creating a STAT module from the contents of an existing
// CATD module.
//

#include <iostream>
#include <fstream>
#include <strstream>

#include <algorithm>

#include <cstdlib>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;

#include "sysutils/fileutils.h"



// stooopid IRIX has non-standard getopt() behavior; it returns -1
// instead of EOF when the last command line argument is processed

#ifdef _SGIAPI
#include <getopt.h>
#else
const int GETOPTDONE = EOF;
#endif



#include <sdts++/container/sc_Record.h>
#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Writer.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/builder/sb_Module.h>
#include <sdts++/builder/sb_Stat.h>
#include <sdts++/builder/sb_Catd.h>


#include "emit.h"


static const char* _ident = 
   "$Id: mkstat.cpp,v 1.5 2003/02/13 23:37:46 mcoletti Exp $";



bool verbose;                   // true if user wants noisy output
bool force_lowercase;           // XXX Kludge!  This doesn't need it!



void
usage( const char* executable )
{
  cout << "Usage:\t"
       << executable
       << " [-h]"
       << " [-v]"
       << " <CATD module filename>"
       << " <STAT module filename>\n";

  exit( -1 );

} // usage





//
// Given a file name for a module, open it and return how many records it has.
//
static
int
_getRecordCount( string const& file_name )
{
  ifstream ifs( file_name.c_str(), ios::in );

  if ( ! ifs )
  {
    cerr << "unable to open module " << file_name << "\n";
    return 0;
  }

  sio_8211Reader reader( ifs );

  sio_8211ForwardIterator i( reader );

  int count = 0;

  while ( i )
  {
    ++count;
    ++i;
  }

  ifs.close();

  return count;

} // _getRecordCount
 



void
create_stat( ifstream& catd_stream, 
             string const& catd_fn, 
             string const& stat_fn,
             ofstream& stat_stream )
{
  // first blat out the 8211 DDR

  sb_Stat stat_module;


  sio_8211Schema stat_schema;

  stat_module.getSchema( stat_schema );

  sio_8211Writer writer( stat_stream, 
                         fileutils::basename( stat_fn ), 
                         stat_schema );


  writer.emitDDR();             // blat out ISO 8211 header


                                // first we iterate through the catalog
                                // module so that we're assured of
                                // getting the canonical file names
                                // for this transfer -- I didn't want
                                // to risk using any sort of directory
                                // walking scheme that would
                                // inadvertently pick up files that
                                // happened to be leftover from other
                                // transfer encoding runs


  sio_8211Reader reader( catd_stream );
  sio_8211ForwardIterator record_itr( reader );

  sc_Record record;

  sb_Catd catd_module;


  string path = fileutils::dirname( catd_fn );

                                // assume that all the
                                // modules we're going to look at are
                                // in the same directory as the CATD
                                // module; so, strip out the directory
                                // part of the path name so we can
                                // later prepend that to the file
                                // names we find in the CATD module


  path += "/";                  // '/' terminate path


  string tmp_string;

  long id_counter = 0;


  stat_module.setNSAD( 0 );     // set once and then re-used

  while( record_itr )
  {
    record_itr.get( record );

    if ( ! catd_module.setRecord( record ) )
      {
        cerr << catd_fn << " is not a valid CATD module ... aborting\n";
        exit( 1 );
      }

    catd_module.getNAME( tmp_string );
    stat_module.setMNRF( tmp_string );

    if ( "STAT" == tmp_string ) // we're currently writing it, so just
    {                           // skip it; we'll build it last
      ++record_itr;
      continue;
    }

    id_counter++;
    stat_module.setID( id_counter ); // Set RCID

    catd_module.getTYPE( tmp_string );
    stat_module.setMNTF( tmp_string );

    catd_module.getFILE( tmp_string );
    stat_module.setNREC(  _getRecordCount( path + tmp_string )  );

    emit( stat_module, writer );

    ++record_itr;
  }

                                // now add the STAT module record
  id_counter++;

  stat_module.setID( id_counter );

  stat_module.setMNTF( "Transfer Statistics" );
  stat_module.setNREC( id_counter );
  stat_module.setMNRF( "STAT" );

  emit( stat_module, writer );

} // create_stat


int
main( int argc, char** argv )
{

#ifdef EF
  EF_PROTECT_FREE  = 1;         // ElectricFence
  EF_PROTECT_BELOW = 0;
  EF_ALIGNMENT     = 0;
#endif


  extern char *optarg;
  extern int   optind;

  string catd_filename;
  string stat_filename;

  verbose = false;

  int ch;                      // getopt char buffer

  while((ch = getopt(argc, argv, "hv")) != GETOPTDONE)
    switch(ch)
    {
      case 'v':
        verbose = true;
        break;

      case 'h':
        usage( argv[0] );
        exit( 0 );
        break;

      case '?':
      default:
        usage( argv[0] );
        exit( 1 );
        break;
    }

  if ( optind == argc )
    {
      cerr << "missing CATD module file name\n";
      usage( argv[0] );
      exit( 1 );
    }

  catd_filename = argv[optind];

  if ( catd_filename.empty() )
    {
      cerr << "must specifiy CATD module filename\n";
      usage( argv[0] );
      exit( 1 );
    }

  if ( ! argv[optind+1] )
    {
      cerr << "must specifiy a STAT module filename\n";
      usage( argv[0] );
      exit( 1 );
    }

  stat_filename = argv[optind+1];


  ifstream catd_stream;

  catd_stream.open( catd_filename.c_str(), ios::in );

  if( ! catd_stream )
    {
      cout << "Unable to open CATD module " << catd_filename 
           << ".  Aborting.\n";
      return 2;
    }


  ofstream stat_stream;

  stat_stream.open( stat_filename.c_str(), ios::out );

  if( ! stat_stream )
    {
      cout << "Unable to create STAT module " << stat_filename 
           << ".  Aborting.\n";
      return 2;
    }
                                //  now go run off and make the STAT
                                //  module by iterating through the
                                //  CATD module, getting file names,
                                //  opening those up, and then
                                //  counting each record.

  create_stat( catd_stream, catd_filename, stat_filename, stat_stream );



  catd_stream.close();          // clean-up
  stat_stream.close();


  return 0;

}

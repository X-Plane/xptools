//
// sio_ReadWrite.cpp
//



// stooopid IRIX has non-standard getopt() behavior; it returns -1
// instead of EOF when the last command line argument is processed
#ifdef _SGIAPI
#   include <getopt.h>
#elif _MSC_VER
#   pragma warning(disable:4786)
#   include <Windows/getopt.h>
    const int GETOPTDONE = -1;
#else
#   include <cstdlib>
#   include <cstdio>
#   ifdef HAVE_UNISTD_H
#      include <unistd.h>
#   endif
    const int GETOPTDONE = EOF;
#endif



#include <iostream>
#include <fstream>
#include <algorithm>


#include <sdts++/io/sio_Reader.h>
#include <sdts++/io/sio_Writer.h>
#include <sdts++/io/sio_ConverterFactory.h>



#include <sdts++/container/sc_Record.h>
#include <sdts++/container/sc_Field.h>
#include <sdts++/container/sc_Subfield.h>


using namespace std;


static const char* _ident = "$Id: sio_ReadWrite_t.cpp,v 1.11 2002/11/27 00:21:34 mcoletti Exp $";




bool verbose     = false;       // true if user wants noisy output

bool record_iden = false;       // true if user wants an 8211 record
                                // identifier field emitted with each DR




void
usage( const char * name )
{
  cerr << name << ": [flags] in_8211_file out_8211_file\n"
       << "  flags:\n"
       << "\t-v\tverbose\n"
       << "\t-i\tuse 8211 record identifier field\n"
       << "\t-r\treuse 8211 leaders and directories\n"
       << "\t-s mnemonic -b (b(u)?i(8|16|24|32))|(bfp(32|64))\n"
       << "\t\tassign binary converter to subfield mnemonic\n";
} // usage()






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



// returns true if the schema contains the ISO 8211 Reserved Field
bool
foundRecIdenField( sio_8211Schema const & schema )
{
  // find the DDF RECORD IDENTIFIER FIELD, which has a field
  // mnemonic of "0001"

  sio_8211Schema::const_iterator field_format = 
    find( schema.begin(), schema.end(), "0001" );

  if ( field_format == schema.end() ) 
  {
    return false;
  }

  return true;

} // foundRecIdenField




int
main( int argc, char** argv )
{

  extern char *optarg;
  extern int   optind;

  bool         reuse_flag = false;
  string       subfield_mnemonic;

  sio_8211_converter_dictionary converters;

                                // set up some reasonable defaults
  // set up meaningful default converters for these subfields

  converters["X"] = sio_ConverterFactory::instance()->get( "BI32" );
  converters["Y"] = sio_ConverterFactory::instance()->get( "BI32" );
  converters["ELEVATION"] = sio_ConverterFactory::instance()->get( "BI16" );

  string last_mnemonic;

  int ch;

  while ((ch = getopt(argc, argv, "rvis:b:")) != GETOPTDONE)
  {
    switch(ch) 
    {
    case 'r':
      reuse_flag = true;	// reuse directories
      continue;

    case 'v':
      verbose = true;		// verbose output
      continue;

    case 'i':
      record_iden = true;       // use 8211 record identifier field
      continue;

    case 's' :
      converters[optarg] = 0x0;	// initialize it to null
      last_mnemonic = optarg;   // save the mnemonic
      continue;

    case 'b' :
      assign_converter( converters, last_mnemonic, optarg );
      continue;
  
    case '?':
    default:
      usage( argv[0] );
      return -4;
    }
  }


  if (optind == argc )
    {
      cerr << "must specify an SDTS module file name\n";
      exit(1);
    }


  char*        ifs_name = 0x0;
  char*        ofs_name = 0x0;

  ifs_name = argv[optind++];

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

  sio_8211Reader  reader( ifs, &converters );
  sio_8211ForwardIterator i ( reader );



  ofs_name = argv[optind++];

  if ( ! ofs_name )
  {
      cerr << "must specify an output file\n";
      exit(1);
  }

  ofstream ofs;

#ifdef WIN32
  ofs.open( ofs_name, ios::binary );
#else
  ofs.open( ofs_name );
#endif

  if ( ! ofs )
  {
      cerr << "couldn't open " << ofs_name << "\n";
      return -3;
  }


  sio_8211Schema schema = reader.getSchema();


                                // If the user wants to use the ISO
                                // 8211 record identifier field, then
                                // prepend the appropriate field
                                // format to the front of the schema
                                // if it does not already exist there.
                                // This will clue in the sio_Wrtiter
                                // to emit that field with proper
                                // values for each 8211 DR.

  if ( record_iden && ! foundRecIdenField(schema) )
  {
     schema.push_front( sio_8211FieldFormat() );
     schema.front().setDataStructCode( 
        sio_8211FieldFormat::elementary );
     schema.front().setDataTypeCode( 
        sio_8211FieldFormat::implicit_point );
     schema.front().setName( "DDF RECORD IDENTIFER" );
     schema.front().setTag( "0001" );

  }

  sio_8211Writer writer( ofs, ofs_name, schema );

  writer.emitDDR();             // blat out the DDR based on the file name and
                                // schema information

  sc_Record record;


  int records = 0;

  if ( reuse_flag )
  {
    writer.reuseLeaderAndDirectory(); // use dropped leaders and directories
  }


  while ( i )
    {
      i.get( record );

      if ( verbose )
	{
	  cout << record << "\n";
	}

      writer.put( record );

      ++i;
      ++records;
    }


  ifs.close();
  ofs.close();

  cout << "wrote " << records << " record(s) to " << ofs_name << "\n";

  return 0;
}

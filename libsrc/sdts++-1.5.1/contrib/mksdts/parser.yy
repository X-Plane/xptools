%{

#include <unistd.h>
#include <cstdio>
#include <cstring>




// stooopid IRIX has non-standard getopt() behavior; it returns -1
// instead of EOF when the last command line argument is processed

#ifdef _SGIAPI
#include <getopt.h>
#else
const int GETOPTDONE = EOF;
#endif

#include <iostream>
#include <fstream>
#include <strstream>
#include <iomanip>

#include <string>
#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;

#include <sdts++/container/sc_Record.h>
#include <sdts++/container/sc_Field.h>
#include <sdts++/container/sc_Subfield.h>

#include <sdts++/io/sio_Writer.h>
#include <sdts++/io/sio_8211Converter.h>

#include <sdts++/builder/sb_Utils.h>



int yylex();
int yyerror( const char * s );



extern int lineno;		// current line number in the source

sio_8211Schema schema;		// schema used to build the DDR and to
				// properly write out 8211 records; i.e.,
				// this contains field and subfield information

ofstream       outfile;         // output file stream for 8211 module
string         outfile_name;    // the file name associated with that stream

sio_8211Writer writer( outfile, "" );


sc_Record record;		// SDTS output record


sio_8211Schema::const_iterator field_format_itr; 
                                // current field definition iterator

sio_8211Schema::const_iterator first_user_field_format_itr; 
                                // points to the first user field in the schema;
                                // that is, if there is a reserved field,
                                // then this will point to the first field
                                // after it, otherwise this will refer to
                                // the first field

sio_8211FieldFormat::const_iterator subfield_format_itr;
                                // current subfield definition iterator


string module_mnemonic;		// mnemonic for the module

int    record_id = 1;           // current SDTS record number


bool verbose = false;		// true if user wants copious output

bool recIdenField = false;      // true if user wants record identfier fields

string yylval_string;		// current string value



sio_8211Converter_A     converter_A; // converters bound to subfields
sio_8211Converter_I     converter_I;
sio_8211Converter_R     converter_R;
sio_8211Converter_S     converter_S;
sio_8211Converter_BI8   converter_BI8;
sio_8211Converter_BI16  converter_BI16;
sio_8211Converter_BI24  converter_BI24;
sio_8211Converter_BI32  converter_BI32;
sio_8211Converter_BUI8  converter_BUI8;
sio_8211Converter_BUI16 converter_BUI16;
sio_8211Converter_BUI24 converter_BUI24;
sio_8211Converter_BUI32 converter_BUI32;
sio_8211Converter_BFP32 converter_BFP32;
sio_8211Converter_BFP64 converter_BFP64;






// used to blat out noisy output if verbose true
template <class T>
void
echo_subfield_assignment( string const& label, T const & value )
{
  cout << "\tadded subfield (" 
       << label
       << ":"
       << value
       << ")" << endl;
} // echo_subfield_assignment


%}



%union
{
  long   i_val;
  double d_val;
}

%token <i_val> INT, BINARY
%token <d_val> FLOAT
%token STRING, ID
%token FIELD, RECORD, MODULE
%token BI8, BI16, BI24, BI32
%token BUI8, BUI16, BUI24, BUI32
%token BFP32, BFP64
%token REGEXP, DROP


%start module_def


%%



module_def : MODULE ID 
             {
	       module_mnemonic = yylval_string;
	       if ( verbose )
		 { cout << "module:\t " << module_mnemonic << endl; }
	     } 
             field_defs 
             {


#ifdef SDTSXXDEBUG

                 cout << "\n" << __FILE__ << ":" << __LINE__ << "\n";

                 ostream_iterator<sio_8211FieldFormat> ostr_it( cout, "\n" );

                 copy( schema.begin(), schema.end(), ostr_it );
#endif


                                // We have to push the RCID and MODN
                                // subfields in reverse order as the
                                // schema already contains the field
                                // format entry for all the fields

                                // first RCID 

               schema.front().push_front( sio_8211SubfieldFormat() );

               schema.front().front().setLabel( "RCID" );
               schema.front().front().setFormat( 
                  sio_8211SubfieldFormat::variable );
               schema.front().front().setType( sio_8211SubfieldFormat::I ); 
               schema.front().front().setConverter( &converter_I );


                                // then MODN

               schema.front().push_front( sio_8211SubfieldFormat() );

               schema.front().front().setLabel( "MODN" );
               schema.front().front().setFormat( 
                  sio_8211SubfieldFormat::variable );
               schema.front().front().setType( sio_8211SubfieldFormat::A ); 
               schema.front().front().setConverter( &converter_A );

                                // save the place of the first user
                                // defined field since the record
                                // identifier field might be prepended
                                // before it

               first_user_field_format_itr = schema.begin();



#ifdef SDTSXXDEBUG

  cout << "\n" << __FILE__ << ":" << __LINE__ << "\n";

  copy( schema.begin(), schema.end(), ostr_it );

#endif

                                // if the -r switch used, then add the
                                // ISO 8211 record identifier reserved
                                // field

               if ( recIdenField )
                 {
                   schema.push_front( sio_8211FieldFormat() );
                   schema.front().setDataStructCode( 
                       sio_8211FieldFormat::elementary );
                   schema.front().setDataTypeCode( 
                       sio_8211FieldFormat::implicit_point );
                   schema.front().setName( "DDF RECORD IDENTIFER" );
                   schema.front().setTag( "0001" );
                 }


#ifdef SDTSXXDEBUG

  cout << "\n" << __FILE__ << ":" << __LINE__ << "\n";

  copy( schema.begin(), schema.end(), ostr_it );

  cout << "\n" << endl;;

#endif

                                // tell the writer the field and subfield
                                // formats

               writer.setSchema( schema );

               if ( verbose )
                 {
                   ostream_iterator<sio_8211FieldFormat> osff_itr( cout, "\n");
                   copy( schema.begin(), schema.end(), osff_itr );
                 }

               writer.emitDDR(); // schema built, so blat out DDR

             }
             records ;


field_defs : field_def | field_defs field_def;

field_def  : FIELD STRING
             {  // XXX What about defining arrays?
	       schema.push_back( sio_8211FieldFormat() );
	       schema.back().setDataStructCode( sio_8211FieldFormat::vector );
	       schema.back().setDataTypeCode( 
				  sio_8211FieldFormat::mixed_data_type );
	       schema.back().setName( yylval_string );
	     }
             ID
             {
	       schema.back().setTag( yylval_string );
	     }
	     '{'
	     subfield_defs
	     '}' 
	;

subfield_defs : | subfield_def | subfield_defs ',' subfield_def ;

subfield_def  :	ID 
                {
		  schema.back().push_back( sio_8211SubfieldFormat() );
		  schema.back().back().setLabel( yylval_string );
		  schema.back().back().setFormat( // XXX, what if fixed?
                             sio_8211SubfieldFormat::variable );
		} 
                ':' type ;

type :	   'A' { schema.back().back().setType( sio_8211SubfieldFormat::A );
                 schema.back().back().setConverter( &converter_A ); }
        |  'I' { schema.back().back().setType( sio_8211SubfieldFormat::I ); 
                 schema.back().back().setConverter( &converter_I ); }
        |  'R' { schema.back().back().setType( sio_8211SubfieldFormat::R ); 
                 schema.back().back().setConverter( &converter_R ); }
        |  'S' { schema.back().back().setType( sio_8211SubfieldFormat::S ); 
                 schema.back().back().setConverter( &converter_S ); }
        |   BI8 { schema.back().back().setType( sio_8211SubfieldFormat::B );
	          schema.back().back().setLength(8); 
		  schema.back().back().setConverter( &converter_BI8 ); }
        |   BI16 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(16); 
		  schema.back().back().setConverter( &converter_BI16 ); }
        |   BI24 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(24); 
		  schema.back().back().setConverter( &converter_BI24 ); }
        |   BI32 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(32); 
		  schema.back().back().setConverter( &converter_BI32 ); }
        |   BUI8 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(8); 
		  schema.back().back().setConverter( &converter_BUI8 ); }
        |   BUI16 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(16); 
		  schema.back().back().setConverter( &converter_BUI16 ); }
        |   BUI24 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(24); 
		  schema.back().back().setConverter( &converter_BUI24 ); }
        |   BUI32 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(32); 
		  schema.back().back().setConverter( &converter_BUI32 ); }
        |   BFP32 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(32); 
		  schema.back().back().setConverter( &converter_BFP32 ); }
        |   BFP64 { schema.back().back().setType( sio_8211SubfieldFormat::B ); 
	          schema.back().back().setLength(64); 
		  schema.back().back().setConverter( &converter_BFP64 ); }
        |   REGEXP { cerr << "regular expressions not handled yet\n" ; exit(-1);}
	;


records :	record | records record ;

record  :	RECORD
                {
                  if ( verbose)
                    {
                      cout << "adding record " << record_id << endl;
                    }
                                // write out record
                } '{' fields '}' 
                {
                  if ( ! writer.put( record ) )
                    {
                      cerr << "problem writing record ... aborting\n";
                      exit( -1 );
                    }

                                // reset for next record
                  record.clear();
                }
        ;

fields	:	field | fields field ;

field	:	ID 
                {               // find the field format and add the 
                                // field to the global record container
                  field_format_itr = find( schema.begin(), 
                                           schema.end(), 
                                           yylval_string );

                  if ( field_format_itr == schema.end() )
                    {
                      cerr << yylval_string << " field definition not found\n";
                      return -1;
                    }

                  if ( verbose )
                    {
                      cout << "    adding field "
                           << (*field_format_itr).getTag() << endl;
                    }

                  subfield_format_itr = (*field_format_itr).begin();

                  if ( subfield_format_itr == (*field_format_itr).end() )
                    {
                      cerr << "no subfield formats defined for " 
                           << yylval_string << "\n";
                      return -1;
                    }

                  record.push_back( sc_Field() );
                  record.back().setMnemonic( (*field_format_itr).getTag() );

                                // Add MODN and RCID, if necessary (i.e., 
                                // if it's the first field, then it should
                                // automatically have the MODN & RCID)

                  if ( field_format_itr == first_user_field_format_itr && 
                       (*subfield_format_itr).getLabel() == "MODN" )
                    {
                                // MODN first

                      record.back().push_back( sc_Subfield() );
                      record.back().back().setMnemonic( 
                        (*subfield_format_itr).getLabel() );
                      record.back().back().setA( module_mnemonic );

                      if ( verbose )
                        {
                          cout << "\tadded subfield (" 
                               << (*subfield_format_itr).getLabel()
                               << ":"
                               << module_mnemonic
                               << ")" << endl;
                        }

                      subfield_format_itr++;

                                // then RCID

                      record.back().push_back( sc_Subfield() );
                      record.back().back().setMnemonic( 
                        (*subfield_format_itr).getLabel() );
                      record.back().back().setI( record_id++ );


                      if ( verbose )
                        {
                          cout << "\tadded subfield (" 
                               << (*subfield_format_itr).getLabel()
                               << ":"
                               << record_id - 1
                               << ")" << endl;
                        }

                      subfield_format_itr++;

                      yylval_string = ""; // clear for next token

                    }

                }
                '{' data '}' ;

data    : 	datum
                | data ',' datum ;

datum 	:	BINARY 
                { // add subfield to the last field in the record
                  record.back().push_back( sc_Subfield() );
                  record.back().back().setMnemonic( 
                    (*subfield_format_itr).getLabel() );
                  record.back().back().setI( $1 );

                  if ( verbose )
                    {
                      echo_subfield_assignment( (*subfield_format_itr).getLabel(),
                                                yylval_string );
                    }
                  subfield_format_itr++;
                }
        |       INT 
                { // add subfield to the last field in the record
                  record.back().push_back( sc_Subfield() );
                  record.back().back().setMnemonic( 
                    (*subfield_format_itr).getLabel() );

                  switch ( (*subfield_format_itr).getType() )
                    {
                    case sio_8211SubfieldFormat::A:
                      cerr << "got " << $1 << " when a string was expected\n";
                      exit( -1 );
                      break;

                    case sio_8211SubfieldFormat::I: 
                      record.back().back().setI( $1 );
                      break;

                    case sio_8211SubfieldFormat::R:
                      record.back().back().setR( $1 );
                      break;
                      
                    case sio_8211SubfieldFormat::S: 
                      record.back().back().setS( $1 );
                      break;
                      
                    case sio_8211SubfieldFormat::C:
                      cerr << "unsupported subfield type 'C'\n"; 
                      exit(-1);
                      break;

                    case sio_8211SubfieldFormat::B: 
                      record.back().back().setBI32( $1 ); // XXX hack!
                      break;

                    case sio_8211SubfieldFormat::X:
                      cerr << "unsupported subfield type 'X'\n"; exit(-1);

                    default:
                      cerr << "invalid sio_8211SubfieldFormat type\n"; exit(-1);
                    }

                  

                  if ( verbose )
                    {
                      echo_subfield_assignment( (*subfield_format_itr).getLabel(),
                                                yylval_string );
                    }
                  subfield_format_itr++;
                }
        |       FLOAT 
                { // add subfield to the last field in the record
                  record.back().push_back( sc_Subfield() );
                  record.back().back().setMnemonic( 
                    (*subfield_format_itr).getLabel() );


                  switch ( (*subfield_format_itr).getType() )
                    {
                    case sio_8211SubfieldFormat::A:
                      cerr << "got " << $1 << " when a string was expected\n";
                      exit( -1 );
                      break;

                    case sio_8211SubfieldFormat::I: 
                      record.back().back().setI( $1 );
                      break;

                    case sio_8211SubfieldFormat::R:
                      record.back().back().setR( $1 );
                      break;
                      
                    case sio_8211SubfieldFormat::S: 
                      record.back().back().setS( $1 );
                      break;
                      
                    case sio_8211SubfieldFormat::C:
                      cerr << "unsupported subfield type 'C'\n"; 
                      exit(-1);
                      break;

                    case sio_8211SubfieldFormat::B: 
                      record.back().back().setBI32( $1 ); // XXX hack!
                      break;

                    case sio_8211SubfieldFormat::X:
                      cerr << "unsupported subfield type 'X'\n"; exit(-1);

                    default:
                      cerr << "invalid sio_8211SubfieldFormat type\n"; exit(-1);
                    }


                  if ( verbose )
                    {
                      echo_subfield_assignment( (*subfield_format_itr).getLabel(),
                                                yylval_string );
                    }
                  subfield_format_itr++;
                }
        |       STRING 
                { // add subfield to the last field in the record
                  record.back().push_back( sc_Subfield() );
                  record.back().back().setMnemonic( 
                    (*subfield_format_itr).getLabel() );
                  record.back().back().setA( yylval_string );

                  if ( verbose )
                    {
                      echo_subfield_assignment( (*subfield_format_itr).getLabel(),
                                                yylval_string );
                    }

                  subfield_format_itr++;
                }
        |       // null datum, so add empty subfield
                {

                  sc_Subfield::SubfieldType subfield_type;

                  switch ( (*subfield_format_itr).getType() )
                    {
                    case sio_8211SubfieldFormat::A:
                      subfield_type = sc_Subfield::is_A; break;
                    case sio_8211SubfieldFormat::I: 
                      subfield_type = sc_Subfield::is_I; break;
                    case sio_8211SubfieldFormat::R:
                      subfield_type = sc_Subfield::is_R; break;
                    case sio_8211SubfieldFormat::S: 
                      subfield_type = sc_Subfield::is_S; break;
                    case sio_8211SubfieldFormat::C:
                      subfield_type = sc_Subfield::is_C; break;
                    case sio_8211SubfieldFormat::B: 
                      cerr << "can't have null binary subfields\n"; exit(-1); 
                    case sio_8211SubfieldFormat::X:
                      cerr << "unsupported subfield type 'X'\n"; exit(-1);
                    default:
                      cerr << "invalid sio_8211SubfieldFormat type\n"; exit(-1);
                    }

                  sb_Utils::add_empty_subfield( record.back(),
                                                (*subfield_format_itr).getLabel(), 
                                                subfield_type );

                  if ( verbose )
                    {
                      echo_subfield_assignment( (*subfield_format_itr).getLabel(),
                                                yylval_string );
                    }

                  subfield_format_itr++;
                }
        ;



%%


const char* _ident = "$Id: parser.yy,v 1.7 2003/02/13 23:37:45 mcoletti Exp $";


int
yyerror( const char * s )
{
   cout << __FILE__ << " : " << s << " at " << lineno << endl;
} // yyerror


void
usage( string const & fn )
{
  cerr << fn << " : [-r] [-v] [-i infile] -o outfile\n"
       << "\t-r : toggle on writing ISO 8211 record identifier fields\n"
       << "\t-v : toggle on noisy output\n"
       << "\t-i : file containing SDTS definition language\n"
       << "\t-o : generated SDTS module file\n";
} // usage


int
main( int argc, char** argv )
{
  // yydebug = 0;
  lineno  = 1;

  extern FILE* yyin;            // what the lexer reads from

  extern char *optarg;		// getopt() variables
  extern int   optind;
         int   ch;

  while ((ch = getopt(argc, argv, "vri:o:")) != GETOPTDONE)
    switch(ch) 
      {
    case 'v':
      verbose = true;           // verbose output
      break;

    case 'r':
      recIdenField = true;
      break;

    case 'i':
#ifdef WIN32
      yyin = fopen( optarg, "rb" );
#else
      yyin = fopen( optarg, "r" );
#endif
      if ( ! yyin )
      {
        cerr << "couldn't open " << optarg << "\n";
        return -2;
      }

      break;

    case 'o':
#ifdef WIN32
      outfile.open( optarg, ios::binary );
#else
      outfile.open( optarg );
#endif
      if ( ! outfile )
      {
        cerr << "couldn't open " << optarg << "\n";
        return -3;
      }
      outfile_name = optarg;
      writer.setFileTitle( optarg );
      break;

    case '?':
    default:
      usage( argv[0] );
      return -4;
    }

  if ( outfile_name.empty() )
    {
      cerr << "missing output file name\n";
      usage( argv[0] );
      return -5;
    }


  if ( ! yyin )  /* by default, we'll use standard input */
    {
      yyin = stdin;
    }


  if ( 0 == yyparse() )
  {
    cout << "wrote " << outfile_name << endl;
  }
  else
  {
    cout << "error at line " << lineno << ", no file written" << endl;
    unlink( outfile_name.c_str() );
  }

  return 0;
}

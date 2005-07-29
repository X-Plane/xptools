//
// sio_Writer.cpp
//

#include <iostream>
#include <iterator>
#include <fstream>



using namespace std;



#include <sdts++/io/sio_Writer.h>


#include <sdts++/container/sc_Record.h>
#include <sdts++/container/sc_Field.h>
#include <sdts++/container/sc_Subfield.h>

#include <sdts++/io/sio_ConverterFactory.h>


#define VERBOSE


//
// makes a bogus schema for a repeating binary field
//
void
build_binary_schema( sio_8211Schema& schema )
{

  // BOGUS field

  schema.push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat&  field_format = schema.back();

  field_format.setDataStructCode( sio_8211FieldFormat::array );
  field_format.setDataTypeCode( sio_8211FieldFormat::bit_string );
  field_format.setName( "BOGUS" );
  field_format.setTag( "BLAH" );
  field_format.setIsRepeating( true ); // hint that this is a _repeating_ 
                                // binary field, so that the extra
                                // parenthesis will be added

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "X" );
  field_format.back().setType( sio_8211SubfieldFormat::B );
  field_format.back().setLength( 32 );
  // XXX to be added: field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "Y" );
  field_format.back().setType( sio_8211SubfieldFormat::B );
  field_format.back().setLength( 32 );
  // XXX to be added: field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

} // build_binary_schema




//
// builds a schema suitable for writing an SDTS IDEN module
//
void
build_iden_schema( sio_8211Schema& schema )
{



  // IDENTIFICATION field

  schema.push_back( sio_8211FieldFormat() );

  sio_8211FieldFormat& field_format = schema.back();

  field_format.setDataStructCode( sio_8211FieldFormat::vector );
  field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  field_format.setName( "IDENTIFICATION" );
  field_format.setTag( "IDEN" );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MODN" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "RCID" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "STID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "STVS" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DOCU" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PRID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PRVS" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "PDOC" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "TITL" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DAID" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DAST" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "MPDT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "DCDT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "SCAL" );
  field_format.back().setType( sio_8211SubfieldFormat::I );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

  field_format.push_back( sio_8211SubfieldFormat() );

  field_format.back().setLabel( "COMT" );
  field_format.back().setType( sio_8211SubfieldFormat::A );
  field_format.back().setFormat( sio_8211SubfieldFormat::variable );
  field_format.back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );
   


  // CONFORMANCE field

  schema.push_back( sio_8211FieldFormat() );

  schema.back().setDataStructCode( sio_8211FieldFormat::vector );
  schema.back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema.back().setName( "CONFORMANCE" );
  schema.back().setTag( "CONF" );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "FFYN" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "VGYN" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "GTYN" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "RCYN" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "EXSP" );
  schema.back().back().setType( sio_8211SubfieldFormat::I );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "FTLV" );
  schema.back().back().setType( sio_8211SubfieldFormat::I );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "CDLV" );
  schema.back().back().setType( sio_8211SubfieldFormat::I );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "NGDM" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );


  // ATTRIBUTE ID field

  schema.push_back( sio_8211FieldFormat() );

  schema.back().setDataStructCode( sio_8211FieldFormat::array );
  schema.back().setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
  schema.back().setName( "ATTRIBUTE ID" );
  schema.back().setTag( "ATID" );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "MODN" );
  schema.back().back().setType( sio_8211SubfieldFormat::A );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "A" ) );

  schema.back().push_back( sio_8211SubfieldFormat() );

  schema.back().back().setLabel( "RCID" );
  schema.back().back().setType( sio_8211SubfieldFormat::I );
  schema.back().back().setFormat( sio_8211SubfieldFormat::variable );
  schema.back().back().setConverter( sio_ConverterFactory::instance()->get( "I" ) );


} // build_iden_schema




void
add_subfield( sc_Field& field, string const& mnemonic, string const& value )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );
  field.back().setA( value );
} // add_subfield




void
add_subfield( sc_Field& field, string const& mnemonic, int value )
{
  field.push_back( sc_Subfield() );

  field.back().setMnemonic( mnemonic );
  field.back().setI( value );
} // add_subfield




void
build_iden_record( sc_Record& record )
{
  record.push_back( sc_Field() );

  record.back().setMnemonic( "IDEN" );
   
  add_subfield( record.back(), "MODN", "IDEN" );
  add_subfield( record.back(), "RCID", 1 );
  add_subfield( record.back(), "STID", "SPATIAL DATA TRANSFER STANDARD" );
  add_subfield( record.back(), "STVS", "1994 JUNE 10" );
  add_subfield( record.back(), "DOCU", "FIPS PUB 173-1" );
  add_subfield( record.back(), "PRID", "SDTS TOPOLOGICAL VECTOR PROFILE" );
  add_subfield( record.back(), "PRVS", "VERSION 1.0 JUNE 10, 1994" );
  add_subfield( record.back(), "PDOC", "FIPS 173-1 PART 4" );
  add_subfield( record.back(), "TITL", "BOGUS TITLE" );
  add_subfield( record.back(), "DAID", "" );
  add_subfield( record.back(), "DAST", "DEM" );
  add_subfield( record.back(), "MPDT", "1986" );
  add_subfield( record.back(), "DCDT", "19970820" );
  add_subfield( record.back(), "SCAL", 100000 );
  add_subfield( record.back(), "COMT", "Generated by sdts++." );

  record.push_back( sc_Field() );

  record.back().setMnemonic( "CONF" );
   
  add_subfield( record.back(), "FFYN", "Y" );
  add_subfield( record.back(), "VGYN", "Y" );
  add_subfield( record.back(), "GTYN", "Y" );
  add_subfield( record.back(), "RCYN", "N" );
  add_subfield( record.back(), "EXSP", 1 );
  add_subfield( record.back(), "FTLV", 4 );

} // build_iden_record



   
int
main( int argc, char** argv )
{
  if ( argc < 2 ) 
    {
      cerr << "usage: " << argv[0] << " 8211outfile " <<endl;
      return 1;
    }

#ifdef WIN32
  ofstream ddf( argv[1], ios::binary );
#else
  ofstream ddf( argv[1], ios::out );
#endif

  if ( ! ddf ) 
    {
      cerr << "couldn't open "
           << argv[1] << endl;
      return 2;
    }


  sio_8211Schema iden_schema;
  build_iden_schema( iden_schema );


  cout << "schema: \n";

  ostream_iterator<sio_8211FieldFormat> os_itr( cout, "\n" );

  copy( iden_schema.begin(), iden_schema.end(), os_itr );


  sio_8211Writer  writer( ddf, argv[1], iden_schema );

  if ( ! writer.good() )
    {
      cerr << "writer constructor wedged ...\n";
      exit( 1 );                // something screwed up
    }

  if ( ! writer.emitDDR() )
    {
      cerr << "writer failed to emit DDR ...\n";
      exit( 2 );
    }

  sc_Record record;             // make a fake IDEN module record
  build_iden_record( record );

  cout << "IDEN record:\n" << record << "\n";

  if ( ! writer.put( record ) )
    {
      exit( 3 );
    }
        
  ddf.close();

  exit( 0 );
}

// 
// $Id: sb_iden_t.cpp,v 1.2 2002/10/10 20:35:25 mcoletti Exp $
//


#include <iostream>
#include <fstream>
using namespace std;

#include <sdts++/builder/sb_Iden.h>
#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>


// binary 32 bit integer converter function
sio_8211Converter_BI32  bi32_converter;



const char* iden_str = "Standard Idenification";
const char* version = "Standard Version";
const char* doc_ref = "Document Reference";
const char* profile_iden = "Profile";
const char* profile_version = "Profile Version";
const char* profile_reference = "Profile Reference";
const char* title = "A Title";
const char* data_id = "Data ID";
const char* data_struct = "Data Structure";
const char* map_date = "Map Date";
const char* data_date = "Data Creation Date";
const long scale = 1234567;
const char* comment = "no";
const char* composites = "Y";
const char* vector_geometry = "Y";
const char* vector_topology = "N";
const char* raster = "N";
const long extern_ref = 1;
const long features_level = 3;
const long coding_level = 1;
const char* non_geo_dimension = "N";
const char* module_name = "FOON";
const long record_id = 23;



int
main( int argc, char** argv )
{
  if ( ! argv[1] ) 
    {
      cerr << "usage: "
           << argv[0] << " 8211file " << endl;
      return 1;
    }

#ifdef WIN32
ifstream ddf( argv[1], ios::binary );
#else
ifstream ddf( argv[1] );
#endif

  if ( ! ddf ) 
    {
      cerr << "couldn't open "
           << argv[1] << endl;
      return 2;
    }

  sio_8211Reader  reader( ddf );

  sc_Record record;

  sio_8211ForwardIterator i( reader );


  while( i )
    {
      i.get( record );

      cout << record;
      cout << "\n***\n";

      {
        sb_Iden aIden( record );

        //dumpSbObject( aIden );
        cout << aIden << "\n";
      }
      ++i;
    }

  sb_Iden iden;

  iden.setStandardIdentification( iden_str );
  iden.setStandardVersion( version );
  iden.setStandardDocumentationReference( doc_ref );
  iden.setProfileIdentification( profile_iden );
  iden.setProfileVersion( profile_version  );
  iden.setProfileDocumentationReference( profile_reference );
  iden.setTitle( title );
  iden.setDataID( data_id );
  iden.setDataStructure( data_struct );
  iden.setMapDate( map_date );
  iden.setDataSetCreationDate( data_date );
  iden.setScale( scale );
  iden.setComment( comment );

  iden.setComposites( composites );
  iden.setVectorGeometry( vector_geometry );
  iden.setVectorTopology( vector_topology );
  iden.setRaster( raster );
  iden.setExternalSpatialReference( extern_ref );
  iden.setFeaturesLevel( features_level );
  iden.setCodingLevel( coding_level );
  iden.setNonGeoSpatialDimensions( non_geo_dimension );
  iden.setMnemonic( module_name );
  iden.setID( record_id );

  cout << iden << endl;
      
  // ident.setAttributeID( sb_ForeignID const & );

  if ( ! iden.getRecord( record ) )
    {
      cerr << "unable to get record from ident object\n";
      exit(1);
    }

  cout << record;

  exit( 0 );
}

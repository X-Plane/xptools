#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <pair.h>

using namespace std;

#include <sysutils/fileutils.h>

#include <sdts++/io/sio_Reader.h>

#include <sdts++/builder/sb_Utils.h>
#include <sdts++/builder/sb_Accessor.h>
#include <sdts++/builder/sb_Catd.h>
#include <sdts++/builder/sb_Cell.h>
#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Rsdf.h>


typedef pair<double,double> coordinate_t;


void
error( string const & message )
{
   cerr << message << "\n";

   exit( 1 );
} // error





int
main( int argc, char** argv )
{

   // first get the spatial address of top left corner

   coordinate_t top_left_corner;

   sb_Accessor accessor( argv[1] );

   sb_Rsdf rsdf_record;

   if ( ! accessor.get( rsdf_record ) )
   {
      error( "unable to read RSDF module" );
   }

   rsdf_record.getSADR( top_left_corner.first,
                        top_left_corner.second );

//     cout << "top left corner: "
//          << top_left_corner.first << " "
//          << top_left_corner.second << "\n";

   // then get the distance between postings in X and Y directions


   sb_Iref iref_record;

   if ( ! accessor.get( iref_record ) )
   {
      error( "unable to read IREF module" );
   }

   double x_resolution, y_resolution;

   iref_record.getXComponentHorizontalResolution( x_resolution );
   iref_record.getYComponentHorizontalResolution( y_resolution );

//     cout << "x resolution: " << x_resolution
//          << " y resolution: " << y_resolution
//          << endl;


   // finally, crack open the CELL module and start blatting coordinates


                                // get binary converters for
                                // CELL module

   sio_8211_converter_dictionary converters;

   if ( ! sb_Utils::addConverters( argv[1], converters ) )
   {
      error( "unable to read CATD module " + string(argv[1]) );
   }

   sb_Directory directory( argv[1] );

   sb_Catd catd_record;

                                // generally DEMs only have a single
                                // CELL module; the naming convention
                                // SHOULD be "CEL0", but there might
                                // be weird ones that use "CELL"

   if ( directory.find( "CEL0", catd_record ) )
   {
   }
   else if ( directory.find( "CELL", catd_record ) )
   {
   }
   else
   {
      error( "unable to find a CELL module" );
   }

                                // open up the CELL module

   string cell_filename;

   catd_record.getFILE( cell_filename );

   cell_filename.insert( 0, fileutils::dirname( argv[1] ) + "/" );

   ifstream cell_stream( cell_filename.c_str() );

   if ( ! cell_stream )
   {
      error( "unable to open cell module " + cell_filename  );
   }

   sc_Record raw_cell_record;   // CELL module record before interpretation
                                // via sb_Catd object


   sio_8211Reader cell_reader( cell_stream, &converters );
   sio_8211ForwardIterator i( cell_reader );

   sb_Cell< deque<int> > cell_record;

   deque<int> elevations;

   back_insert_iterator< deque<int> > elevation_inserter( elevations );


   size_t current_row = 0;

   // cout.setf( ios::fixed );
   cout.precision( 8 );

   while ( i )
   {

      i.get( raw_cell_record );

      if ( ! cell_record.setRecord( raw_cell_record ) )
      {
         error( "unable to read CELL module" );
      }

      if ( ! cell_record.loadData( elevation_inserter ) )
      {
         error( "unable to get elevations" );
      }


      for ( size_t j = 0;
            j < elevations.size();
            j++ )
      {
                                // don't worry about printing
                                // void/null data; USGS DEMs use
                                // -32XXX values to flag for this kind
                                // of data

         if ( elevations[j] > -32000.0 )
         {
            cout << top_left_corner.first + (j * x_resolution) << " "
                 << top_left_corner.second + (current_row * y_resolution) << " "
                 << elevations[j]
                 << endl;
         }
      }

      elevations.clear();       // if we don't clear this, then all the
                                // subsequent rows will append their
                                // data to the current row's

      ++current_row;
      ++i;
   }

   exit( 0 );

}

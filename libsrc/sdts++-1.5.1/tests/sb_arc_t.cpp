//
// $Id: sb_arc_t.cpp,v 1.2 2002/10/11 19:55:50 mcoletti Exp $
//

#include <iostream>
#include <fstream>

using namespace std;


// binary 32 bit integer converter function
//sio_8211Converter_BI32  bi32_converter;


int
main( int argc, char** argv )
{

  cout << "this module isn't supported by sdts++ yet\n";

#ifdef NOP
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
      cerr << "couldn't open " << argv[1] << endl;
      return 2;
    }

#endif

  exit( 0 );
}

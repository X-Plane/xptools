//
// $Id: t_dirname.cpp,v 1.2 2002/11/27 00:21:34 mcoletti Exp $
//

#include <iostream>

#include "fileutils.h"

using namespace std;

int
main( int argc, char** argv )
{
   if ( argv[1] )
      cout << fileutils::dirname( argv[1] ) << "\n"; 
   else
      cout << fileutils::dirname( string("") ) << "\n"; 

   exit( 0 );
}

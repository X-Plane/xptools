//
// $Id: t_stringutils.cpp,v 1.1 2001/07/17 20:50:15 mcoletti Exp $
//

#include <iostream>
#include <string>
#include <cassert>

using namespace std;

#include "stringutils.h"


int
main( int argc, char** argv )
{
   {
      string s = "012345     ";    // string with 5 trailing spaces
      string t = s;

      assert( 5 == stringutils::chomp( s ) );

      cout << "[" << s << "]\n";

      assert( 5 == t.size() - s.size() );
   }

   {
      string s = "     ";    // string of 5 spaces
      string t = s;

      assert( 5 == stringutils::chomp( s ) );

      assert( 5 == t.size() - s.size() );
   }

   {
      string s("");

      assert( 0 == stringutils::chomp( s ) );
   }


   exit( 0 );
}

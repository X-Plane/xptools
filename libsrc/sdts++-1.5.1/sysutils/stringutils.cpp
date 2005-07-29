//
// $Id: stringutils.cpp,v 1.1 2001/07/17 20:50:15 mcoletti Exp $
//
//

#include "stringutils.h"


using namespace std;


size_t
stringutils::chomp( string & s )
{
   string::size_type last_pos = s.find_last_not_of( " " );

   string::size_type size = s.size();

   if ( string::npos == last_pos )
   {
      // Either the string was empty or it's comprised of all spaces;
      // if the latter, resize it to null; if the former return zero.

      if ( size )
      {
         s = "";

         return size;           // we've reduced by size spaces, so return that
      }
      else
      {
         return 0;              // empty string, so return zero
      }
   }
   else
   {
      s.resize( last_pos + 1 ); // resize to chop out white space
   } 

   return size - s.size();

} // stringutils::chomp( string & s )


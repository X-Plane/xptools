//
// filename.cc
//

#include "filename.h"

#include <algorithm>
#include <iostream>
#include <cctype>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

static const char* _ident = "$Id: filename.cpp,v 1.1 1999/02/11 22:08:44 mcoletti Exp $";


extern bool verbose;
extern bool force_lowercase;


inline
static
string::value_type
_lowercase( string::value_type c ) { return tolower( c ); }


string
filename( string const & fn )
{
   string tmp_str( fn );

   if ( force_lowercase )
   {
      transform( fn.begin(), fn.end(), tmp_str.begin(), _lowercase );
   }

   return tmp_str;
} // filename


#ifndef HAVE_BASENAME

const char *
basename( const char * s )
{
   register char *base;

   base = strrchr( s, '/' );

   if ( base ) return base + 1;

   return s;
} // basename



// returns true if the path in the given string denotes a directory;
// false if it's a file or something else
static
bool
_isDir( string const & s )
{
   struct stat file_stat;

   stat( s.c_str(), &file_stat );

   if ( file_stat.st_mode & S_IFDIR )
   {
      return true;
   }

   return false;
} // _isDir



string
dirname( string const&  s )
{
   if ( ! s.size() )            // if we get a null path, return "."
   {
      return string(".");
   }


   // Search for the last directory component in the path; but in
   // "/tmp/foo", "foo" might be a directory or a file.  If a file,
   // then return "/tmp", otherwise return "/tmp/foo/".

   string::size_type last_pos = s.find_last_of( "/" );

   if ( last_pos )           // if we found a '/'
   {
      if (  _isDir( s.substr( last_pos ) )  )
      {
                                // return the root directory if that's
                                // the only "/" found; otherwise
                                // return the next directory up

         if ( 1 == last_pos ) { return string("/"); }

         return s.substr( 0, last_pos );
      }

   } 
   else                         // no '/' in path, so ...
   {
      if ( _isDir( s ) )        // if it's a directory, return that
      {
         return s;
      }

      return string("..");      // otherwise, it's a file, and so return
                                // reference to parent directory
   }
   
} // dirname


#endif

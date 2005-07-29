//
// $Id: fileutils.cpp,v 1.3 2001/03/25 21:54:11 mcoletti Exp $
//


#include "fileutils.h"


using namespace std;

/**
 find the last character -- if it's a '/', remove it be shrinking
 ``path'' by one
*/
static
void
removeEndSlashes( string & path )
{
   // find the last non-space character (because some paths may be
   // malformed in that they may have trailing spaces for some goofy
   // reason)
   string::size_type last_pos = path.find_last_not_of( " " );

   // if it's a '/' shrink ``path'' by one to remove it
   if ( string::npos != last_pos )
   {
      if ( path[last_pos] == '/' )
      { path.resize( path.size() - 1 ); }
   } 

} // removeEndSlashes( string & path )


string
fileutils::dirname( string const & original_path )
{
   // we want our own copy to mangle
   string path( original_path );

   if ( ! path.size() )         // if we get a null path, return "."
   {
      return string(".");
   }

   removeEndSlashes( path );

   string::size_type last_pos = path.find_last_of( "/" );

   if ( string::npos != last_pos ) // if we found a '/'
   {
      return path.substr( 0, last_pos );
   } 

   return string( "." );        // no '/' in path, so just return $CWD

} // fileutils::dirname



string
fileutils::basename( string const & original_path )
{
   // we want our own copy to mangle
   string path( original_path );

   if ( ! path.size() )         // if we get a null path, return ""
   {
      return string( "" );
   }

   removeEndSlashes( path );

   string::size_type last_pos = path.find_last_of( "/" );

   if ( string::npos != last_pos ) // if we found a '/'
   {
      return path.substr( last_pos + 1, path.size()  );
   } 

   return path;                 // no '/' in path, so just return original

} // fileutils::basename

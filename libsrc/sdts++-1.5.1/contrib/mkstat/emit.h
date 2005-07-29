//
// emit.h
//
// $Id: emit.h,v 1.2 2003/02/13 23:37:46 mcoletti Exp $
//

#ifndef EMIT_H
#define EMIT_H

#include <string>
#include <iostream>

using namespace std;

class sb_Module;
class sio_8211Writer;


// This function will take the state of the given builder module and
// emit it as a record to the given writer.
void emit( sb_Module const & module, sio_8211Writer& writer );



//
// emits a valid sed statement for converting the first string into the second
// into the given output stream
//
template <class T>
void
emit_sed( string const& orig_str, T const& val, ostream& os )
{
  os << "s/" << orig_str << "/" << val << "/g\n";

} // emit_sed



//
// This does what you'd expect.
//
inline
string
clipTrailingSpaces(string const& str)
{
  string tmp_str( str );

  string::size_type pos = tmp_str.find_last_not_of( " " );

  if ( string::npos != pos ) { tmp_str.resize( pos + 1 ); }

  return tmp_str;
} // clipTrailingSpaces



#endif

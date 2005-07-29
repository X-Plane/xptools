//
// filename.h
//
// $Id: filename.h,v 1.2 2003/02/13 23:37:46 mcoletti Exp $
//

#include <string>

using namespace std;

// takes the given file name and converts it to all lower depending on
// what the global bool force_lowercase is set to

string filename( string const & fn );

#ifndef HAVE_BASENAME
// UNIX basename for those systems that lack it.
const char * basename(const char *);

// UNIX dirname for those systems that lack it.
string dirname( string const & );

#endif

//
// sio_Error_t.cpp
//

#include <cassert>

#include <iostream>

#include <sdts++/io/sio_Error.h>


int
main( int argc, char** argv )
{
   sio_Error error;

   assert( error.good() );

   assert( error.setstate( sio_Error::badbit ) );

   assert( error.bad() );

   assert( error.fail() );

   assert( ! error.clear() );

   assert( error.good() );

   assert( error.setstate( sio_Error::failbit ) );

   assert( error.fail() );

   return 0;
}

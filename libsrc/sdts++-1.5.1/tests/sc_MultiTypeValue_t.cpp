//
// sc_MultiTypeValue.cpp
//

#include <cassert>

#ifdef HAVE_ISO_HEADERS
#include <iostream>
#else
#include <iostream>
#endif



using namespace std;



#include <sdts++/container/sc_MultiTypeValue.h>


int
main( int argc, char** argv )
{

  sc_MultiTypeValue mtv;

  long          long_val;
  unsigned long ulong_val;
  string        string_val;

  // test initial state as being null

  assert( ! mtv.getLong( long_val ) );


  // test set and get

  mtv.setLong( 42 );

  assert( mtv.getLong( long_val ) );

  assert( 42 == long_val );


  // test pedantic get (i.e., trying to get a value of the wrong type

                                // XXX This function has changed
                                // behaviour; now this returns true if
                                // the multivalue type is a signed
                                // integer.  It will convert to an
                                // unsigned and return that value if
                                // this is the case.  I had to do this
                                // because the sc_Subfield::is_I is
                                // blind to signed and unsigned types.
                                // There was no way a priori to know
                                // what type I was dealing with.

  assert( mtv.getUnsignedLong( ulong_val ) );


  // test setting to different type and getting a value of that type

  mtv.setUnsignedLong( 24 );

  assert( mtv.getUnsignedLong( ulong_val ) );

  assert( 24 == ulong_val );


  // string testing

  mtv.setString( "FOO" );

  assert( mtv.getString( string_val ) );

  assert( "FOO" == string_val );

  assert( ! mtv.getUnsignedLong( ulong_val ) );	// can't get long from string


  // copy ctor test

  {
    sc_MultiTypeValue local_mtv( mtv );
    string my_val;

    assert( local_mtv.getString( my_val ) );

    assert( "FOO" == my_val );

    assert( ! local_mtv.getUnsignedLong( ulong_val ) );
  }


  // assignment operator test

  {
    sc_MultiTypeValue local_mtv;
    string my_val;

    local_mtv = mtv;

    assert( local_mtv.getString( my_val ) );

    assert( "FOO" == my_val );

    assert( ! local_mtv.getUnsignedLong( ulong_val ) );
  }


  // null reset test

  mtv.setNull();

  assert( ! mtv.getString( string_val ) );


  // done!

  exit( 0 );
}

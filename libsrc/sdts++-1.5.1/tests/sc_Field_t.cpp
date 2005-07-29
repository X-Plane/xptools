//
// sc_Field_t.cpp
//
// $Id: sc_Field_t.cpp,v 1.6 2002/11/27 00:21:34 mcoletti Exp $
//
// TODO:
//
//   We should add checks for _all_ get*() and set*() members, not just a
//   sampling.
//

#include <cassert>
#include <iostream>


using namespace std;


#include <sdts++/container/sc_Field.h>


int
main( int argc, char** argv )
{

  sc_Field f;

  unsigned long ulong_val;
  string        string_val;


				// name and mnemonic should start out null

  assert( "" == f.getName() );
  assert( "" == f.getMnemonic() );

  f.setName( "Blah blah" );
  f.setMnemonic( "FOO" );

  assert( "Blah blah" == f.getName() );
  assert( "FOO" == f.getMnemonic() );

  assert( f.empty() );		// also should start out with no subfields


				// insert a subfield and insure it behaves ok

  f.push_back( sc_Subfield() );

  assert( ! f.empty() );


  f.back().setA( "BAZ" );

  assert( f.back().getA( string_val ) );

  assert( "BAZ" == string_val );



  // copy ctor test

  {
    sc_Field local_f( f );
    string my_val;

    assert( local_f.back().getA( my_val ) );

    assert( "BAZ" == my_val );

    assert( ! local_f.back().getBUI8( ulong_val ) );
  }


  // assignment operator test

  {
    sc_Field local_f;
    string my_val;

    local_f = f;

    assert( local_f.back().getA( my_val ) );

    assert( "BAZ" == my_val );

    assert( ! local_f.back().getBUI8( ulong_val ) );
  }


  // null reset test

  f.back().setUnvalued();

  assert( ! f.back().getA( string_val ) );


  f.clear();

  assert( f.empty() );

  exit( 0 );
}

//
// sc_Subfield_t.cpp
//
// $Id: sc_Subfield_t.cpp,v 1.10 2003/06/10 17:50:56 mcoletti Exp $
//

#include <cassert>
#include <iostream>

#include <cmath>
#include <climits>

#ifdef _MSC_VER
#define HAVE_FLOAT_H
#endif

#ifdef HAVE_FLOAT_H
#include <float.h>
#endif


using namespace std;



#include <sdts++/container/sc_Subfield.h>


const double MIN_DBL_DISTANCE = DBL_MIN * 2;
const double MIN_FLT_DISTANCE = FLT_MIN * 2;



bool
equal( double const& l, double const& r )
{
   double d = fabs( l - r );

                                // if the difference is less than
                                // twice the smallest possible double
                                // value, then the two values are
                                // close enough to be equal
   return  d < MIN_DBL_DISTANCE;
} // equal



bool
equal( float const& l, float const& r )
{
   double d = fabs( static_cast<double>(l) -
                    static_cast<double>(r) );

   float  f = static_cast<float>(d);

                                // if the difference is less than
                                // twice the smallest possible double
                                // value, then the two values are
                                // close enough to be equal
   return  f < MIN_FLT_DISTANCE;
} // equal



int
main( int argc, char** argv )
{

  sc_Subfield sf;

  int           int_val;
  long          long_val;
  unsigned long ulong_val;
  float         float_val;
  double        double_val;
  string        name = "TEST";

  // test initial state as being null

  assert( ! sf.getI( long_val ) );


  // name and mnemonic tests

  assert( sf.setName( name )     == name );
  assert( sf.setMnemonic( name ) == name );


  // first test the "I" type set and get

  sf.setI( 42 );

  assert( sf.getI( long_val ) );

  assert( 42 == long_val );

  assert( sc_Subfield::is_I == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );

  // this should fail as the subfield is now "locked" as an "I" type

  assert( ! sf.getBUI8( ulong_val ) );


  // now test BUI*

  sf.setBUI8( 24 );

  assert( sf.getBUI8( ulong_val ) );

  assert( 24 == ulong_val );

  assert( sc_Subfield::is_BUI8 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBUI16( 24 );

  assert( sf.getBUI16( ulong_val ) );

  assert( 24 == ulong_val );

  assert( sc_Subfield::is_BUI16 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBUI24( 24 );

  assert( sf.getBUI24( ulong_val ) );

  assert( 24 == ulong_val );

  assert( sc_Subfield::is_BUI24 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBUI32( 24 );

  assert( sf.getBUI32( ulong_val ) );

  assert( 24 == ulong_val );

  assert( sc_Subfield::is_BUI32 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  // now test BI*


  sf.setBI8( -99 );

  assert( sf.getBI8( long_val ) );

  assert( -99 == long_val );

  assert( sc_Subfield::is_BI8 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBI16( -99 );

  assert( sf.getBI16( long_val ) );

  assert( -99 == long_val );

  assert( sc_Subfield::is_BI16 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBI24( -99 );

  assert( sf.getBI24( long_val ) );

  assert( -99 == long_val );

  assert( sc_Subfield::is_BI24 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBI32( -99 );

  assert( sf.getBI32( long_val ) );

  assert( -99 == long_val );

  assert( sc_Subfield::is_BI32 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );





  // now test BFP*


  sf.setBFP32( 123.45 );

  assert( sf.getBFP32( float_val ) );

  assert(  equal( static_cast<float>(123.45), float_val )  );

  assert( sc_Subfield::is_BFP32 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );


  sf.setBFP64( 123.45 );

  assert( sf.getBFP64( double_val ) );

  assert(  equal( 123.45, double_val )  );

  assert( sc_Subfield::is_BFP64 == sf.getSubfieldType() );

  assert( sf.getInt( int_val ) );

  assert( sf.getFloat( float_val ) );

  assert( sf.getDouble( double_val ) );




  // string testing

  string tmp_string;

  sf.setA( "FOO" );

  assert( sf.getA( tmp_string ) );

  assert( "FOO" == tmp_string );

  assert( ! sf.getBUI8( ulong_val ) );	// can't get long from string

  assert( ! sf.getInt( int_val ) );

  assert( ! sf.getFloat( float_val ) );

  assert( ! sf.getDouble( double_val ) );


  // copy ctor test

  {
    sc_Subfield local_sf( sf );
    string my_val;

    assert( local_sf.getA( my_val ) );

    assert( "FOO" == my_val );

    assert( ! local_sf.getBUI8( ulong_val ) );

    assert( sf.getName(  )     == name );
    assert( sf.getMnemonic(  ) == name );

  }


  // assignment operator test

  {
    sc_Subfield local_sf;
    string my_val;

    local_sf = sf;

    assert( local_sf.getA( my_val ) );

    assert( "FOO" == my_val );

    assert( ! local_sf.getBUI8( ulong_val ) );

    assert( sf.getName(  )     == name );
    assert( sf.getMnemonic(  ) == name );

  }


  // null reset test

  sf.setUnvalued();

  assert( ! sf.getA( tmp_string ) );


  // done!

  exit( 0 );
}

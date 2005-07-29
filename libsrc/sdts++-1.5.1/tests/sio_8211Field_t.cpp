//
// sio_8211Field_t.cpp
//
// $Id: sio_8211Field_t.cpp,v 1.7 2002/11/27 00:21:34 mcoletti Exp $
//

#include <cassert>
#include <memory.h>

#include <iostream>
#include <string>
#include <algorithm>


using namespace std;


#include <sdts++/io/sio_8211DirEntry.h>
#include <sdts++/io/sio_8211Field.h>
#include <sdts++/io/sio_8211DDRLeader.h>
#include <sdts++/io/sio_8211Utils.h>
#include <sdts++/io/sio_Buffer.h>

int 
main( int argc, char** argv )
{

  {
    sio_8211Field field;

    // by default, a field should have no data, which naturally
    // has no length
    assert( 0 == field.getData().size() );
    assert( 0 == field.getDataLength() );

  }

  {
    // Now create a field that has a stated data length of 25
    long field_length = 25;
    sio_8211Field field( field_length );

    // Now insure that there isn't any data (because we didn't
    // add any data.
    assert( 0 == field.getData().size() );
    assert( 25 == field.getDataLength() );

    // Now copy over this field to make sure that its state
    // makes it over fine.   
    sio_8211Field another_field( field );

    assert( 0 == another_field.getData().size() );
    assert( 25 == another_field.getDataLength() );

  }
   
  { 
    // Now add bogus subfield data to the field

    sio_8211Field field;

    vector<char> field_data;
    field_data.push_back( 'f' );
    field_data.push_back( 'o' );
    field_data.push_back( 'o' );
    field_data.push_back( sio_8211UnitTerminator );

    field.setData( field_data );

    // Now try and get it back -- they should be the same		
    vector<char> subfield_value;
    long position = 0;

    assert( true == field.getVariableSubfield( subfield_value, position) );
    assert( (*mismatch( subfield_value.begin(), subfield_value.end(), 
                      field_data.begin()).second) == 
            sio_8211UnitTerminator );

    // the position should be incremented to the 'start' of the next subfield
    assert( 4 == position );
   
  }

  {
    //  Test the ctor that takes a buffer argument.

    string buffData( "ABCDEFG" );
    buffData += sio_8211UnitTerminator;

    vector<char> raw_data( buffData.size() );

    copy( buffData.begin(), buffData.end(), raw_data.begin() );

    sio_Buffer buffer( raw_data );
    sio_8211Field field( buffer );
		
    //  Now lets see if things match
    vector<char>const& buffDataOut = field.getData();
    long buffLengthOut = field.getDataLength();

    assert( equal( buffDataOut.begin(), buffDataOut.end(), 
                   buffData.begin() ));

    assert( buffLengthOut == buffData.length() );
	
  }

  {
    // test getField
		
    sio_8211Field field;

    string data = "ABCDEFGH";

    vector<char> raw_data( data.size() );

    copy( data.begin(), data.end(), raw_data.begin() );

    field.setData( raw_data );

    sio_Buffer buffer = field.getField();

    //  get buffer data from new buffer and test

    vector<char> const& buffData = buffer.data();
    long buffLength = buffer.length();
	
    assert( buffLength == data.length() );
    assert( equal( buffData.begin(), buffData.end(), data.begin() ) );
	
  }

  return 0;

}



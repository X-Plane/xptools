//
// sio_8211Buffer_t.cpp
//

#include <cassert>
#include <memory.h>

#include <iostream>



using namespace std;


#include <sdts++/io/sio_Buffer.h>



void
copy_test( sio_Buffer buffer, vector<char> const& data )
{
    // should be identical to the original string
    assert ( equal( data.begin(), data.end(), buffer.data().begin() ) );
}


int
main(int argc, char** argv)
{
                                // create a test data string and then
                                // convert it over to the container
                                // that sio_Buffer expects

    char text[] = "0123456789";
    vector<char> data( strlen(text) );

    copy( &text[0], &text[strlen(text)], data.begin() );

    sio_Buffer buffer;


    assert( buffer.addData( &data[0], data.size() ) );


    // the data in the buffer should be identical to the string we used
    // to add data to it in the first place

    assert ( equal( data.begin(), data.end(), buffer.data().begin() ) );


    // now add more data  
    assert( buffer.addData( text, strlen(text) ) );


    // it should be twice as big
    assert( data.size() * 2 == buffer.length() );

    // and the second half should be identical to the original string
    assert ( equal( buffer.data().begin() + strlen(text), 
                    buffer.data().end(), data.begin() ) );


    // now blow the buffer away
    assert ( buffer.reset() );


    // the length should be zero
    assert ( 0 == buffer.length() );



    // now add data again
    assert( buffer.addData( &data[0], data.size() ) );


    // again, it should be the same as the original text
    assert( data.size() == buffer.length() );


    // and it should be identical to the original string

    assert ( equal( data.begin(), data.end(), buffer.data().begin() ) );


    copy_test( buffer, data );


    return 0;
}

//
//		sio_8211DirEntry_t
//


#include <iostream>
#include <strstream>

#include <cassert>

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

using namespace std;


#include <sdts++/io/sio_8211DirEntry.h>
#include <sdts++/io/sio_8211DRLeader.h>



int main(int argc, char** argv)
{

  sio_8211DirEntry dir_entry;
  sio_8211DRLeader tmp_leader;  // bogus leader to be used by dir entry


  // insure everything initialized sanely

  assert( 0  == dir_entry.getPosition() );
  assert( 0  == dir_entry.getFieldLength() );
  assert( "" == dir_entry.getTag() );
  assert( 0  == dir_entry.getField() );
  assert( 0  == dir_entry.getLeader() );


  // we don't need a real field, we just want to make sure that the
  // address is ok
  dir_entry.setField( reinterpret_cast<sio_8211Field const*>( 0xbad ) );

  // unfortunately we do need a real leader because a side effect of
  // setting the position, field length, and tag is that the leader
  // widts are automagically updated
  dir_entry.setLeader( &tmp_leader );

  dir_entry.setPosition( 10 );
  dir_entry.setFieldLength( 200 );
  dir_entry.setTag( "BOGUS" );


  // natch, we now make sure we get back what we set
  assert( 10      == dir_entry.getPosition() );
  assert( 200     == dir_entry.getFieldLength() );
  assert( "BOGUS" == dir_entry.getTag() );
  assert( reinterpret_cast<sio_8211Field const*>(0xbad) == dir_entry.getField() );
  assert( &tmp_leader == dir_entry.getLeader() );


  // as a side-effect of some of the sets, the leader will have
  // modified field widths, so we need to check that, too

  assert( 2 == tmp_leader.getSizeOfFieldPosField() ); // 10 is 2 characters wide

  assert( 3 == tmp_leader.getSizeOfFieldLengthField() ); // 200 is 3 characters wide

  assert( strlen("BOGUS") == tmp_leader.getSizeOfFieldTagField() );


  {  // exercise copy ctor

    sio_8211DirEntry dir_entry_copy( dir_entry );


    // obviously the values should be the same as what's in the old dir_entry

    assert( 10      == dir_entry.getPosition() );
    assert( 200     == dir_entry.getFieldLength() );
    assert( "BOGUS" == dir_entry.getTag() );
    assert( reinterpret_cast<sio_8211Field const*>(0xbad) == dir_entry.getField() );
    assert( &tmp_leader == dir_entry.getLeader() );

  }


  {  // exercise assignment operator

    sio_8211DirEntry dir_entry_copy;

    dir_entry_copy = dir_entry;


    // obviously the values should be the same as what's in the old dir_entry

    assert( 10      == dir_entry.getPosition() );
    assert( 200     == dir_entry.getFieldLength() );
    assert( "BOGUS" == dir_entry.getTag() );
    assert( reinterpret_cast<sio_8211Field const*>(0xbad) == dir_entry.getField() );
    assert( &tmp_leader == dir_entry.getLeader() );

  }


  {  // test stream operators

    strstream ss;

    ss << dir_entry;


    sio_8211DirEntry dir_entry_copy(tmp_leader); // have to re-use leader for
    // proper formatting information

    ss >> dir_entry_copy;


    // obviously the values should be the same as what's in the old dir_entry

    assert( 10      == dir_entry_copy.getPosition() );
    assert( 200     == dir_entry_copy.getFieldLength() );
    assert( "BOGUS" == dir_entry_copy.getTag() );
    // We don't check the sio_8211Field pointer as that won't get set during
    // a read from an input stream.
    assert( &tmp_leader == dir_entry_copy.getLeader() );

  }

  return 0;
}

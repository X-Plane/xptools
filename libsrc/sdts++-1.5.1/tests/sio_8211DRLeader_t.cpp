//
// sio_8211DDRLeader_t.cpp
//


#include <iostream>
#include <strstream>

#include <cassert>


#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


using namespace std;


#include <sdts++/io/sio_8211DRLeader.h>


int main(int argc, char** argv)
{
	
  sio_8211DRLeader dr_leader;

  // insure that the leader got created with sane values

  assert( 'D' == dr_leader.getLeaderIdentifier() );
  assert( 0   == dr_leader.getRecordLength() );
  assert( 0   == dr_leader.getBaseAddrOfFieldArea() );
  assert( 0   == dr_leader.getSizeOfFieldLengthField() );
  assert( 0   == dr_leader.getSizeOfFieldPosField() );
  assert( 0   == dr_leader.getSizeOfFieldTagField() );

  // check the set functions

  dr_leader.setLeaderIdentifier( 'X' );
  dr_leader.setRecordLength( 1 );
  dr_leader.setBaseAddrOfFieldArea( 2 );
  dr_leader.setSizeOfFieldLengthField( 3 );
  dr_leader.setSizeOfFieldPosField( 4 );
  dr_leader.setSizeOfFieldTagField( 5 );

  assert( 'X' == dr_leader.getLeaderIdentifier() );
  assert( 1   == dr_leader.getRecordLength() );
  assert( 2   == dr_leader.getBaseAddrOfFieldArea() );
  assert( 3   == dr_leader.getSizeOfFieldLengthField() );
  assert( 4   == dr_leader.getSizeOfFieldPosField() );
  assert( 5   == dr_leader.getSizeOfFieldTagField() );


  // check stream I/O functions

  strstream ss;

  ss << dr_leader;


  sio_8211DRLeader second_dr_leader;

  ss >> second_dr_leader;


  // the second DDR leader should be the same as the first

  assert( 'X' == second_dr_leader.getLeaderIdentifier() );
  assert( 1   == second_dr_leader.getRecordLength() );
  assert( 2   == second_dr_leader.getBaseAddrOfFieldArea() );
  assert( 3   == second_dr_leader.getSizeOfFieldLengthField() );
  assert( 4   == second_dr_leader.getSizeOfFieldPosField() );
  assert( 5   == second_dr_leader.getSizeOfFieldTagField() );



  return 0; 
}


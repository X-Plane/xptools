//
// sio_8211DDRLeader_t.cpp
//
// $Id: sio_8211DDRLeader_t.cpp,v 1.5 2002/11/27 00:21:34 mcoletti Exp $
//

#include <iostream>
#include <strstream>

#include <cassert>


#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

using namespace std;


#include <sdts++/io/sio_8211DDRLeader.h>


int main(int argc, char** argv)
{
	
  sio_8211DDRLeader ddr_leader;

  // insure that the leader got created with sane values

  assert( 'L' == ddr_leader.getLeaderIdentifier() );
  assert( 0   == ddr_leader.getRecordLength() );
  assert( 0   == ddr_leader.getBaseAddrOfFieldArea() );
  assert( 0   == ddr_leader.getSizeOfFieldLengthField() );
  assert( 0   == ddr_leader.getSizeOfFieldPosField() );
  assert( 0   == ddr_leader.getSizeOfFieldTagField() );
  assert( 6   == ddr_leader.getFieldControlLength() ); // this is hard-set to 6

  // check the set functions

  ddr_leader.setLeaderIdentifier( 'X' );
  ddr_leader.setRecordLength( 1 );
  ddr_leader.setBaseAddrOfFieldArea( 2 );
  ddr_leader.setSizeOfFieldLengthField( 3 );
  ddr_leader.setSizeOfFieldPosField( 4 );
  ddr_leader.setSizeOfFieldTagField( 5 );

  assert( 'X' == ddr_leader.getLeaderIdentifier() );
  assert( 1   == ddr_leader.getRecordLength() );
  assert( 2   == ddr_leader.getBaseAddrOfFieldArea() );
  assert( 3   == ddr_leader.getSizeOfFieldLengthField() );
  assert( 4   == ddr_leader.getSizeOfFieldPosField() );
  assert( 5   == ddr_leader.getSizeOfFieldTagField() );


  // check stream I/O functions

  strstream ss;

  ss << ddr_leader;


  sio_8211DDRLeader second_ddr_leader;

  ss >> second_ddr_leader;


  // the second DDR leader should be the same as the first

  assert( 'X' == second_ddr_leader.getLeaderIdentifier() );
  assert( 1   == second_ddr_leader.getRecordLength() );
  assert( 2   == second_ddr_leader.getBaseAddrOfFieldArea() );
  assert( 3   == second_ddr_leader.getSizeOfFieldLengthField() );
  assert( 4   == second_ddr_leader.getSizeOfFieldPosField() );
  assert( 5   == second_ddr_leader.getSizeOfFieldTagField() );
  assert( 6   == second_ddr_leader.getFieldControlLength() ); // this is hard-set to 6


  return 0; 
}


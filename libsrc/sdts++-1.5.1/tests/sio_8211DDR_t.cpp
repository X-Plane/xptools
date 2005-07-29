//
// sio_8211DDR_t.cpp
//
// Exercises the DDR class.
//

#ifdef _MSC_VER
#pragma warning(disable:4786)
#include <xutility>  // for min()
#include <utility>   // for min()
#endif

#include <cassert>
#include <algorithm>
#include <functional>

#include <iostream>
#include <strstream>



using namespace std;


#include <sdts++/io/sio_8211DDR.h>
#include <sdts++/io/sio_8211DDRField.h>


// comparison operator for sio_8211DirEntry
class DirEntriesEqual
{
public:

  bool operator()( sio_8211DirEntry const& left, sio_8211DirEntry const& right )
    {
      return left.getFieldLength() == right.getFieldLength() &&
        left.getPosition() == right.getPosition() &&
        left.getTag() == right.getTag();
    }
}; // binary predicate class DirEntriesEqual




// comparison operator for sio_8211Field
//
// Note that since the read in DDR will have the field terminators
// stripped (because we don't bother to store them, the comparisons
// have to be a bit lenient.  That is, we allow for lengths to differ by 
// one character (presumably the missing field terminator), and then do
// a strict binary comparison that doesn't include the field terminator.
//
class FieldsEqual
{
public:

  bool operator()( sio_8211Field const& left, sio_8211Field const& right )
    {
                                // The two 8211 fields should only
                                // differ by an
                                // sio_8211FieldTermintor; this is
                                // because the one on the right was
                                // created for writing, and so should
                                // have the field terminator in its
                                // raw data buffer, and the second is
                                // for reading, which has terminators
                                // implicitly filters upon reading.

        return ( (*mismatch( right.getData().begin(), right.getData().end(),
                             left.getData().begin() ).second) ==
                 sio_8211FieldTerminator );

    }
}; // binary predicate class FieldEqual



int main(int argc, char** argv)
{
  // create a DDR from scratch

  sio_8211DDR first_ddr;

   // This is a handy short-cut for creating a legit DDR

  sio_8211FileTitleField file_title_field("BOGUS TITLE");

  first_ddr.addField( "0001", file_title_field.getField() );

   //  Check DDRLeader to see if it is OK.
  const sio_8211DDRLeader * const first_ddr_leader =
    dynamic_cast<const sio_8211DDRLeader*>(&(first_ddr.getLeader()));

  assert( first_ddr_leader->getFieldControlLength() == 6 );
  assert( first_ddr_leader->getRecordLength() == 0 );
  assert( first_ddr_leader->getLeaderIdentifier() == 'L' );
   
  assert( first_ddr_leader->getSizeOfFieldPosField() == 1 );
  assert( first_ddr_leader->getSizeOfFieldTagField() == 4 );

  // XXX It might be a good idea to check the fieldArea and 
  // XXX directory of the DDR too.  However, it might be overkill

  // Build another DDR via << and >>.  Compare the 2 ddrs.  They
  // should be the same.

  // blat it out to a temporary string stream
  strstream ss;
  ss << first_ddr;

   // and blat it out to a second DDR
  sio_8211DDR second_ddr;
  ss >> second_ddr;

  if ( ! ss ) { exit(1); }      // abort if stream wedged
		
	// get leader from second_ddr
  const sio_8211DDRLeader * const second_ddr_leader =
    dynamic_cast<const sio_8211DDRLeader*>(&(second_ddr.getLeader()));

   // first the leader
                                // insure we have valid leaders

  assert( first_ddr_leader && second_ddr_leader ); 
  assert( first_ddr_leader->getFieldControlLength() == 
          second_ddr_leader->getFieldControlLength() );
  assert( first_ddr_leader->getRecordLength() == 
          second_ddr_leader->getRecordLength() );
  assert( first_ddr_leader->getLeaderIdentifier() == 
          second_ddr_leader->getLeaderIdentifier() );
  assert( first_ddr_leader->getBaseAddrOfFieldArea() == 
          second_ddr_leader->getBaseAddrOfFieldArea() );
  assert( first_ddr_leader->getSizeOfFieldLengthField() == 
          second_ddr_leader->getSizeOfFieldLengthField() );
  assert( first_ddr_leader->getSizeOfFieldPosField() == 
          second_ddr_leader->getSizeOfFieldPosField() );
  assert( first_ddr_leader->getSizeOfFieldTagField() == 
          second_ddr_leader->getSizeOfFieldTagField() );

  // then the directories
  assert( equal( first_ddr.getDirectory().begin(), first_ddr.getDirectory().end(),
                 second_ddr.getDirectory().begin(), DirEntriesEqual() ) );

  // then the field area
  assert( equal( first_ddr.getFieldArea().begin(), first_ddr.getFieldArea().end(),
                 second_ddr.getFieldArea().begin(), FieldsEqual() ) );

  return 0;
}


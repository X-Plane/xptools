//
// sio_8211DR_t.cpp
//
// Exercises the DR class.
//

#include <cassert>
#include <algorithm>
#include <functional>


#include <iostream>
#include <strstream>

#ifdef _MSC_VER
#include <xutility>  // for min()
#pragma warning(disable:4786)
#else
#include <utility>   // for min()
#endif

using namespace std;

#include <sdts++/io/sio_8211DR.h>
#include <sdts++/io/sio_8211Field.h>


// electric fence debugging variables
#ifdef EFENCE
extern int EF_ALIGNMENT;
extern int EF_PROTECT_BELOW;
extern int EF_PROTECT_FREE;
extern int EF_ALLOW_MALLOC_0;
extern int EF_FILL;
#endif



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
      return sio_8211FieldTerminator ==
        (*mismatch( left.getData().begin(), left.getData().end(),
                  right.getData().begin() ).first);
    }
}; // binary predicate class FieldEqual



int
main(int argc, char** argv)
{
#ifdef EFENCE
   EF_PROTECT_FREE = 1;
   EF_PROTECT_BELOW = 1;
   EF_ALIGNMENT = 0;
#endif

  // create a DDR from scratch
  sio_8211DR first_dr, second_dr;
  sio_8211Field first_field;

  string tmp_str = "BOGUS";     // create some bogus test data
  vector<char> tmp_data(5);     // and then copy it over to a data type
                                // palatable to the sio_8211Field
  copy( tmp_str.begin(), tmp_str.end(), tmp_data.begin() );

  tmp_data.push_back( sio_8211FieldTerminator );

  first_field.setData( tmp_data );
  first_dr.addField( "0001", first_field );

  //  Check contents of DRLeader.
  const sio_8211DRLeader * const first_dr_leader =
    dynamic_cast<const sio_8211DRLeader*>(&(first_dr.getLeader()));

  const sio_8211DRLeader * const second_dr_leader =
    dynamic_cast<const sio_8211DRLeader*>(&(second_dr.getLeader()));

  assert( first_dr_leader->getRecordLength() == 0 );
  assert( first_dr_leader->getLeaderIdentifier() == 'D' );
  assert( first_dr_leader->getBaseAddrOfFieldArea() == 0 );
  assert( first_dr_leader->getSizeOfFieldLengthField() == 1 );
  assert( first_dr_leader->getSizeOfFieldPosField() == 1 );
  assert( first_dr_leader->getSizeOfFieldTagField() == 4 );


  //  Now exercise << and >> operators.

  // blat first_dr out to a temporary string stream
  strstream ss;
  ss << first_dr;

// and blat it out to a second DDR
  ss >> second_dr;

  // start comparing the two DDR's; they should both be the same

// first the leader
  assert( first_dr_leader && second_dr_leader ); // insure we have valid leaders

  assert( first_dr_leader->getRecordLength() ==
          second_dr_leader->getRecordLength() );

  assert( first_dr_leader->getLeaderIdentifier() ==
          second_dr_leader->getLeaderIdentifier() );

  assert( first_dr_leader->getBaseAddrOfFieldArea() ==
          second_dr_leader->getBaseAddrOfFieldArea() );

  assert( first_dr_leader->getSizeOfFieldLengthField() ==
          second_dr_leader->getSizeOfFieldLengthField() );

  assert( first_dr_leader->getSizeOfFieldPosField() ==
          second_dr_leader->getSizeOfFieldPosField() );

  assert( first_dr_leader->getSizeOfFieldTagField() ==
          second_dr_leader->getSizeOfFieldTagField() );

  // then the directories
  assert( equal( first_dr.getDirectory().begin(), first_dr.getDirectory().end(),
                 second_dr.getDirectory().begin(), DirEntriesEqual() ) );

  // then the field area
  assert( equal( first_dr.getFieldArea().begin(), first_dr.getFieldArea().end(),
                 second_dr.getFieldArea().begin(), FieldsEqual() ) );


  return 0;
}


//
// $Id: container_t.cpp,v 1.3 2002/11/27 00:21:34 mcoletti Exp $
//

#include <cassert>

#include <string>
#include <iostream>

#include <sdts++/container/sc_Subfield.h>
#include <sdts++/container/sc_Field.h>



using namespace std;


int
main(int argc, char *argv[])
{
  sc_Subfield subfield, copied;
  sc_Field field;
  string tempstring;

  field.push_back(sc_Subfield());
  field.back().setMnemonic("X");
  field.push_back(sc_Subfield());
  field.back().setMnemonic("Y");
  field.push_back(sc_Subfield());
  field.back().setMnemonic("Z");
  field.setMnemonic("PNT");
  field.setName("Point");

  sc_Field::iterator i;

  int inc = 0;

  for ( i = field.begin();
	i != field.end();
	i++ )
  {
//    cout << "Subfield Mnemonic: " << (*i).getMnemonic() << '\n';
    (*i).setBUI32( 123 * inc++ );

//  Test the != operator.
    assert( copied != (*i) );

//  Test the = operator.
    copied = (*i);

//  Test the == operator
    assert( copied == (*i) );
  }

  unsigned long val;
  inc = 0;
  for ( i = field.begin();
	i != field.end();
	i++ )
  {
//  Check to make sure that the data is there.
    assert( (*i).getBUI32(val) );
//  Check to make sure that the data is correct.
    assert( (val / 123) == inc );
    inc++;
  }

//  Check to make sure that some information is being returned.
  tempstring = field.getMnemonic();
  assert( tempstring == "PNT" );
  tempstring = field.getName();
  assert( tempstring == "Point" );

  return 0;
}

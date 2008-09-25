//
// sio_8211FieldFormat_t.cpp
//
//

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


using namespace std;

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include <sdts++/io/sio_8211FieldFormat.h>


int
main(int argc, char** argv)
{
  sio_8211FieldFormat field_format;

  string tag = "ATAG";
  string name = "ANAME";
  sio_8211FieldFormat::data_struct_code dsc = sio_8211FieldFormat::array;
  sio_8211FieldFormat::data_type_code   dtc = sio_8211FieldFormat::bit_string;
  bool repeating_flag = true;

  // set some default values

  field_format.setDataStructCode( dsc );
  field_format.setDataTypeCode( dtc );
  field_format.setTag( tag );
  field_format.setName( name );
  field_format.setIsRepeating( repeating_flag );


  // insure that we get back the values we set

  assert( dsc == field_format.getDataStructCode( ) );
  assert( dtc == field_format.getDataTypeCode( ) );
  assert( tag == field_format.getTag( ) );
  assert( name == field_format.getName( ) );
  assert( repeating_flag == field_format.isRepeating(  ) );


  // exercise copy ctor

  {
      sio_8211FieldFormat ff( field_format );

      assert( dsc == ff.getDataStructCode( ) );
      assert( dtc == ff.getDataTypeCode( ) );
      assert( tag == ff.getTag( ) );
      assert( name == ff.getName( ) );
      assert( repeating_flag == ff.isRepeating(  ) );
  }


  return 0;

}

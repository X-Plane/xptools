//
//		sio_8211DDRField_t.cpp   
//

#include <cassert>
#include <memory.h>

#include <string>
#include <vector>

#include <iostream>


using namespace std;


#include <sdts++/io/sio_8211DDRField.h>
#include <sdts++/io/sio_8211Field.h>
#include <sdts++/io/sio_8211DDRLeader.h>
#include <sdts++/io/sio_Buffer.h>


const char   data_struct_code = '0';
const char   data_type_code   = '1';
const string field_name       = "FIELD NAME";
const string subfield_array   = "SUBFIELD";
const string format_string    = "(2A(10))";



//
// The field's attributes should be the same as the 
// global variables that were used to set it.
// We use a copy of the field to exercise the copy ctor.
//
void
test_field( sio_8211DDRField field )
{
   assert( field.getDataStructCode()   == data_struct_code );
   assert( field.getDataTypeCode()     == data_type_code );
   assert( field.getDataFieldName()    == field_name );
   assert( field.getArrayDescriptor()  == subfield_array );
   assert( field.getFormatControls()   == format_string );

} // test_field



int 
main( int argc, char** argv )
{

   sio_8211DDRField ddr_field;

   // insure that the data field name, array descriptors, and format control
   // strings start out with nothing in them

   assert( ddr_field.getDataFieldName() == "" );
   assert( ddr_field.getArrayDescriptor() == "" );
   assert( ddr_field.getFormatControls() == "" );


   // now set them; test_field insures that you get what you set

   ddr_field.setDataStructCode( data_struct_code );
   ddr_field.setDataTypeCode( data_type_code );
   ddr_field.setDataFieldName( field_name );
   ddr_field.setArrayDescriptor( subfield_array );
   ddr_field.setFormatControls( format_string );

   test_field( ddr_field );


   // now do the same thing, except the DDR field will get its
   // values from a leader and a field

   sio_8211DDRLeader    ddr_leader;
   sio_8211Field        field;

   const string field_data =  
       "0100;&" +
       field_name + sio_8211UnitTerminator + 
       subfield_array + sio_8211UnitTerminator + 
       format_string;

                                // but we use vectors to contain the
                                // data, so we must convert from the
                                // string to a vector<char> container
                                // (yes, I could have just done
                                // everything with the vector, but not
                                // only was the string representation
                                // legacy code, I found it a lot
                                // easier to use it as a convenience
                                // for making a field data area by
                                // hand)

   vector<char> raw_data( field_data.size() ); 

   copy( field_data.begin(), field_data.end(), raw_data.begin() );


   //  add some bogus data to the field
   field.setData( raw_data );

   //  create a DDR field with the given leader and field
   sio_8211DDRField ddr_field2( ddr_leader, field );

   test_field( ddr_field2 );


   // Now check out that the raw sio_Buffer you get from the 
   // DDR field is ok -- it should be in synch with the raw data
   // that was used to create the DDR field in the first place.

   //  Grab a raw buffer of the DDR field contents
   sio_Buffer test_buffer = ddr_field2.getField();

                                // The only difference between the
                                // original raw data and the buffer we
                                // get from the field is that the
                                // buffer should have a field
                                // terminator at its end

   assert( (*mismatch( raw_data.begin(), raw_data.end(),
                       test_buffer.data().begin() ).second) ==
           sio_8211FieldTerminator );


   return 0;

}



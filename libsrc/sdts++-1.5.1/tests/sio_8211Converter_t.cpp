//
// sio_8211Converter_t.cpp
//

#include <cstdlib>
#include <cassert>


#include <sdts++/container/sc_Subfield.h>

#include <sdts++/io/sio_Buffer.h>
#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_8211Utils.h>


using namespace std;


int
main()
{
   // For each converter, start with a subfield value, convert it to a
   // fixed length and then variable length buffers; for each
   // conversion, insure that the converted data matches the original
   // subfield value.

   const string str_data  = "ABCDE";
   const long   long_data = 42;

   sc_Subfield subfield;
   sio_Buffer  buffer;


   sio_8211Converter_A A_converter;

   subfield.setA( str_data );

   A_converter.addSubfield( subfield, buffer );

   assert( equal( buffer.data().begin(), buffer.data().end(),
                  str_data.begin() ) );

   {
      // now remake an entirely new subfield from the data buffer
      // created from the original subfield.  They _should_ be
      // identical.

      sc_Subfield subfield;

      A_converter.makeFixedSubfield( subfield,
                                     &buffer.data()[0],
                                     buffer.length() );

      string s;

      assert( subfield.getA( s ) );

      assert( s == str_data );
   }

   buffer.reset();              // set up for next test by clearing out all old
                                // data




   sio_8211Converter_I I_converter;

   subfield.setI( long_data );

   I_converter.addSubfield( subfield, buffer );

   {
      // now remake an entirely new subfield from the data buffer
      // created from the original subfield.  They _should_ be
      // identical.

      sc_Subfield subfield;

      I_converter.makeFixedSubfield( subfield,
                                     &buffer.data()[0],
                                     buffer.length() );

      long l;

      assert( subfield.getI( l ) );

      assert( long_data == l );

                                // the variable length function
                                // requires a unit terminator to know
                                // when there's no more data

      buffer.addData( sio_8211UnitTerminator );

      I_converter.makeVarSubfield( subfield,
                                   &buffer.data()[0],
                                   buffer.length(),
                                   sio_8211UnitTerminator );

      assert( subfield.getI( l ) );

      assert( long_data == l );
   }

   buffer.reset();



   float float_data = 123.45789;

   sio_8211Converter_BFP32 BFP32_converter;


   subfield.setBFP32( float_data );

   BFP32_converter.addSubfield( subfield, buffer );

   {
      // now remake an entirely new subfield from the data buffer
      // created from the original subfield.  They _should_ be
      // identical.

      sc_Subfield subfield;

      BFP32_converter.makeFixedSubfield( subfield,
                                         &buffer.data()[0],
                                         buffer.length() * 8 );

                                // BFP32 requires field length to be
                                // in bits not bytes, which is why we
                                // multiply by eight

      float f;

      assert( subfield.getBFP32( f ) );

      assert( float_data == f );

   }





   exit( 0 );
}

/**

$Id: sio_ConverterFactory_t.cpp,v 1.1 2001/02/07 19:12:46 mcoletti Exp $

*/

#include <iostream>
#include <cassert>

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_ConverterFactory.h>

int
main( int argc, char** argv )
{

   assert( dynamic_cast<sio_8211Converter_A *>
           (sio_ConverterFactory::instance()->get( "A" )) );

   assert( dynamic_cast<sio_8211Converter_I *>
           (sio_ConverterFactory::instance()->get( "I" ) ) );

   assert( dynamic_cast<sio_8211Converter_R *>
           (sio_ConverterFactory::instance()->get( "R" ) ) );

   assert( dynamic_cast<sio_8211Converter_S *>
           (sio_ConverterFactory::instance()->get( "S" ) ) );

   // this should fail since this converter type isn't supported by sdts++
   assert( ! sio_ConverterFactory::instance()->get( "C" ) );


   assert( dynamic_cast<sio_8211Converter_BI8 *>
           (sio_ConverterFactory::instance()->get( "BI8" ) ) );

   assert( dynamic_cast<sio_8211Converter_BI16 *>
           (sio_ConverterFactory::instance()->get( "BI16" ) ) );

   assert( dynamic_cast<sio_8211Converter_BI24 *>
           (sio_ConverterFactory::instance()->get( "BI24" ) ) );

   assert( dynamic_cast<sio_8211Converter_BI32 *>
           (sio_ConverterFactory::instance()->get( "BI32" ) ) );


   assert( dynamic_cast<sio_8211Converter_BUI8 *>
           (sio_ConverterFactory::instance()->get( "BUI8" ) ) );

   assert( dynamic_cast<sio_8211Converter_BUI16 *>
           (sio_ConverterFactory::instance()->get( "BUI16" ) ) );

   assert( dynamic_cast<sio_8211Converter_BUI24 *>
           (sio_ConverterFactory::instance()->get( "BUI24" ) ) );

   assert( dynamic_cast<sio_8211Converter_BUI32 *>
           (sio_ConverterFactory::instance()->get( "BUI32" ) ) );


   assert( dynamic_cast<sio_8211Converter_BFP32 *>
           (sio_ConverterFactory::instance()->get( "BFP32" ) ) );

   assert( dynamic_cast<sio_8211Converter_BFP64 *>
           (sio_ConverterFactory::instance()->get( "BFP64" ) ) );


   // and now intentionally try to get a bogus converter


   assert( ! sio_ConverterFactory::instance()->get( "BOGUS" ) );


   exit( 0 );
}

//
// This file is part of the SDTS++ toolkit, written by the U.S.
// Geological Survey.  It is experimental software, written to support
// USGS research and cartographic data production.
//
// SDTS++ is public domain software.  It may be freely copied,
// distributed, and modified.  The USGS welcomes user feedback, but makes
// no committment to any level of support for this code.  See the SDTS
// web site at http://mcmcweb.er.usgs.gov/sdts for more information,
// including points of contact.
//

#include "sio_ConverterFactory.h"

#include <cctype>

#include "io/sio_8211Converter.h"


using namespace std;


/// canonical instance of sio_ConverterFactory
std::auto_ptr<sio_ConverterFactory> sio_ConverterFactory::instance_;



struct ConverterFactory_Imp
{
      auto_ptr<sio_8211Converter_A     > converterA;
      auto_ptr<sio_8211Converter_I     > converterI;
      auto_ptr<sio_8211Converter_R     > converterR;
      auto_ptr<sio_8211Converter_S     > converterS;
      auto_ptr<sio_8211Converter_C     > converterC;
      auto_ptr<sio_8211Converter_BI8   > converterBI8;
      auto_ptr<sio_8211Converter_BI16  > converterBI16;
      auto_ptr<sio_8211Converter_BI24  > converterBI24;
      auto_ptr<sio_8211Converter_BI32  > converterBI32;
      auto_ptr<sio_8211Converter_BUI8  > converterBUI8;
      auto_ptr<sio_8211Converter_BUI16 > converterBUI16;
      auto_ptr<sio_8211Converter_BUI24 > converterBUI24;
      auto_ptr<sio_8211Converter_BUI32 > converterBUI32;
      auto_ptr<sio_8211Converter_BFP32 > converterBFP32;
      auto_ptr<sio_8211Converter_BFP64 > converterBFP64;

      ConverterFactory_Imp()
         : converterA( 0x0 ),
           converterI( 0x0 ),
           converterR( 0x0 ),
           converterS( 0x0 ),
           converterC( 0x0 ),
           converterBI8( 0x0 ),
           converterBI16( 0x0 ),
           converterBI24( 0x0 ),
           converterBI32( 0x0 ),
           converterBUI8( 0x0 ),
           converterBUI16( 0x0 ),
           converterBUI24( 0x0 ),
           converterBUI32( 0x0 ),
           converterBFP32( 0x0 ),
           converterBFP64( 0x0 )
      {}

      sio_8211Converter_A * getA()
      {
         if ( ! converterA.get() )
         {
            // for g++ we can't do this in one step as it can't find
            // the assignment operator; I'd use reassign() but VC++
            // doesn't have it.  So, I'm stuck with this ugliness.
            auto_ptr<sio_8211Converter_A> foo(new sio_8211Converter_A );
            converterA = foo;
         }
         return converterA.get();
      } // getA


      sio_8211Converter_I * getI()
      {
         if ( ! converterI.get() )
         {
            auto_ptr<sio_8211Converter_I> foo(new sio_8211Converter_I );
            converterI = foo;
         }
         return converterI.get();
      } // getI


      sio_8211Converter_R * getR()
      {
         if ( ! converterR.get() )
         {
            auto_ptr<sio_8211Converter_R> foo( new sio_8211Converter_R );
            converterR = foo;
         }
         return converterR.get();
      } // getA


      sio_8211Converter_S * getS()
      {
         if ( ! converterS.get() )
         {
            auto_ptr<sio_8211Converter_S> foo( new sio_8211Converter_S );
            converterS = foo;
         }
         return converterS.get();
      } // getS


      sio_8211Converter_C * getC()
      {
         if ( ! converterC.get() )
         {
            auto_ptr<sio_8211Converter_C> foo( new sio_8211Converter_C );
            converterC = foo;
         }
         return converterC.get();
      } // getC


      sio_8211Converter_BI8 * getBI8()
      {
         if ( ! converterBI8.get() )
         {
            auto_ptr<sio_8211Converter_BI8> foo( new sio_8211Converter_BI8 );
            converterBI8 = foo;
         }
         return converterBI8.get();
      } // getBI8


      sio_8211Converter_BI16 * getBI16()
      {
         if ( ! converterBI16.get() )
         {
            auto_ptr<sio_8211Converter_BI16> foo( new sio_8211Converter_BI16 );
            converterBI16 = foo;
         }
         return converterBI16.get();
      } // getBI16


      sio_8211Converter_BI24 * getBI24()
      {
         if ( ! converterBI24.get() )
         {
            auto_ptr<sio_8211Converter_BI24> foo( new sio_8211Converter_BI24 );
            converterBI24 = foo;
         }
         return converterBI24.get();
      } // getBI24


      sio_8211Converter_BI32 * getBI32()
      {
         if ( ! converterBI32.get() )
         {
            auto_ptr<sio_8211Converter_BI32> foo( new sio_8211Converter_BI32 );
            converterBI32 = foo;
         }
         return converterBI32.get();
      } // getBI32


      sio_8211Converter_BUI8 * getBUI8()
      {
         if ( ! converterBUI8.get() )
         {
            auto_ptr<sio_8211Converter_BUI8> foo( new sio_8211Converter_BUI8 );
            converterBUI8 = foo;
         }
         return converterBUI8.get();
      } // getBUI8


      sio_8211Converter_BUI16 * getBUI16()
      {
         if ( ! converterBUI16.get() )
         {
            auto_ptr<sio_8211Converter_BUI16> foo( new sio_8211Converter_BUI16 );
            converterBUI16 = foo;
         }
         return converterBUI16.get();
      } // getBUI16



      sio_8211Converter_BUI24 * getBUI24()
      {
         if ( ! converterBUI24.get() )
         {
            auto_ptr<sio_8211Converter_BUI24> foo( new sio_8211Converter_BUI24 );
            converterBUI24 = foo;
         }
         return converterBUI24.get();
      } // getBUI24



      sio_8211Converter_BUI32 * getBUI32()
      {
         if ( ! converterBUI32.get() )
         {
            auto_ptr<sio_8211Converter_BUI32> foo( new sio_8211Converter_BUI32 );
            converterBUI32 = foo;
         }
         return converterBUI32.get();
      } // getBUI32



      sio_8211Converter_BFP32 * getBFP32()
      {
         if ( ! converterBFP32.get() )
         {
            auto_ptr<sio_8211Converter_BFP32> foo( new sio_8211Converter_BFP32 );
            converterBFP32 = foo;
         }
         return converterBFP32.get();
      } // getBFP32



      sio_8211Converter_BFP64 * getBFP64()
      {
         if ( ! converterBFP64.get() )
         {
            auto_ptr<sio_8211Converter_BFP64> foo( new sio_8211Converter_BFP64 );
            converterBFP64 = foo;
         }
         return converterBFP64.get();
      } // getBFP64

}; // struct ConverterFactory_Imp



sio_ConverterFactory::sio_ConverterFactory()
   : imp_( new ConverterFactory_Imp() )
{} // sio_ConverterFactory ctor




sio_ConverterFactory *
sio_ConverterFactory::instance()
{
   if ( ! sio_ConverterFactory::instance_.get() )
   {
      auto_ptr<sio_ConverterFactory> foo( new sio_ConverterFactory() );
      sio_ConverterFactory::instance_ = foo;
   }

   return sio_ConverterFactory::instance_.get();

} // sio_ConverterFactory::instance



sio_8211Converter *
sio_ConverterFactory::get( std::string const & type )
{
   if ( type.empty() )
   {
#ifdef SDTSXXDEBUG
      cerr << "binary type without a subfield\n";
#endif
      return 0x0;
   }

   // XXX change this to a switch statement!


   switch ( toupper(type[0]) )
   {
      case 'A' :
         return imp_->getA();
         break;

      case 'I' :
         return imp_->getI();
         break;

      case 'R' :
         return imp_->getR();
         break;

      case 'S' :
         return imp_->getS();
         break;

      case 'B' :
         if ( 'I' == toupper(type[1]) ) // signed
         {
            switch( type[2] )
            {
               case '8' :
                  return imp_->getBI8();
                  break;
               case '1':
                  return imp_->getBI16();
                  break;
               case '2' :
                  return imp_->getBI24();
                  break;
               case '3' :
                  return imp_->getBI32();
                  break;
            }
         }
         else if ( 'U' == toupper(type[1]) && // unsigned
                   'I' == toupper(type[2]) )
         {
            switch( type[3] )
            {
               case '8' :
                  return imp_->getBUI8();
                  break;

               case '1' :
                  return imp_->getBUI16();
                  break;

               case '2' :
                  return imp_->getBUI24();
                  break;

               case '3' :
                  return imp_->getBUI32();
                  break;
            }
         }
         else if ( 'F' == toupper(type[1]) && // floating point
                   'P' == toupper(type[2]) )
         {
            switch( type[3] )
            {
               case '6' :
                  return imp_->getBFP64();
                  break;

               case '3' :
                  return imp_->getBFP32();
                  break;
            }
         }
         break;
   }

   return 0x0;

} // sio_ConverterFactory::get

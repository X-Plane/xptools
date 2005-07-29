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
#ifndef INCLUDED_SIO_CONVERTERFACTORY_H
#define INCLUDED_SIO_CONVERTERFACTORY_H

#include <string>
#include <memory>



class sio_8211Converter;


/// opaque implementation structure for ConverterFactory
struct ConverterFactory_Imp;


/**

   This is a convenience class for creating an appropriate converter
   based on a string that dictates an SDTS data represenation.

 */
class sio_ConverterFactory
{
   public:

      /// return a converter that matches the given type
      /**

         The type is one of:

            A: graphic characters, alphanumeric characters, or 
               alphabetic characters.
            I: implicit-point (integer)
            R: explicit-point unscaled (fixed point real)
            S: explicit-point scaled (floating point real)
            B: bitfield data NOT SUPPORTED
            C: character mode bitfield (binary in zero and one characters)
               NOT SUPPORTED

         The 'B' type may have additional qualification as follows:
            BI8:    8 bit signed integer
            BI16:   16 bit signed integer
            BI24:   24 bit signed integer
            BI32:   32 bit signed integer
            BUI:    unsigned integer, length specified by implementation
            BUI8:   8 bit unsigned integer
            BUI16:  16 bit unsigned integer
            BUI24:  24 bit unsigned integer
            BUI32:  32 bit unsigned integer
            BFP32:  32 bit floating point real
            BFP64:  64 bit floating point real

       */
      sio_8211Converter * get( std::string const & type );

      /// return the canonical instance of the converter factory
      static sio_ConverterFactory * instance();

   private:

      /// private because this is a Singleton
      sio_ConverterFactory();

      /// implementation detail
      std::auto_ptr<ConverterFactory_Imp> imp_;

      static std::auto_ptr<sio_ConverterFactory> instance_;

}; // class sio_ConverterFactory

#endif

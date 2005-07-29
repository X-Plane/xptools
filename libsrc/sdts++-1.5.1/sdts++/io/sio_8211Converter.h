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
#ifndef INCLUDED_SIO_8211CONVERTER_H
#define INCLUDED_SIO_8211CONVERTER_H


#ifndef INCLUDED_SIO_BUFFER_H
#include <sdts++/io/sio_Buffer.h>
#endif

#ifndef INCLUDED_SIO_CONVERTER_H
#include <sdts++/io/sio_Converter.h>
#endif

/**
   A Converter converts between an SDTS subfield and 'raw' data.

   \note

    Niether the fixed or variable make() functions set the subfield
    mnemonic or the subfield name.

    \todo XXX consider singleton pattern to access static instances for all supported types

 */
class sio_8211Converter : public sio_Converter
{
   public:

      ///
      sio_8211Converter() {}

      ///
      virtual ~sio_8211Converter();



      /// Fixed width subfields
      /** Returns the length of the data actually converted. This may be
          different than 'length' if an error occured. */
      virtual long makeFixedSubfield(sc_Subfield& subfield,
                                     char const* data,
                                     long bitLength) const = 0;

#ifdef VECTOR_ITERATOR_POINTER_NOT_EQUIVALENT
      long makeFixedSubfield(sc_Subfield& subfield,
                                     std::vector<char>::const_iterator & data,
                                     long bitLength) const
        {
          return makeFixedSubfield( subfield, &data[0], bitLength );
        }
#endif

      /// Variable width subfields
      /** Returns the length of the data converted. This does not__ included
          the delimiter (if any).*/
      virtual long makeVarSubfield(sc_Subfield& subfield,
                                   char const* data,
                                   long maxLength,
                                   char delimiter) const;

#ifdef VECTOR_ITERATOR_POINTER_NOT_EQUIVALENT
      long makeVarSubfield(sc_Subfield& subfield,
                                   std::vector<char>::const_iterator & data,
                                   long maxLength,
                                   char delimiter) const
        {
          return makeVarSubfield( subfield, &data[0], maxLength, delimiter );
        }
#endif

      /// Adds a "null field" (adds an end-of-field character) to the buffer.
      virtual long addEmptySubfield(sio_Buffer& buffer) const;


      /// Adds a subfield, encoded in 8211 format, to the buffer
      virtual long addSubfield(sc_Subfield const& subf,
                               sio_Buffer& buffer) const = 0;

      /** Like addSubfield(), but for fixed length subfields.  Will not
       be implemented for binary fields, obviously.
      */
      virtual long addFixedSubfield(sc_Subfield const& subf,
                                    long length,
                                    sio_Buffer& buffer) const = 0;
            
   protected:

      /** Determines the length of a variable length subfield in 'data'
       delimited by 'delimiter'.
      */
      long findVariableSubfieldLength(char const* data,
                                      long maxLength,
                                      char delimiter) const;

}; // sio_8211Converter


/// Converter for 8211 'A' data
class sio_8211Converter_A : public sio_8211Converter
{
   public:
 
      sio_8211Converter_A() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long length) const;

      long addSubfield(sc_Subfield const& subf,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const;
}; // sio_8211DConverter_A



/// Converter for 8211 'I' data
class sio_8211Converter_I : public sio_8211Converter
{
   public:
 
      sio_8211Converter_I() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long length) const;

      long addSubfield(sc_Subfield const& subf,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const;
}; // class sio_8211Converter_I



/// Converter for 8211 'R' data
class sio_8211Converter_R : public sio_8211Converter
{
   public:

      sio_8211Converter_R() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long length) const;

      long addSubfield(sc_Subfield const& subf,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const;
}; // class sio_8211Converter_R



// Converter for 8211 'S' data
class sio_8211Converter_S : public sio_8211Converter
{
   public:

      sio_8211Converter_S() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long length) const;

      long addSubfield(sc_Subfield const& subf,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const;

}; // class sio_8211Converter_S



/// Converter for 8111 'C' data
class sio_8211Converter_C : public sio_8211Converter
{
   public:

      sio_8211Converter_C() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long length) const;

      long addSubfield(sc_Subfield const& subf,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const;

}; // class sio_8211Converter_C



/// Converter for 8211 eight bit signed integer data
class sio_8211Converter_BI8 : public sio_8211Converter
{
   public:
      sio_8211Converter_BI8() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BI8



/// Converter for 8211 sixteen bit signed integers
class sio_8211Converter_BI16 : public sio_8211Converter
{
   public:
      sio_8211Converter_BI16() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BI16



/// Converter for 8211 24 bit signed integers
class sio_8211Converter_BI24 : public sio_8211Converter
{
   public:
      sio_8211Converter_BI24() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BI24



/// Converter for 32 bit signed integers
class sio_8211Converter_BI32 : public sio_8211Converter
{
   public:
      sio_8211Converter_BI32() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BI32



/// Converter for eight bit unsigned integers
class sio_8211Converter_BUI8 : public sio_8211Converter
{
   public:
      sio_8211Converter_BUI8() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BUI8



/// Converter for sixteen bit unsigned integers
class sio_8211Converter_BUI16 : public sio_8211Converter
{
   public:
      sio_8211Converter_BUI16() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BUI16



/// Converter for 24 bit unsigned integers
class sio_8211Converter_BUI24 : public sio_8211Converter
{
   public:
      sio_8211Converter_BUI24() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BUI24



/// Converter for 32 bit unsigned integers
class sio_8211Converter_BUI32 : public sio_8211Converter
{
   public:
      sio_8211Converter_BUI32() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BUI32


/// Converter for 32 bit floating point numbers
class sio_8211Converter_BFP32 : public sio_8211Converter
{
   public:
      sio_8211Converter_BFP32() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BFP32


/// Converter for 64 bit floating point numbers
class sio_8211Converter_BFP64 : public sio_8211Converter
{
   public:
      sio_8211Converter_BFP64() {}

      long makeFixedSubfield(sc_Subfield& subfield,
                             char const* data,
                             long bitLength) const;

      long addSubfield(sc_Subfield const& subfield,
                       sio_Buffer& buffer) const;

      long addFixedSubfield(sc_Subfield const& subf,
                            long length,
                            sio_Buffer& buffer) const
      { return addSubfield( subf, buffer ); }

}; // class sio_8211Converter_BFP64

#endif

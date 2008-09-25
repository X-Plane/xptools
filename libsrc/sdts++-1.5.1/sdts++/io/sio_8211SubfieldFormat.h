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
//
// sio_8211SubfieldFormat.h
//


#ifndef INCLUDED_SIO8211SUBFIELDFORMAT_H
#define INCLUDED_SIO8211SUBFIELDFORMAT_H




#if MSC__VER >= 1000
#pragma once
#endif // MSC__VER >= 1000


#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <string>

// forward reference for ``converter_func''
class sc_Subfield;

// This is a signature for a subfield conversion function.  It will accept
// a character pointer to the start of the raw, 8211 subfield data and its
// corresponding length.  It will then convert that data into the given
// sc_Subfield.

// XXX Update above comment for new sio_Converter class.

class sio_8211Converter;



struct sio_8211SubfieldFormatImp;

/**

  8211 subfields have formatting information.  This information includes
  whether or not the subfield is read by a length or is character delimited.
  Moreover, a subfield has a type.  There also exists a set of formatting
  functions for taking a raw subfield and converting it into an sc_Subfield;
  a subfield format will be bound to one of these functions.  And, even though
  it has nothing directly to do with formats per se, a subfield format will
  also have a label, which is a unique string for identifying that subfield.

  sio_8211SubfieldFormats will typically live inside of sio_8211FieldFormats.

*/
class sio_8211SubfieldFormat
{
   public:

      // types

      /**
         Defines 8211 types for:

         A           string
         I           integer
         R           floats
         S           fixed point
         C           character
         B           binary
         X           unused characters

      */
      typedef enum { A, I, R, S, C, B, X } type;

      /**
         Denotes whether a given subfield descriptor is of fixed length or
         is character delimited.
      */
      typedef enum { fixed, variable } format;


      ///
      sio_8211SubfieldFormat();

      ///
      ~sio_8211SubfieldFormat();

      ///
      sio_8211SubfieldFormat( sio_8211SubfieldFormat const & sf );

      ///
      sio_8211SubfieldFormat& operator=( sio_8211SubfieldFormat const & rhs );


      ///
      std::string const& getLabel() const;

      ///
      /**
          If the type is binary, the length is converted to character
          units from bits.  We used to do this in setLength().  However,
          the parser would sometimes not set the binary type before the
          length was set, so the length was never converted properly.
      */
      type getType() const;

      ///
      format getFormat() const;

      /// undefined results if descriptor is not controlled by length
      int getLength() const;

      /// undefined results if descriptor is not controlled by delimiter
      char getDelimiter() const;

      ///
      sio_8211Converter const * getConverter() const;



      ///
      void setLabel( std::string const & );

      ///
      void setType( type );

      ///
      void setFormat( format );

      ///
      void setLength( int ); // sets format to fixed as a side-effect

      ///
      void setDelimiter( char ); // sets format to variable as a side-effect

      ///
      void setConverter( sio_8211Converter const * );


   private:

      ///
      sio_8211SubfieldFormatImp*  imp_;

      friend std::ostream& operator<<( std::ostream& os, sio_8211SubfieldFormat const & );

}; // class sio_8211SubfieldFormat


#endif // INCLUDED_SIO8211SubfieldFormat_H

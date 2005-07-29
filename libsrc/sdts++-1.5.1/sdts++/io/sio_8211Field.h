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
// sio_8211Field.h
//

#ifndef INCLUDED_SIO_8211FIELD_H
#define INCLUDED_SIO_8211FIELD_H



#include <iostream>
#include <vector>
#include <string>


#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif



class sio_Buffer;
class sio_8211DirEntry;
class sio_8211DDRLeader;



/// This represents a generic ISO 8211 field.
class sio_8211Field
{
   public:

      ///
      sio_8211Field();

      /// fieldSize is the size of this field, in bytes.
      explicit sio_8211Field(long fieldSize);

      ///
      sio_8211Field( sio_8211Field const& );

      ///
      sio_8211Field( sio_Buffer const& buffer );

      ///
      virtual ~sio_8211Field() {}

      /**
         All fields have two things we're interested in... the raw
         data it contains, and how much of that data there is.
         somewhat redundant as now you can directly query the vector
         for its size 
      */
      long getDataLength() const;

      /// does what you expect
      void setDataLength( int );

      ///
      std::vector<char> const& getData() const;

      ///
      std::vector<char>      & getData() ;


      /// Sets the contents of the field. A copy of 'data' is made.
      bool setData(std::vector<char> const& data);

      /// adds the given data to the current field
      bool addData(std::vector<char> const& data);


      /**
       All getVariableSubfield() operations attempt to extract a variable width
       value. By default, the variable width subfield delimeter is the ISO8211 
       Unit Terminator. This can be overridden.
       Value should refer to a buffer that's allocated enough space to hold
       the subfield, in the case of the char*.  'startPos' should indicate 
       where in the field's data area the subfield starts.  It will be changed
       to refer to the next position in the field's data area; this will
       either be the first byte of the next subfield, or one past the last
       byte of the field's data area.
      */
      bool getVariableSubfield( std::vector<char>& value,
                                long& startPos,
                                char delim = sio_8211UnitTerminator ) const;


      /// returns the data area
      sio_Buffer getField() const;


      friend std::istream& operator>>( std::istream & istr, sio_8211Field & field );
      friend std::ostream& operator<<( std::ostream & ostr, sio_8211Field const & field );

   protected:

      ///
      std::vector<char> data_;

      /// field size
      int          size_;

      virtual std::istream & streamExtract( std::istream & istr );
      virtual std::ostream & streamInsert( std::ostream & ostr) const;
};


///
std::istream &
operator>>( std::istream & istr, sio_8211Field & field );

///
std::ostream &
operator<<( std::ostream & ostr, sio_8211Field const & field);

#endif  // INCLUDED_SIO_8211FIELD_H

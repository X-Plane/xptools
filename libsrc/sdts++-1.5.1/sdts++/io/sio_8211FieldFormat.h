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
// sio_8211FieldFormat.h
//


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#define INCLUDED_SIO8211FIELDFORMAT_H


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#ifndef INCLUDED_SIO8211SUBFIELDFORMAT_H
#include <sdts++/io/sio_8211SubfieldFormat.h>
#endif



#include <list>
#include <map>
#include <string>


///
struct sio_8211FieldFormatImp;


/**

  Each 8211 field has several features.  It has an optional name.  It has
  a tag, which is used to locate the field formatting information in the
  DR's.  It also has a type.  Morever, a field format can consist of several
  subfield formats.  (Which is why it's implemented as a container of subfield
  formats.

  This is used to pull in field values from an 8211 DR.  Given the start
  position and length of a field's data in an 8211 DR, you need to then
  find out how to convert that data into something meaningful.  You
  would do so by getting the appropriate field format for that data,
  and use it to convert the data.

  No internal consistency checking is done.  It is possible, for example, to
  have a field format set as an elementary data structure that has subfields;
  this violates the standard as field formats only have subfields if it's a
  vector or an array field.

  */
class sio_8211FieldFormat : public std::list<sio_8211SubfieldFormat>
{
   public:

      ///
      typedef std::list<sio_8211SubfieldFormat>::iterator iterator;

      ///
      typedef std::list<sio_8211SubfieldFormat>::const_iterator const_iterator;


      /// XXX using ``vector'' as one of the enumerated names may cause problems.
      typedef enum { elementary, vector, array, concatenated } data_struct_code;

      /**
          XXX these are a bit verbose, though they do comply with the
          standard's nomenclature.  */
      typedef enum { char_string,
                     implicit_point,
                     explicit_point,
                     explicit_point_scaled,
                     char_bit_string,
                     bit_string,
                     mixed_data_type } data_type_code;

      ///
      sio_8211FieldFormat();

      ///
      sio_8211FieldFormat( sio_8211FieldFormat const & );

      ///
      ~sio_8211FieldFormat();



      ///
      sio_8211FieldFormat& operator=( sio_8211FieldFormat const & );




      ///
      data_struct_code getDataStructCode( ) const;

      ///
      data_type_code   getDataTypeCode( ) const;

      ///
      std::string const&    getTag( ) const;

      ///
      std::string const&    getName( ) const;

      ///
      char             getFieldTerm( ) const;

      ///
      char             getUnitTerm( ) const;

      /// returns true if this is a repeatin field
      /** XXX for binary repeating field special case **/
      bool             isRepeating() const;



      ///
      void setDataStructCode( data_struct_code );

      ///
      void setDataTypeCode( data_type_code );

      ///
      void setTag( std::string const & );

      ///
      void setName( std::string const& );

      ///
      void setFieldTerm( char );

      ///
      void setUnitTerm( char );

      ///
      void setIsRepeating( bool repeating );


      ///  returns comparison of the two field tags
      bool operator<( sio_8211FieldFormat const& rhs ) const;

      ///  returns comparison of the two field tags
      bool operator>( sio_8211FieldFormat const& rhs ) const;

      ///  returns comparison of the two field tags
      bool operator==( sio_8211FieldFormat const& rhs ) const;

      ///  returns comparison of the two field tags
      /**  This special string comparison is needed by find(). */
      bool operator==( std::string const& rhs ) const;

      /// returns comparison of the two field tags
      /**  This special string comparison is needed by find(). */
      bool operator!=( std::string const& rhs ) const;

      ///  returns comparison of the two field tags
      bool operator!=( sio_8211FieldFormat const& rhs ) const { return ! (*this == rhs); }


   private:

      /// hidden implementation details
      sio_8211FieldFormatImp* imp_;

      friend std::ostream& operator<<( std::ostream& os, sio_8211FieldFormat const& ff );

}; // class sio_8211FieldFormat


///
std::ostream& operator<<( std::ostream& os, sio_8211FieldFormat const& ff );


class sio_8211DDRField;
class sio_8211Converter;


///
typedef std::map<std::string, sio_8211Converter*, std::less<std::string> >
sio_8211_converter_dictionary;



/**

Takes the given DDR field and modifies the given field format object
to hold the field and subfield formats found in the DDR field.  The
map of a string to a pointer to a converter class is important.  It
allows this function to properly bind a binary conversion function for
a given subfield label.  In other words, for each subfield format,
there exists a corresponding converter class that will convert a raw
DR subfield into an equivalent sc_Subfield.  For 8211 A, I, R, and S
types, this is fairly straightforward.  The binary types are a bit of
a problem; e.g., a SADR could be a BI32 or a BU16.  That type
information isn't stored in the 8211 file that the SADR was found in,
so the user has to supply that information to give the reader a hint
as to how to cope with that data.


        Returns true if successful, else false.

 XXX Does this belong here?  And what about the field tag information?
 XXX Should that be passed in separately, or should we make the
 XXX DDRField know its tag?

  */
bool sio_8211MakeFieldFormat( sio_8211FieldFormat & ff ,
                              sio_8211DDRField const & ddr_field,
                              std::string const & field_tag,
                              sio_8211_converter_dictionary const* binary_converter_hints = 0
   );



/**
 An sio_8211Writer needs to know all the fields and subfields that
 can be written out for a given record.  It needs to know all this
 information up front (i.e., at construct time) so that it can
 write out a proper DDR.  The writer also needs to know field and subfield
 formatting information so that it can properly write out each record
 it gets via sio_Writer::put().  All this information is conveniently
 wrapped up in a container of field formats, sio_8211Schema.

 Each schema will be unique for a module type.  For example, an IDEN
 module schema will have field formats for the identification,
 conformance, and attribute id fields.  This will include all the
 subfield label and type information.  See build_iden_schema() in
 the test app "../sio_Writer_t/sio_Writer.cpp".

*/
typedef std::list <sio_8211FieldFormat> sio_8211Schema;



#endif // INCLUDED_SIO8211FIELDFORMAT_H

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
// sio_8211DDRField.h
//

#ifndef INCLUDED_SIO_8211DDRFIELD_H
#define INCLUDED_SIO_8211DDRFIELD_H

#include <string>


#ifndef INCLUDED_SIO_BUFFER_H
#include <sdts++/io/sio_Buffer.h>
#endif

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


class sio_8211DDRLeader;
class sio_8211Field;

/**

   An sio_8211DDRField is used to interpret the data in a raw
   sio_8211Field.

       XXX Note that printable graphics access members are undefined
       XXX as they're superfluous

*/
class sio_8211DDRField
{

   public:

      ///
      sio_8211DDRField();

      ///
      sio_8211DDRField(sio_8211DDRLeader const& leader,
                       sio_8211Field const& field);

      ///
      virtual ~sio_8211DDRField();

      ///
      char getDataStructCode() const;

      ///
      char getDataTypeCode() const;


      ///
      std::string const& getDataFieldName() const;

      ///
      std::string const& getArrayDescriptor() const;

      ///
      std::string const& getFormatControls() const;

      ///
      virtual sio_Buffer getField() const;

      ///
      void setDataFieldName( std::string const &name );

      ///
      void setDataStructCode( char code );

      ///
      void setDataTypeCode( char code );

      ///
      void setDataStructCode( sio_8211FieldFormat::data_struct_code dsc );

      ///
      void setDataTypeCode( sio_8211FieldFormat::data_type_code dtc );

      ///
      void setArrayDescriptor( std::string const &descriptor );

      ///
      void setFormatControls( std::string const &control );

   private:

      ///
      char   dataStructCode_;

      ///
      char   dataTypeCode_;

      ///
      std::string fieldName_;

      ///
      std::string arrayDescr_;

      ///
      std::string formatControls_;

      /// From the ddr leader.
      long   fieldControlLength_;

}; // sio_8211DDRField



/// This corresponds to the 8211 DDR file title field
class sio_8211FileTitleField : public sio_8211DDRField
{
   public:

      ///
      sio_8211FileTitleField( );

      ///
      sio_8211FileTitleField( std::string const& title );

      ///
      sio_Buffer getField() const;

}; // class sio_8211FileTitleField



/// This corresponds to the 8211 DDR record identifier field
class sio_8211RecordIdentifierField : public sio_8211DDRField
{
   public:

      ///
      sio_8211RecordIdentifierField();

      ///
      sio_Buffer getField() const;

      /**
       XXX Kludge!!! This allows for easy insertion of the current
       XXX value into an outgoing stream; called by an sio_Writer object.
      */
      sio_Buffer recordNum() const;

      ///
      int & recordNum() { return recnum_; }


   private:

      ///
      int recnum_;

}; // class sio_8211FileTitleField



#endif  // INCLUDED_SIO_8211DDRFIELD_H

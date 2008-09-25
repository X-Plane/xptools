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
// sio_8211DDRField.cpp
//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <strstream>



#include <sdts++/io/sio_8211DDRField.h>

#ifndef INCLUDED_SIO_8211FIELD_H
#include <sdts++/io/sio_8211Field.h>
#endif

#ifndef INCLUDED_SIO_8211DDRLEADER_H
#include <sdts++/io/sio_8211DDRLeader.h>
#endif


using namespace std;




// Needed to define this else VC++ whines about no default ctor's for the
// derived classes.
sio_8211DDRField::sio_8211DDRField()
  : fieldName_(""), arrayDescr_(""), formatControls_("")
{
  fieldName_.reserve(256);         // having to reserve enough space
  arrayDescr_.reserve(256);        // by hand to work around bug in
  formatControls_.reserve(256);    // VC++ <1>
} // sio_8211DDRField ctor

// <1> this bug basically involves assigning string values to strings that
// already have enough space to accomodate the new value; then a memcpy()
// call gets called that over-runs the allocated space somehow.  I've not
// been able to consistently reproduce this error though; and it was only
// thanks to BoundsChecker that I was able to find this problem at all.


sio_8211DDRField::sio_8211DDRField(sio_8211DDRLeader const& leader,
                                   sio_8211Field const& field)
  : fieldControlLength_(leader.getFieldControlLength())
{

  dataStructCode_ = field.getData()[0];
  dataTypeCode_ = field.getData()[1];

  // Starting at the end of the field controls, look for an optional data field
  // name followed by a unit terminator.

  long pos = fieldControlLength_;

  vector<char> tmp_data;

  if ( field.getVariableSubfield(tmp_data, pos) )
    {
      fieldName_.assign( &tmp_data[0], tmp_data.size() );
//         fieldName_.resize( tmp_data.size() );
//         copy( tmp_data.begin(), tmp_data.end(), fieldName_.begin() );
    }


  if ( field.getVariableSubfield(tmp_data, pos) )
    {
      arrayDescr_.assign( &tmp_data[0], tmp_data.size() );
//         arrayDescr_.resize( tmp_data.size() );
//         copy( tmp_data.begin(), tmp_data.end(), arrayDescr_.begin() );
    }

  if ( field.getVariableSubfield(tmp_data, pos) )
    {
      formatControls_.assign( &tmp_data[0], tmp_data.size() );
//         formatControls_.resize( tmp_data.size() );
//         copy( tmp_data.begin(), tmp_data.end(), formatControls_.begin() );
    }

}

sio_8211DDRField::~sio_8211DDRField()
{
}

char
sio_8211DDRField::getDataStructCode() const
{
  return dataStructCode_;
}

char
sio_8211DDRField::getDataTypeCode() const
{
  return dataTypeCode_;
}

string const&
sio_8211DDRField::getDataFieldName() const
{
  return fieldName_;
}

string const&
sio_8211DDRField::getArrayDescriptor() const
{
  return arrayDescr_;
}

string const&
sio_8211DDRField::getFormatControls() const
{
  return formatControls_;
}

void
sio_8211DDRField::setDataFieldName( string const &name )
{
  fieldName_ = name;
}

void
sio_8211DDRField::setDataStructCode( char code )
{
  dataStructCode_ = code;
}

void
sio_8211DDRField::setDataTypeCode( char code )
{
  dataTypeCode_ = code;
}

void
sio_8211DDRField::setDataStructCode( sio_8211FieldFormat::data_struct_code dsc )
{
  switch ( dsc )
    {
    case sio_8211FieldFormat::elementary :
      setDataStructCode( '0' );
      break;
    case sio_8211FieldFormat::vector :
      setDataStructCode( '1' );
      break;
    case sio_8211FieldFormat::array :
      setDataStructCode( '2' );
      break;
    case sio_8211FieldFormat::concatenated :
      setDataStructCode( '3' );
      break;
    }
}

void
sio_8211DDRField::setDataTypeCode( sio_8211FieldFormat::data_type_code dtc )
{
  switch ( dtc )
    {
    case  sio_8211FieldFormat::char_string :
      setDataTypeCode( '0' );
      break;
    case sio_8211FieldFormat::implicit_point :
      setDataTypeCode( '1' );
      break;
    case sio_8211FieldFormat::explicit_point :
      setDataTypeCode( '2' );
      break;
    case sio_8211FieldFormat::explicit_point_scaled :
      setDataTypeCode( '3' );
      break;
    case sio_8211FieldFormat::char_bit_string :
      setDataTypeCode( '4' );
      break;
    case sio_8211FieldFormat::bit_string :
      setDataTypeCode( '5' );
      break;
    case sio_8211FieldFormat::mixed_data_type :
      setDataTypeCode( '6' );
      break;
    }
}

void
sio_8211DDRField::setArrayDescriptor( string const &descriptor )
{
  arrayDescr_.assign( descriptor );
}

void
sio_8211DDRField::setFormatControls( string const &control )
{
  formatControls_ = control;
}






sio_Buffer
sio_8211DDRField::getField() const
{
  sio_Buffer buffer;

  // first add the format controls

  buffer.addData( getDataStructCode() );
  buffer.addData( getDataTypeCode() );
  buffer.addData( "00", 2 ); // unused auxiliary controls
  buffer.addData( ";&", 2 ); // stupid print characters

  // then the data field name

  buffer.addData( getDataFieldName().c_str(), getDataFieldName().length() );
  buffer.addData( sio_8211UnitTerminator );

  // then the array descriptors (subfield labels)

  buffer.addData( getArrayDescriptor().c_str(), getArrayDescriptor().length() );
  buffer.addData( sio_8211UnitTerminator );

  // then the field formats

  buffer.addData( getFormatControls().c_str(), getFormatControls().length() );

  // add final field terminator

  buffer.addData( sio_8211FieldTerminator );

  return buffer;
} // sio_8211DDRField::getField() const







sio_8211FileTitleField::sio_8211FileTitleField()
{
  setDataStructCode( '0' );
  setDataTypeCode( '0' );
  setDataFieldName( "" ); // file name
} // sio_8211FileTitleField ctor



sio_8211FileTitleField::sio_8211FileTitleField( string const& title )
{
  setDataStructCode( '0' );
  setDataTypeCode( '0' );
  setDataFieldName( title ); // file name
} // sio_8211FileTitleField ctor



sio_Buffer
sio_8211FileTitleField::getField() const
{
  sio_Buffer buffer;

  // first add the format controls

  buffer.addData( getDataStructCode() );
  buffer.addData( getDataTypeCode() );
  buffer.addData( "00", 2 ); // unused auxiliary controls
  buffer.addData( ";&", 2 ); // stupid print characters

  // then the data field name

  buffer.addData( getDataFieldName().c_str(), getDataFieldName().length() );

  // add two unit terminators, one for delimiting the file title, and
  // the other for the field tag pairs

  buffer.addData( sio_8211UnitTerminator );

  buffer.addData( sio_8211UnitTerminator );

  // add final field terminator

  buffer.addData( sio_8211FieldTerminator );

  return buffer;
} // sio_8211FileTitleField::getField() const








sio_8211RecordIdentifierField::sio_8211RecordIdentifierField()
  : recnum_( 1 )                // 8211 always starts at '1' for each file
{
  setDataStructCode( '0' );
  setDataTypeCode( '1' );
  setDataFieldName( "DDF RECORD IDENTIFIER" );
} // sio_8211RecordIdentifierField ctor



sio_Buffer
sio_8211RecordIdentifierField::getField() const
{
  sio_Buffer buffer;

  // first add the format controls

  buffer.addData( getDataStructCode() );
  buffer.addData( getDataTypeCode() );
  buffer.addData( "00", 2 ); // unused auxiliary controls
  buffer.addData( ";&", 2 ); // stupid print characters

  // then the data field name

  buffer.addData( getDataFieldName().c_str(), getDataFieldName().length() );


  // add final field terminator

  buffer.addData( sio_8211FieldTerminator );

  return buffer;
} // sio_8211FileTitleField::getField() const



sio_Buffer
sio_8211RecordIdentifierField::recordNum() const
{
  std::strstream ss;

  ss << setfill('0') << setw(7) << recnum_ << sio_8211FieldTerminator << ends;

  ss.freeze();                  // lock the number in place

  return sio_Buffer( ss.str(), strlen( ss.str() ) );
} // sio_8211RecordIdentifierField::recordNum

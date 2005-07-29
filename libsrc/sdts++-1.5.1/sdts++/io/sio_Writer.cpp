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
// sio_Writer.cpp
//


#include <strstream>
#include <fstream>

#include <string>
#include <algorithm>
#include <cstdlib>

#ifndef INCLUDED_SIO_WRITER_H
#include <sdts++/io/sio_Writer.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif

#ifndef INCLUDED_SC_FIELD_H
#include <sdts++/container/sc_Field.h>
#endif

#ifndef INCLUDED_SIO_8211DR_H
#include <sdts++/io/sio_8211DR.h>
#endif

#ifndef INCLUDED_SIO_8211DDR_H
#include <sdts++/io/sio_8211DDR.h>
#endif

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

#ifndef INCLUDED_SIO_8211DDRFIELD_H
#include <sdts++/io/sio_8211DDRField.h>
#endif

#ifndef INCLUDED_SIO_BUFFER_H
#include <sdts++/io/sio_Buffer.h>
#endif

#ifndef INCLUDED_SIO_CONVERTER_H
#include <sdts++/io/sio_Converter.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif



static const char* ident_ = "$Id: sio_Writer.cpp,v 1.12 2002/11/27 00:21:34 mcoletti Exp $";


using namespace std;



// returns true if the schema contains the ISO 8211 Reserved Field
static
bool
foundRecIdenField_( sio_8211Schema const & schema )
{
  // find the DDF RECORD IDENTIFIER FIELD, which has a field
  // mnemonic of "0001"

  sio_8211Schema::const_iterator field_format = 
    find( schema.begin(), schema.end(), "0001" );

  if ( field_format == schema.end() ) 
  {
    return false;
  }

  return true;

} // foundRecIdenField_



sio_Writer::~sio_Writer( )
{}


//
// The guts for an sio_8211Writer object.
//


struct sio_8211Writer_Imp
{
  sio_8211Writer_Imp( sio_8211Schema const & schema,
                      const char* title,
                      ofstream& ofs ) 
    : schema_( schema ), 
      title_( title ),
      ofs_( ofs ),
      droppedLeaderAndDir_( false ),
      wroteDroppedLeaderAndDir_( false ),
      haveRecIdenField_( false )
  {
    haveRecIdenField_ = foundRecIdenField_( schema );
  }

  sio_8211Writer_Imp( const char* title,
                      ofstream& ofs ) 
    : title_( title ),
      ofs_( ofs ),
      droppedLeaderAndDir_( false ),
      wroteDroppedLeaderAndDir_( false ),
      haveRecIdenField_( false )
  {}

  sio_8211Writer_Imp( sio_8211Schema const & schema,
                      string const& title,
                      ofstream& ofs ) 
    : schema_( schema ), 
      title_( title ),
      ofs_( ofs ),
      droppedLeaderAndDir_( false ),
      wroteDroppedLeaderAndDir_( false ),
      haveRecIdenField_( false )
  {
    haveRecIdenField_ = foundRecIdenField_( schema );
  }

  sio_8211Writer_Imp( string const& title,
                      ofstream& ofs ) 
    : title_( title ),
      ofs_( ofs ),
      droppedLeaderAndDir_( false ),
      wroteDroppedLeaderAndDir_( false ),
      haveRecIdenField_( false )
  {}


  sio_8211Schema          schema_;
  // Contains the field and subfield formatting information for
  // that will be used to build the DDR and to write out each
  // record.

  string                  title_; /// file title

  ofstream &              ofs_;

  sio_8211DDR             ddr_;

  bool                    droppedLeaderAndDir_;
  // is true if we're going to start writing records without
  // leaders and directories to save space -- let's just hope
  // that the operator knows that the records MUST be heterogeneous
  // for each put() invocation

  bool                    wroteDroppedLeaderAndDir_;
  // is true if we wrote the last leader (the special one with 
  // the type set to 'R' instead of 'D') and directory -- this means
  // that put() can tell its sio_8211DR to not emit the leader and
  // and directory

  bool                    haveRecIdenField_;
  // is true if the schema contains an entry for the ISO 8211 "0001"
  // record identfier reserved field

  sio_8211RecordIdentifierField recIdenField_;
  // iff__ haveRecIdenField is true, then this field will contain the
  // current record identifier field, which is incremented with each
  // put()

}; //  struct sio_8211Writer_Imp




sio_8211Writer::sio_8211Writer( ofstream & ofs,
                                const char* title,
                                sio_8211Schema const & schema )
  : imp_( new sio_8211Writer_Imp( schema, title, ofs ) )
{
} // sio_8211Writer ctor



sio_8211Writer::sio_8211Writer( ofstream & ofs,
                                const char* title )
  : imp_( new sio_8211Writer_Imp( title, ofs ) )
{
} // sio_8211Writer ctor



sio_8211Writer::sio_8211Writer( ofstream & ofs,
                                string const& title,
                                sio_8211Schema const & schema )
  : imp_( new sio_8211Writer_Imp( schema, title, ofs ) )
{
} // sio_8211Writer ctor



sio_8211Writer::sio_8211Writer( ofstream & ofs,
                                string const& title )
  : imp_( new sio_8211Writer_Imp( title, ofs ) )
{
} // sio_8211Writer ctor



void
sio_8211Writer::setFileTitle( const char* fn )
{
  imp_->title_ = fn;
} // sio_8211Writer::setFileTitle



void
sio_8211Writer::setSchema( sio_8211Schema const & schema )
{
  imp_->schema_ = schema;

  imp_->haveRecIdenField_ = foundRecIdenField_( schema );

} // sio_8211Writer::setSchema



bool
sio_8211Writer::emitDDR()
{

#ifdef SDTSXX_DEBUG
  cout << __FILE__ << ":" << __LINE__ << "\n";
  ostream_iterator<sio_8211FieldFormat> os_itr( cout, "\n" );
  copy( imp_->schema_.begin(), imp_->schema_.end(), os_itr );
#endif

  makeDDR_();			// build DDR

  imp_->ofs_ << imp_->ddr_;	// blat it out to the file stream

   return imp_->ofs_.good();

} // sio_8211Writer::emitDDR()




sio_8211Writer::~sio_8211Writer()
{
  delete imp_;
} // sio_8211Writer dtor




// XXX this should probably go into some utilties package
static
bool
int2string_( int val, string& str )
{
  strstream tmp_strstream;
  tmp_strstream << val;

  string tmp_string;
  tmp_strstream >> str;

  return tmp_strstream.good();
} // int_2string




//
// A utility function that adds a subfield format string to 
// "field_format" followed by a ','.
//
//  E.g., four string subfields five bytes long will add
// "4A(5)," to "field_format".
//
static
void
addFormat_( sio_8211SubfieldFormat const & subfield_format,
            int count,
            string & field_format )
{

  if ( count > 1 )		// then emit the count for that format
  {
#ifdef OLD
    char buffer[4];
    itoa_( count, buffer, 10 );
#endif
    string int_val;
    int2string_( count, int_val ); // XXX check return value
    field_format += int_val;
  }

  switch ( subfield_format.getType() )
  {
  case sio_8211SubfieldFormat::A :
    field_format.append( "A" );
    break;
  case sio_8211SubfieldFormat::I :
    field_format.append( "I" );
    break;
  case sio_8211SubfieldFormat::R :
    field_format.append( "R" );
    break;
  case sio_8211SubfieldFormat::S :
    field_format.append( "S" );
    break;
  case sio_8211SubfieldFormat::C :
    field_format.append( "C" );
    break;
  case sio_8211SubfieldFormat::B :
    field_format.append( "B" );
    break;
  case sio_8211SubfieldFormat::X :
    field_format.append( "X" );
    break;
  }

				// if fixed length, then blat out the length

  if ( subfield_format.getFormat() == sio_8211SubfieldFormat::fixed )
  {
    // char buffer[8];

    //_itoa( subfield_format.getLength(), buffer, 10 );

    string int_val;		// XXX check return value
    int2string_( subfield_format.getLength(), int_val ); 

    field_format.append( "(" );
    field_format.append( int_val );
    field_format.append( ")" );
  }

  field_format.append( "," );	// add format delimiter

} // addFormat_()





//
// An extra pair of parenthesis will have to be added around the
// field format string iff it's a repeating solitary binary field.
// This compensates for an ISO 8211 limitation.
//
// XXX Do I really__ have the conditions completely correct?
//
static
void
doBinaryRepeatingField_( sio_8211FieldFormat const& field_format,
                         string & field_format_str )

{				// ONLY do this if the field is
				// repeating and binary.

  if ( field_format.isRepeating() &&
       field_format.getDataTypeCode() == sio_8211FieldFormat::bit_string )
  {
    field_format_str.insert( 0, "(" );
    field_format_str += ")";
  }
} // doBinaryRepeatingField_





//
// Makes a string containing the 8211 subfield format for
// the given subfields.  When done, "field_format_str" will contain
// the subfield labels and the subfield formats per the 8211 spec.
//
// Please note that this isn't a full 8211 implementation; we take
// advantage of SDTS specifics here.  For one thing, the standard doesn't 
// explicitly define character delimiters, so we don't check to see
// if consecutive subfields differ by specific character delimiters.
// Similarly, we don't check to see if consecutive subfield formats
// differ by length.  We only care if the subfield types__ differ
// from one subfield to another.
//
static
void
makeFieldFormatString_( sio_8211FieldFormat const & field_format,
                        string & field_format_str )
{
  field_format_str = "("; // insure we're working from an empty format
      
  int count = 1; // number of current subfield formats with same type

  sio_8211FieldFormat::const_iterator last_subfield_format = 
    field_format.begin();
  sio_8211FieldFormat::const_iterator current_subfield_format = 
    last_subfield_format;

  for ( current_subfield_format++; // skip to second item, if any
        current_subfield_format != field_format.end();
        current_subfield_format++ )
  {

    if ( (*current_subfield_format).getType() 
         == (*last_subfield_format).getType() &&
         (*current_subfield_format).getFormat() 
         == (*last_subfield_format).getFormat() )
    {

                                // If the subfields are fixed length,
                                // then insure that their respective
                                // lengths are the same; if they are
                                // variable length, then make sure
                                // that their delimiters are
                                // identical, too.

      switch ( (*last_subfield_format).getFormat() )
      {
      case sio_8211SubfieldFormat::fixed :
        if ( (*last_subfield_format).getLength() ==
             (*current_subfield_format).getLength() )
        {
          count++;
        }
        else
        {
          addFormat_( *last_subfield_format, count, field_format_str );
          count = 1;
          last_subfield_format = current_subfield_format;
          continue;
        }
        break;
      case sio_8211SubfieldFormat::variable :
        if ( (*last_subfield_format).getDelimiter() ==
             (*current_subfield_format).getDelimiter() )
        {
          count++;
        }
        else
        {
          addFormat_( *last_subfield_format, count, field_format_str );
          count = 1;
          last_subfield_format = current_subfield_format;
          continue;
        }
        break;
      default :
        break;
      }

    }
    else // the subfields are different, so emit the format for the last 
    {    // subfield
      addFormat_( *last_subfield_format, count, field_format_str );
      count = 1;
      last_subfield_format = current_subfield_format;
    }
  }       

  // blat out the last format
  addFormat_( *last_subfield_format, count, field_format_str );

                                // close the outer parenthesis (the
                                // last character will be a comma, so
                                // we over-write it.)

  field_format_str[field_format_str.length() - 1] = ')'; 

  // add additional wrapping parenthesis iff this is a repeating binary field
  doBinaryRepeatingField_( field_format, field_format_str );


} // makeFieldFormatString_




//
// Adds fields to the given DDR that correspond to what's in the schema.
//
// XXX Always returns true.  Need to add error handling.
//
static
bool
addSchema_( sio_8211DDR& ddr,  sio_8211Schema const & schema )
{

  sio_8211DDRField ddr_field;

  string subfield_labels;
  string subfield_formats;


  for ( sio_8211Schema::const_iterator field_format_itr = schema.begin();
        field_format_itr != schema.end();
        field_format_itr++ )
  {
    // field control stuff first
    ddr_field.setDataStructCode( (*field_format_itr).getDataStructCode() );
    ddr_field.setDataTypeCode( (*field_format_itr).getDataTypeCode() );

    // data field name
    ddr_field.setDataFieldName( (*field_format_itr).getName() );


    // Now we need to add the subfield stuff.  First we'll add in the
    // labels.  Then we'll add the subfield formats.

    // First we reset the subfield label string.  The first character will
    // be an '*' if we're dealing with a vector or array field.

    switch ( field_format_itr->getDataStructCode() )
    {
    case sio_8211FieldFormat::elementary : // should be 8211 reserved field

        ddr.addField( (*field_format_itr).getTag(), ddr_field.getField() );
        continue;               // no other special work needs to be done
        break;                  // for these fields, so continue to the next
                                // field format

    case sio_8211FieldFormat::array :

                                // this ridiculousness added to work
                                // around a bizarre bug related to
                                // basic_string::assign().
                                // Unfortunatley, I'm having
                                // difficulty separately reproducing
                                // this error.  Thanks to
                                // BoundsChecker for nailing this one.

        subfield_labels.resize( 256 ); 
        subfield_labels.assign( "*" );

        break;

    case sio_8211FieldFormat::vector : 
    case sio_8211FieldFormat::concatenated : // fall through
        subfield_labels.assign( "" );
        break;

    default :
        return false;
        break;
    }


    // Now we grind through the subfield labels appending each one
    // to the subfield label string.  However, we add the first one
    // separately because we don't want to add in an extra '!' between
    // subfield labels.

    sio_8211FieldFormat::const_iterator sf_itr = (*field_format_itr).begin();
    subfield_labels.append( (*sf_itr).getLabel() );
    sf_itr++;

    for ( ;
          sf_itr != (*field_format_itr).end();
          sf_itr++ )
    {
      subfield_labels.append( "!" );
      subfield_labels.append( (*sf_itr).getLabel() );

    }


    ddr_field.setArrayDescriptor( subfield_labels );

    // ... then the format string
    subfield_formats = "";
    makeFieldFormatString_( *field_format_itr, subfield_formats );

    ddr_field.setFormatControls( subfield_formats );

    // we're done making the field, so just add it to the DDR
    ddr.addField( (*field_format_itr).getTag(), ddr_field.getField() );


  }


  return true;

} // addSchema_



//
// Create all the fields that go into the DDR.  This means adding
// the two reserved DDR fields, one for the file title and the other
// for the record identfier.  Then the schema, which contains all
// the field and subfield information, will have to have appropriate
// DDR fields added.
//
bool
sio_8211Writer::makeDDR_()
{
  // add reserved fields

  sio_8211FileTitleField file_title_field( imp_->title_ );

  imp_->ddr_.addField( string("0000"), file_title_field.getField() );


#ifdef SDTSXXDEBUG

  cout << __FILE__ << ":" << __LINE__ << "\n";

  ostream_iterator<sio_8211FieldFormat> ostr_it( cout, "\n" );

  copy( imp_->schema_.begin(), imp_->schema_.end(), ostr_it );

#endif

  // make fields from schema

  return addSchema_( imp_->ddr_, imp_->schema_ );

} // sio_8211Writer::_makeDDR()





//
// Add the sc_field to the 8211 DR.
//
// 1. find the field format in the schema that matches the sc_field
// 2. grind through all the sc_subfields in sc_field, blatting out the
//    data into an sio_ostreamBuffer using the corresponding 
//    sio_8211SubfieldFormat
// 3. add the field to the DR -- use the sc_field mnemonic and the
//    sio_ostreamBuffer for the 8211 field tag and 8211 field data, respectively
//
// Notes:
//
//   It is possible for some subfields to be missing.  Which is no big deal
//   since some subfields are optional.  In that case, the current subfield 
//   format's mnemonic won't match the current subfield mnemonic.  We blat out
//   an empty subfield value appropriate for the type, and move on to the
//   next subfield format.
//
static
bool
addField_( sio_8211Schema const & schema, 
           sc_Field const & sc_field, 
           sio_8211DR& dr,
           bool isRepeating = false )
{

  // find the field format <1> 
  sio_8211Schema::const_iterator field_format = 
    find( schema.begin(), schema.end(), sc_field.getMnemonic() );

  if ( field_format == schema.end() ) 
  {
#ifdef STDSXXDEBUG
    cerr << "didn't find " << sc_field.getMnemonic() << "\n";
#endif
    return false;
  }

  // SDTS attributes treat their subfields differently.

  bool is_attribute  = ( (*field_format).getTag() == "ATTP" ||
                         (*field_format).getTag() == "ATTS" );


  sc_Field::const_iterator   sc_subfield_itr = sc_field.begin();
  sio_8211FieldFormat::const_iterator sf_itr = (*field_format).begin();

  sio_Buffer buffer; // contains the 8211 data area

  // is true if the last subfield is variable and not fixed length;
  // this is only important when writing repeating fields as fields
  // that end with a fixed length subfield don't need to have a final
  // field terminator
  bool is_last_subfield_variable = true;

  // Step <2>
  for ( ;; )
  {
    // the subfield label and the subfield format match -- so
    // use the subfield format's converter to add the raw
    // subfield data to the buffer

    string sc_subfield_label;

    if ( is_attribute )
    {
      sc_subfield_label = (*sc_subfield_itr).getName();
    }
    else
    {
      sc_subfield_label = (*sc_subfield_itr).getMnemonic();
    }

    if ( (*sf_itr).getLabel() == sc_subfield_label )
    {
#ifdef SDTSXXDEBUG
      cout << (*sf_itr) << endl;
#endif
      if ( (*sf_itr).getConverter() )
      {
        if ( sio_8211SubfieldFormat::fixed == (*sf_itr).getFormat() )
        {
          (*sf_itr).getConverter()->addFixedSubfield( *sc_subfield_itr, 
                                                      (*sf_itr).getLength(), 
                                                      buffer );
        }
        else
        {
          (*sf_itr).getConverter()->addSubfield( *sc_subfield_itr, buffer );
        }
      }
      sc_subfield_itr++;
    }
    // else, the don't match -- which means that the subfield is missing
    // for whatever reason, so blat out an empty value for it
    else
    {
#ifdef SDTSXXDEBUG
      cout << "missing: " << (*sf_itr) << endl;
#endif
      if ( (*sf_itr).getConverter() )
      {
        (*sf_itr).getConverter()->addEmptySubfield( buffer );
      }
    }

    is_last_subfield_variable = 
       ( sio_8211SubfieldFormat::variable == (*sf_itr).getFormat() ) ?
       true : false;

                                // if there are no more records, exit,
                                // skipping the addition of the unit
                                // terminator (after all, we don't
                                // want to have final unit terminator
                                // and field terminator pairs as this
                                // can confuse legacy readers.

    sf_itr++;

    if ( (sf_itr          == field_format->end())  ||
         (sc_subfield_itr == sc_field.end()) )
    {
       break;
    }

    // add a unit terminator, if necessary
    if ( sio_8211SubfieldFormat::variable == (*sf_itr).getFormat() )
    {
      buffer.addData( sio_8211UnitTerminator );
    }

  } // loop while there are subfields


  // All fields MUST end in a field terminator, so add one.

  buffer.addData( sio_8211FieldTerminator );

  // Step <3>

  if ( isRepeating )
  {
     dr.addRepeatingField( buffer, is_last_subfield_variable );
  }
  else
  {
     dr.addField( (*field_format).getTag(), buffer );
  }

  return true;

} // addField_







// Will blat out the given record identifier field to the given DR
static
bool
writeRecIdenField_( sio_8211RecordIdentifierField const & recIdenField,
                   sio_8211DR & dr )
{
  // again, "0001" is the reserved field label for the
  // ISO 8211 record identifier field 
  return dr.addField( "0001", recIdenField.recordNum() );
} // writeRecIdenField





//
// write out the given record to the writer's output stream
//
bool 
sio_8211Writer::put( sc_Record& sc_record )
{
  sio_8211DR  dr;

                                // if there exists a "0001" DDF record
                                // identifier field format in the
                                // schema, then first add and update
                                // that field to the outgoing DR.

  if ( imp_->haveRecIdenField_ )
    {
      if ( ! writeRecIdenField_( imp_->recIdenField_, dr ) )
        {
          return false;
        }
      imp_->recIdenField_.recordNum()++;
    }


  sc_Record::const_iterator sc_field_itr = sc_record.begin();

  addField_( imp_->schema_, *sc_field_itr, dr );

  sc_Record::const_iterator sc_prev_field_itr = 
    sc_field_itr;

  sc_field_itr++;

  for( ;
       sc_field_itr != sc_record.end();
       sc_field_itr++, sc_prev_field_itr++ )
  {
    if (  (*sc_field_itr).getMnemonic() == 
          (*sc_prev_field_itr).getMnemonic() )
    { // if the field is the same as the previous one, then it's repeating
      if ( ! addField_( imp_->schema_, *sc_field_itr, dr, true ) )
      {
        // something bad happened -- more than likely a schema was
        // messed up
        imp_->ofs_.setstate( ios::badbit );
        return false;
      }
    }
    else
    {
      if ( ! addField_( imp_->schema_, *sc_field_itr, dr ) )
      {
        // something bad happened -- more than likely a schema was
        // messed up
        imp_->ofs_.setstate( ios::badbit );
        return false;
      }
    }

  }

  // If the dropped leader and directory has been set, make sure that
  // the record is in the correct mode.  Note that this flag is set by
  // the invoker via sio_8211Writer::reuseLeaderAndDirectory().  This
  // means that the leader and directory for the current DR will not
  // get written IFF it hasn't already been written -- we know if it
  // has if wroteDroppedLeaderAndDir_ has been set.  Otherwise, this is
  // the first such record, so we need to change the leader's identifier
  // (which is done by called the DR's reuseLeaderAndDirectory() )  to 'R'
  // from the default 'D'.  The leader and directory will then get
  // written out normally.

  if ( imp_->droppedLeaderAndDir_ )
  {
    dr.reuseLeaderAndDirectory(true);
  }


  // If we already wrote the dropped leader and directory, then we
  // also need to let the record know that, too.  Note that this flag
  // is only set here.  This means that the leaders and directories
  // for these records will not get written to the stream -- only
  // the field areas will be.

  if ( imp_->wroteDroppedLeaderAndDir_ )
  {
    dr.wroteDroppedLeaderAndDir( true );
  }


  imp_->ofs_ << dr;  // now blat out the record contents


  // Now if we're writing out records without leaders and directories
  // then we know at this point that we've definitely written out the
  // last leader and directory.  We need to remember that for subsequent
  // put()'s, so set the flag appropriately.  Also, it doesn't hurt
  // to set this each time from now on -- it saves a lookup for each record.

  if ( imp_->droppedLeaderAndDir_ )
  {
    imp_->wroteDroppedLeaderAndDir_ = true;
  }


  return good();

} // sio_8211Writer::put( sc_Record& record )



bool
sio_8211Writer::good() const
{
  return imp_->ofs_.good();
} // sio_Writer::good() const




void
sio_8211Writer::reuseLeaderAndDirectory()
{
  imp_->droppedLeaderAndDir_ = true;
} // sio_8211Writer::reuseLeaderAndDirectory()

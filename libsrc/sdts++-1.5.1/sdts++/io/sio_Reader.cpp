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
// sio_Reader.cpp
//


#include <sdts++/io/sio_Reader.h>

#include <string>
#include <functional>
#include <algorithm>
#include <map>


#ifndef INCLUDED_SIO_8211DIRECTORY_H
#include <sdts++/io/sio_8211Directory.h>
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif

#ifndef INCLUDED_SIO_8211DDRLEADER_H
#include <sdts++/io/sio_8211DDRLeader.h>
#endif

#ifndef INCLUDED_SIO_8211DDR_H
#include <sdts++/io/sio_8211DDR.h>
#endif

#ifndef INCLUDED_SIO_8211DR_H
#include <sdts++/io/sio_8211DR.h>
#endif

#ifndef INCLUDED_SIO_8211DDRFIELD_H
#include <sdts++/io/sio_8211DDRField.h>
#endif

#ifndef INCLUDED_SIO_8211FIELD_H
#include <sdts++/io/sio_8211Field.h>
#endif

#ifndef INCLUDED_SIO_CONVERTER_H
#include <sdts++/io/sio_Converter.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif


#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

using namespace std;




static const char* iden_ = "$Id: sio_Reader.cpp,v 1.21 2003/06/06 21:30:43 mcoletti Exp $";


//
// The guts for an sio_8211Reader object.
//
struct sio_8211Reader_Imp
{
  sio_8211Reader_Imp( istream& file ) : file_( file ) {}


  istream& file_;               // stream to an open 8211 file; the user has
                                // responsibility of managing this stream
  sio_8211DDR ddr_;

  field_format_ctr fieldFormats_;

  long DRStart_;                // offset for first DR

}; // struct sio_8211Reader_Imp






//
// Read in the 8211 DDR.  After it's read in, the ddr_ object will be
// ready for use in reading the 8211's data records.
//
static
bool
readDDR_( sio_8211Reader_Imp& reader_imp,
          sio_8211_converter_dictionary const * const converters )
{
  if ( ! reader_imp.file_ ) { return false; }

  reader_imp.file_ >> reader_imp.ddr_; // slurp in the DDR

  if ( ! reader_imp.file_ )
    {
      return false;
    }

  // After we read the DDR, the stream pointer will be sitting at the first
  // character in the first DR.  Remember that location in DRStart_.
  reader_imp.DRStart_ = reader_imp.file_.tellg();

  // Now we need to build the field format "dictionary" that we'll later
  // use to properly parse a DR.

  for ( sio_8211Directory::const_iterator dir_entry =
          reader_imp.ddr_.getDirectory().begin();
        dir_entry != reader_imp.ddr_.getDirectory().end();
        dir_entry++ )
    {

      if ( (*dir_entry).getTag().substr(0,3) == "000" )
        {
          continue;             // this must be a reserved field, so skip it
        }

                                // build a ddr field given the DDR's leader
                                // and the current field data

                                // XXX This will need to change in the same way
                                // XXX that DDRField changed.  That is, we get
                                // XXX a leader and explicitly morph it into a
                                // XXX DDRLeader.

      sio_8211DDRLeader const * const ddr_leader =
        dynamic_cast<sio_8211DDRLeader const * const>(&(reader_imp.ddr_.getLeader()));

      if ( ! ddr_leader ) return false;


                                // XXX for some reason, I can't pass in
                                // XXX the results of getField() directory to
                                // XXX sio_8211DDRField's ctor.

      sio_8211Field const * field = (*dir_entry).getField();
      sio_8211DDRField ddr_field( *ddr_leader, *field );


      reader_imp.fieldFormats_.push_back( sio_8211FieldFormat() );


      sio_8211MakeFieldFormat( reader_imp.fieldFormats_.back(),
                               ddr_field,
                               (*dir_entry).getTag(),
                               converters );

    } // for each DDR field tag

  return true;

} // readDDR_



sio_8211Reader::~sio_8211Reader()
{
  if ( imp_ ) delete imp_;
} // sio_8211Reader dtor




bool
sio_8211Reader::attach( istream & is,
                        sio_8211_converter_dictionary const * const converters )
{
                                // create a brand spanking new
                                // implementation, giving it the new
                                // stream
  sio_8211Reader_Imp* new_imp = new sio_8211Reader_Imp( is );

  if ( ! new_imp ) return false;

                                // if we have an existing
                                // implementation, copy over its
                                // stuff and then blow it away
  if ( imp_ )
    {
      new_imp->ddr_ = imp_->ddr_;
      new_imp->fieldFormats_ = imp_->fieldFormats_;
      new_imp->DRStart_ = imp_->DRStart_;

      delete imp_;
    }

  imp_ = new_imp;               // now that we have new guts, we need to
                                // read in the DDR in preparation for
                                // reading records

  return readDDR_( *imp_, converters );

} // sio_8211Reader::attach




sio_8211ForwardIterator
sio_8211Reader::begin()
{
  return sio_8211ForwardIterator(*this);
} // sio_8211Reader::begin




field_format_ctr &
sio_8211Reader::getSchema()
{
  return imp_->fieldFormats_;
} // sio_8211Reader::getSchema() const


// fills the given sc_field with the subfields found in the given 8211
// field
//
// Returns: < 0 if error, 0 if ok, > 0 for the field_data_pos if
// there're more fields to read.
//
// Fields of the same type that occur consecutively are called
// repeating fields.  If a field is processed and there's more "space"
// left over, then chances are you are dealing with repeating fields.
// To deal with repeating fields, I return the last data position of
// the current field.  That way the caller can invoke this function
// again with the given data position so that the next field can be
// read.

static
int
fillScField_( sio_8211Field const &       dr_field,
              sio_8211FieldFormat const & field_format,
              sc_Field&                   sc_field,
              long                        field_data_pos = 0 )
{
  vector<char>::const_iterator field_data = dr_field.getData().begin();
  field_data += field_data_pos;

  const long   field_data_length = dr_field.getDataLength();


  sc_field.setMnemonic( field_format.getTag() );
  sc_field.setName( field_format.getName() );

  // SDTS attributes treat their subfields differently.

  bool is_attribute  = (field_format.getTag() == "ATTP" ||
                        field_format.getTag() == "ATTS" );


  sc_Field::iterator sc_subfield_itr = sc_field.begin();


  for ( list<sio_8211SubfieldFormat>::const_iterator subfield_format_itr =
          field_format.begin();
        subfield_format_itr != field_format.end();
        subfield_format_itr++, sc_subfield_itr++ )
    {
      // This function takes into consideration that the client may be
      // reusing a sc_Field container.  If they are, then the iterator
      // will be pointing to the first subfield; in which case, we do nothing
      // here and just over-write the subfield that the iterator points at.
      // On the other hand, this could be a "virgin" sc_Field container --
      // we'll know that if the iterator is pointing off the end of the container.
      // In that case, we just insert a new subfield.  But.  We still need
      // a valid iterator.  So, we do everything in a oner: we insert a new
      // subfield just before the "end" of the container and take the resulting
      // iterator and use that for this current iteration. [1]

      if ( sc_subfield_itr == sc_field.end() )
        {
          sc_subfield_itr = sc_field.insert( sc_field.end(), sc_Subfield() );
        }
      // set the subfield name (or mnemonic) first
      if ( is_attribute )
        {
          (*sc_subfield_itr).setName( (*subfield_format_itr).getLabel() );
        }
      else
        {
          (*sc_subfield_itr).setMnemonic( (*subfield_format_itr).getLabel() );
        }

      // fill the actual subfield value

      if ( (*subfield_format_itr).getConverter() )
        {
          long chunk_size; // how much data was converted in characters

          // now call the converter bound to the current subfield type to
          // convert the raw DR subfield into a proper sc_Subfield
          switch ( (*subfield_format_itr).getFormat() )
            {
            case sio_8211SubfieldFormat::fixed :
              chunk_size =
                (*subfield_format_itr).getConverter()->makeFixedSubfield( *sc_subfield_itr,
                                                                          &*field_data,
                                                                          static_cast<long int>((*subfield_format_itr).getLength()) );
              break;
            case sio_8211SubfieldFormat::variable :
              chunk_size =
                (*subfield_format_itr).getConverter()->makeVarSubfield( *sc_subfield_itr,
                                                                        &*field_data,
                                                                        static_cast<long int>(field_data_length - field_data_pos),
                                                                        (*subfield_format_itr).getDelimiter() );
              field_data++;             // skip unit terminator
              field_data_pos++;
              break;
            }
          field_data     += chunk_size; // increment cursor past the data
          field_data_pos += chunk_size;
        }
      else
        {  // XXX Yes, Virginia, there is such a thing as good error handling.
#ifdef DEBUG
          cerr << (*subfield_format_itr).getLabel()
               << " had no converter" << endl;
#endif
          return -1;
        }
    }

  // if we've re-used this subfield container, then there may be some
  // leftover subfields at the end; if so, clean this up

  if ( sc_subfield_itr != sc_field.end() )
    {
      sc_field.erase( sc_subfield_itr, sc_field.end() );
    }

  // All fields MUST end with a field terminator, even binary fields.
  // However, if we're dealing with a _repeating_ field, then only the
  // last field in the sequence will have a field terminator.
  // Therefore, we check that there's only one character left in the
  // current field; if there is only one, then that's likely the field
  // terminator.  Iff that's the case, increment the data pointer past
  // it.

  // Most of the time it's ok to check for a field terminator while
  // there's still field data to be parsed; however, in the case of
  // binary data, it's possible by happenstance that the current octet
  // will correspond to a field terminator, which would then cause the
  // data pointer to be erroneously incremented.  This is why we first
  // check to insure that there's only one character left in the field
  // before checking that it is indeed a field terminator.

  if ( (1 == field_data_length - field_data_pos )  &&
       (sio_8211FieldTerminator == *field_data) )
    {
      field_data++;             // skip any field terminator
      field_data_pos++;
    }


  // if the field data position is less than the length, then we're likely
  // dealing with a repeating field; so, return the position so that the
  // caller can invoke this function again to pick up the next field.
  // Otherwise, return zero indicating everything is ok.

  return ( field_data_pos < field_data_length ) ? field_data_pos : 0;

} // fillScField_





bool
sio_8211Reader::fillScRecord_( sio_8211DR const & dr, sc_Record& sc_record )
{

  sio_8211Directory const& dr_directory = dr.getDirectory();


  list<sio_8211FieldFormat>::const_iterator field_format_itr;


  list<sc_Field>::iterator sc_field_itr = sc_record.begin();


  long field_data_pos = 0;

  // enumerate each DR field, converting and adding them
  // to the sc_record

  for ( sio_8211Directory::const_iterator dir_entry = dr_directory.begin();
        dir_entry != dr_directory.end();
        dir_entry++ )
    {
      // find the field format
      field_format_itr = find( imp_->fieldFormats_.begin(), imp_->fieldFormats_.end(),
                               (*dir_entry).getTag() );

      if ( field_format_itr != imp_->fieldFormats_.end() )
        {
          sio_8211Field const * dr_field = (*dir_entry).getField();

          do
            {
                                // if we're not reusing an existing record
                                // and it's empty, then add the first field
                                // (see [1], above -- the same kind of thing
                                // is going on here)

              if ( sc_field_itr == sc_record.end() )
                {
                  sc_field_itr = sc_record.insert( sc_record.end(), sc_Field() );
                }


              field_data_pos = fillScField_( *dr_field,
                                             (*field_format_itr),
                                             (*sc_field_itr),
                                             field_data_pos );

              if ( field_data_pos < 0 )
              {
                 return false;
              }

              sc_field_itr++;

            }
          while ( field_data_pos > 0 ); // while we've got repeating fields
        }
      else
        {                       // XXX Error handling, anyone?  (But for
                                // XXX reserved tags, this is ok.)
#ifdef DEBUG
          cerr << "field format for "
               << (*dir_entry).getTag() << " not found" << endl;
#endif
        }


    }

  // snip off any fields that may be left over
  // from the last time the record was used

  if ( sc_field_itr != sc_record.end() )
    {
      sc_record.erase( sc_field_itr, sc_record.end() );
    }

  return true;

} // sio_8211Reader::_fillScRecord()




bool
sio_8211Reader::fillScRecord_( long DRoffset, sc_Record& sc_record )
{
  sio_8211DR dr;

  imp_->file_.seekg( DRoffset );

  imp_->file_ >> dr;

  return fillScRecord_( dr, sc_record );

} // sio_8211Reader::_fillScRecord()




sio_8211Reader::sio_8211Reader()
  : imp_( 0x0 )                 // initially start out with a no implementation
{                               // this can be set later by attach()
} // sio_8211Reader ctor



sio_8211Reader::sio_8211Reader( istream & is,
                                sio_8211_converter_dictionary const * const converters )
  : imp_(  new sio_8211Reader_Imp( is )  )
{
  if ( imp_ )
    {
      readDDR_( *imp_, converters );
    }
  // XXX else we have some problems here
} // sio_8211Reader ctor





//
// Iterator stuff
//




struct
sio_8211ForwardIteratorImp
{

  sio_8211ForwardIteratorImp( );

  sio_8211ForwardIteratorImp( sio_8211ForwardIteratorImp const & );

  sio_8211ForwardIteratorImp& operator=( sio_8211ForwardIteratorImp const & );

  sio_8211ForwardIteratorImp( sio_8211Reader & reader );

  bool attach( sio_8211Reader & reader ); // dock with given reader


  sio_8211Reader* reader_;      // reader iterator currently docked with

  bool isDone_;                 // true iff the iterator is pointing of the
				// DA's "end"  XXX seems kludgy, but makes
				// iterator done() more intuitive and fixes
				// premature finishing behavior.

  long DR_end_;                 // stream positioned just past current
				// DR end

  sio_8211DR DR_;

}; // sio_8211ForwardIteratorImp






sio_8211ForwardIteratorImp::sio_8211ForwardIteratorImp( )
  : reader_( 0x0 ), isDone_( false ), DR_end_( 0 )
{
} // sio_8211ForwardIteratorImp::ctor



sio_8211ForwardIteratorImp::sio_8211ForwardIteratorImp( sio_8211ForwardIteratorImp const & rhs )
  : reader_( rhs.reader_ ),
    isDone_( rhs.isDone_ ),
    DR_end_( rhs.DR_end_ ),
    DR_( rhs.DR_ )
{
} // sio_8211ForwardIteratorImp::ctor




sio_8211ForwardIteratorImp&
sio_8211ForwardIteratorImp::operator=( sio_8211ForwardIteratorImp const & rhs )
{
    if ( this == &rhs ) return *this;

    reader_ = rhs.reader_;
    isDone_ = rhs.isDone_;
    DR_end_ = rhs.DR_end_;
    DR_     = rhs.DR_;

    return *this;
} // sio_8211ForwardIteratorImp::ctor




sio_8211ForwardIteratorImp::sio_8211ForwardIteratorImp( sio_8211Reader & reader )
  : reader_( 0x0 ), isDone_( false ), DR_end_( 0 )
{
  attach( reader );             // dock with the given reader
} // sio_8211ForwardIteratorImp::ctor




bool
sio_8211ForwardIteratorImp::attach( sio_8211Reader & reader )
{
  reader_ = &reader;

                                // first insure that the reader is at
                                // the first DR

  reader_->imp_->file_.seekg( reader_->imp_->DRStart_ );

  reader_->imp_->file_.peek();  // FORCE stream to set fail bit if seek
                                // beyond end


                                // if the stream is in an ok state,
                                // read in the first DR.

  if ( reader_->imp_->file_.good() )
    {
                                // then snarf the first DR
      reader_->imp_->file_ >> DR_;

                                // remember where we are because we'll
                                // need that information
                                // for operator++()

      DR_end_ = reader_->imp_->file_.tellg();

                                // check to see if we're dealing with
                                // dropped leaders

      if ( ! DR_.isReusingLeaderAndDirectory() &&
           'R' == DR_.getLeader().getLeaderIdentifier() )
        {
          DR_.reuseLeaderAndDirectory( true );
        }
    }
  else // in the unlikely event that there are NO DR's ...
    {
      isDone_ = true;
    }


  return reader_->imp_->file_.good();

} // sio_8211ForwardIteratorImp::attach




//
// sio_8211ForwardIterator
//



sio_8211ForwardIterator::sio_8211ForwardIterator()
  : imp_( new sio_8211ForwardIteratorImp() )
{
} // sio_8211ForwardIterator ctor



sio_8211ForwardIterator::sio_8211ForwardIterator( sio_8211Reader & reader )
  : imp_( new sio_8211ForwardIteratorImp( reader ) )
{
} // sio_8211ForwardIterator ctor



sio_8211ForwardIterator::sio_8211ForwardIterator( sio_8211ForwardIterator const & fi )
  : imp_( new sio_8211ForwardIteratorImp( *fi.imp_ ) )
{
} // sio_8211ForwardIterator ctor



sio_8211ForwardIterator&
sio_8211ForwardIterator::operator=( sio_8211ForwardIterator const & rhs )
{
  if ( this == &rhs ) return *this;

  *imp_ = *rhs.imp_;

  return *this;
} // sio_8211ForwardIterator ctor



sio_8211ForwardIterator::~sio_8211ForwardIterator()
{
  if ( imp_ ) { delete imp_; }
} // sio_8211ForwardIterator dtor




bool
sio_8211ForwardIterator::get( sc_Record& record )
{
  // Set the bad bit if the record read failed -- this could have
  // happened because the proper converter couldn't be found,
  // there was no field terminator for one of 8211 records, or some
  // more mundane I/O related problem.
  if ( ! imp_->reader_->fillScRecord_( imp_->DR_, record ) )
    {
      imp_->reader_->imp_->file_.setstate( ios::badbit );
      return false;
    }
  return true;
} // sio_8211ForwardIterator::get



void
sio_8211ForwardIterator::operator++()
{

  // first seek to where we supposedly left off with the last read as another
  // iterator may have moved the stream

  imp_->reader_->imp_->file_.seekg( imp_->DR_end_, ios::beg );
  imp_->reader_->imp_->file_.peek(); // FORCE stream to set fail bit if seek beyond end

  // if the stream is in an ok state, read in the next DR.

  if ( imp_->reader_->imp_->file_.good() )
    {
      imp_->reader_->imp_->file_ >> imp_->DR_;
      imp_->DR_end_ = imp_->reader_->imp_->file_.tellg(); // remember
                                // for next ++

      // check to see if we're dealing with dropped leaders

      if ( ! imp_->DR_.isReusingLeaderAndDirectory() &&
           'R' == imp_->DR_.getLeader().getLeaderIdentifier() )
        {
          imp_->DR_.reuseLeaderAndDirectory( true );
        }

    }
  else  // whoopsies ... no such record, so flag that we're done
    {
      imp_->isDone_ = true;
    }

} // sio_8211ForwardIterator::get




bool
sio_8211ForwardIterator::done() const
{
  return imp_->isDone_;
} // sio_8211ForwardIterator::done




sio_8211ForwardIterator::operator void*() const
{
  return ( done() ) ?  reinterpret_cast<void*>(0x0) :
    reinterpret_cast<void*>(0x1);
} // sio_8211ForwardIterator::done

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
// sio_8211Record.cpp
//

#ifdef NEED_TYPEINFO
#include <typeinfo>
#endif

#include <sdts++/io/sio_8211Record.h>

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211FIELD_H
#include <sdts++/io/sio_8211Field.h>
#endif

#ifndef INCLUDED_SIO_8211FIELDAREA_H
#include <sdts++/io/sio_8211FieldArea.h>
#endif



using namespace std;


sio_8211Record::sio_8211Record()
{
} // sio_8211Recore ctor



static
void
set_forward_pointers_( sio_8211Directory& dir, 
                       sio_8211FieldAreaContainer& fields )
{
    // now reconcile the forward pointers from the directory entries
    // to their corresponding field areas
    
    sio_8211FieldAreaContainer::iterator curr_fa = fields.begin();

    for ( sio_8211DirEntryContainer::iterator curr_dir = dir.begin();
          curr_dir != dir.end();
          curr_dir++, curr_fa++ )
    {
        curr_dir->setField( &(*curr_fa) ); // set the forward link
    }

} // set_forward_pointers





sio_8211Record&
sio_8211Record::operator=( sio_8211Record const & rhs )
{
    if ( &rhs == this ) return *this;

    dir_       = rhs.dir_;
    fieldArea_ = rhs.fieldArea_;

    set_forward_pointers_( dir_, fieldArea_ );

    return *this;
}



sio_8211Record::sio_8211Record( sio_8211Record const & rhs )
    : dir_( rhs.dir_ ), fieldArea_( rhs.fieldArea_ )
{
    set_forward_pointers_( dir_, fieldArea_ );
} // sio_8211Record copy ctors



sio_8211Record::~sio_8211Record()
{
   // Does nothing. Just here so that we can have a virtual destructor.
}



sio_8211Directory const&
sio_8211Record::getDirectory() const
{
   return dir_;
}



sio_8211Directory &
sio_8211Record::getDirectory_()
{
   return dir_;
} // sio_8211Record::getDirectory


sio_8211FieldArea const&
sio_8211Record::getFieldArea() const
{
   return fieldArea_;
}


sio_8211FieldArea &
sio_8211Record::getFieldArea_()
{
   return fieldArea_;
}


void
sio_8211Record::setLeader( sio_8211Leader const& leader )
{
   getLeader_() = leader;

} // sio_8211Record::setLeader()


void
sio_8211Record::setDirectory( sio_8211Directory const& dir )
{
   dir_ = dir;
} // sio_8211Record::setDirectory()


void
sio_8211Record::setFieldArea( sio_8211FieldArea const& fieldArea )
{
   fieldArea_ = fieldArea;
} // sio_8211Record::setFieldArea




bool
sio_8211Record::addField( string const & tag, sio_8211Field const& field )
{
                                // add the field to the field area

   fieldArea_.push_back( field );

                                // create a directory entry with the
                                // given tag; have it refer to
                                // the newly added field

   sio_8211DirEntryContainer::iterator curr_dir_entry = dir_.end();

                                // append to list
   curr_dir_entry = 
     dir_.insert( curr_dir_entry, sio_8211DirEntry( getLeader_() ) );

                                // note that we can use back()
                                // reference as we're referring to the
                                // last element

                                // make sure directory entry can get to field

   dir_.back().setField( &(fieldArea_.back()) ); 
   dir_.back().setTag( tag );

                                // now set the field positions and lengths

   if ( 1 == dir_.size() )
   {
      dir_.back().setPosition( 0 );
   }
   else
   {
      --curr_dir_entry;         // move to previous directory entry

                                // calculate new position as being the
                                // previous directory entry's position
                                // plus that entry's length

      dir_.back().setPosition( 
         (*curr_dir_entry).getPosition() + (*curr_dir_entry).getFieldLength() );
   }

   dir_.back().setFieldLength( field.getDataLength() );

   return true;

} // sio_8211Record::addField



bool
sio_8211Record::addField( string const & tag, sio_Buffer const& field_data )
{
   return addField( tag, sio_8211Field( field_data ) );
} // sio_8211Record::addField()




istream&
operator>>(istream& istr, sio_8211Record& record)
{
   return record.streamExtract(istr);
}

ostream&
operator<<(ostream& ostr, sio_8211Record const& record)
{
   return record.streamInsert(ostr);
}



// Insure that the given leader has the correct record length and
// field area offsets.
void
sio_8211Record::synchLeaderWithRecord_( )
{
   long length = 24;            // initially include the leader length

                                // add the directory size

   length += 
     getDirectory().size() * ( getLeader_().getSizeOfFieldLengthField() +
                               getLeader_().getSizeOfFieldPosField() +
                               getLeader_().getSizeOfFieldTagField() );

                                // add the field area size

   for ( sio_8211FieldAreaContainer::const_iterator i = getFieldArea().begin();
         i != getFieldArea().end();
         i++ )
   {
      length += (*i).getData().size();
   }

                                // The obvious question: "Why + 1?"
                                // The answer is "Because there's a
                                // terminator at the end of the record."

   getLeader_().setRecordLength( length + 1 );

   getLeader_().setBaseAddrOfFieldArea( 24 + getDirectory().size() * 
                                  ( getLeader_().getSizeOfFieldLengthField() +
                                    getLeader_().getSizeOfFieldPosField() +
                                    getLeader_().getSizeOfFieldTagField() ) + 1 );

} // synchLeaderWithRecord_




ostream& 
sio_8211Record::streamInsert(ostream& ostr) const
{
  // We must cast away const as synLeaderWithRecord_() can potentially
  // change the leader information
  sio_8211Record& rec =  const_cast<sio_8211Record&>(*this);

  // insure that the leader is up to date with the record
  
  rec.synchLeaderWithRecord_( );

  ostr << setfill('0');
  ostr << getLeader();
  ostr << getDirectory();
  ostr << getFieldArea();

  return ostr;
} // sio_8211Record::streamInsert()

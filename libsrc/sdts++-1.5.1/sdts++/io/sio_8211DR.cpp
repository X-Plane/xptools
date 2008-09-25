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
// sio_8211DR.cpp
//

#include <sdts++/io/sio_8211DR.h>

#include <vector>


#ifndef INCLUDED_SIO_BUFFER_H
#include <sdts++/io/sio_Buffer.h>
#endif

#ifndef INCLUDED_SIO_8211DRLEADER_H
#include <sdts++/io/sio_8211DRLeader.h>
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211DIRECTORY_H
#include <sdts++/io/sio_8211Directory.h>
#endif


using namespace std;



sio_8211DR::sio_8211DR()
    : reuseLeaderAndDir_(false), wroteDroppedLeaderAndDir_(false)
{
   getDirectory_().setLeader( getLeader_() );
} // ctor




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




sio_8211DR&
sio_8211DR::operator=( sio_8211DR const& rhs )
{
    if ( &rhs == this ) return *this;

    leader_                   = rhs.leader_;
    reuseLeaderAndDir_        = rhs.reuseLeaderAndDir_;
    wroteDroppedLeaderAndDir_ = rhs.wroteDroppedLeaderAndDir_;

    getDirectory_() = rhs.getDirectory();
    getFieldArea_() = rhs.getFieldArea();

    set_forward_pointers_( getDirectory_(), getFieldArea_() );

    return *this;
}


sio_8211DR::sio_8211DR( sio_8211DR const& rhs )
  : leader_( rhs.leader_ ),
    reuseLeaderAndDir_( rhs.reuseLeaderAndDir_ ),
    wroteDroppedLeaderAndDir_( rhs.wroteDroppedLeaderAndDir_ )
{
                                // presumably handled by parent copy ctor
//   getDirectory_() = rhs.getDirectory();
//   getFieldArea_() = rhs.getFieldArea();
   getDirectory_().setLeader( getLeader_() );
} // ctor


sio_8211DR::sio_8211DR(sio_8211DRLeader const& leader,
                       sio_8211Directory const& dir)
          : reuseLeaderAndDir_(true), wroteDroppedLeaderAndDir_(false)
{
   setLeader( leader );
   setDirectory( dir );
   getDirectory_().setLeader( getLeader_() );
} // ctor



void
sio_8211DR::reuseLeaderAndDirectory(bool flag)
{
   if ( reuseLeaderAndDir_ = flag )
   {  // This is harmless if we're reading as the leader identifier
      // should already be set to this value.
      getLeader_().setLeaderIdentifier('R');
   }
}


// If this is set, then the leader and directory will
// not be emitted in streamInsert().  Note that for this
// to happen is like having two keys to open a safe.
// reuseLeaderAndDir_ has to be set as well as this flag.
void
sio_8211DR::wroteDroppedLeaderAndDir( bool flag )
{
   wroteDroppedLeaderAndDir_ = flag;
} // sio_8211DR::wroteDroppedLeaderAndDir( bool flag )



bool
sio_8211DR::isReusingLeaderAndDirectory() const
{
   return reuseLeaderAndDir_;
}



// A repeating field is a field that has multiple instances of the
// same field.  When this is invoked, we want to add the raw data
// contents of the given buffer to the last field, thus making it a
// repeating field.  (Although it may already be one if this function
// was already invoked on it.)  We could__ have just added another
// field, but that would have wasted space by adding another directory
// entry and field terminator.  Another complication is that if the
// last subfield is fixed length, then we don't need the final field
// terminator; otherwise we need to change the field terminator
// written by the last addField() or addRepeatingField to a unit
// terminator so that 8211 readers know that these are repeating
// fields.
bool
sio_8211DR::addRepeatingField( sio_Buffer const& buffer,
                               bool              is_variable )
{

   vector<char>::iterator terminator = getFieldArea_().back().getData().end();
   terminator--;              // (now back up to the field iterator)

   // If variable length subfield, change the field terminator to a
   // unit terminator so that the readers know that this is a
   // repeating field.  If fixed length subfields, then we don't need
   // field terminators between the repearing fields; so erase the one
   // at the end of the previous field.
   if ( is_variable )
   {
      *terminator = sio_8211UnitTerminator;
   }
   else                         // erase the terminator
   {
      getFieldArea_().back().getData().erase( terminator );
   }

                                // now copy the new stuff to the end,
                                // starting from the old field
                                // terminator

   getFieldArea_().back().getData().insert(
      getFieldArea_().back().getData().end(),
      buffer.data().begin(), buffer.data().end() );


                                // make sure that the record's
                                // directory is in the loop and knows
                                // about fatter field

   getDirectory_().back().setFieldLength(
      getFieldArea_().back().getData().size() );

   return true;

} // sio_8211DR::addRepeatingField




istream&
sio_8211DR::streamExtract(istream& istr)
{
   if ( ! istr ) { return istr ; } // bail immediately if problem with stream

   // Assume istr is positioned on byte zero of an ISO8211 DR.

   sio_8211DRLeader tmpLeader;

   if ( ! reuseLeaderAndDir_ )
      {
         // Get the leader.
         istr >> tmpLeader;
         if ( ! istr ) { return istr ; }

         // insure that the leader information is set up properly
         setLeader( tmpLeader );

         // let the directory know about the leader, too
         // XXX Odd because I had to add this as this was
         // XXX working inconsistently.
         getDirectory_().setLeader( getLeader_() );

         // Get the directory.
         istr >> getDirectory_();
         if ( ! istr ) { return istr ; }


      }

   // Erase any previous fields.
   getFieldArea_().clear();


   // Get the new fields.
   long fieldAreaStart = istr.tellg();

   for ( sio_8211Directory::iterator i = getDirectory_().begin();
         i != getDirectory_().end();
         i++)
      {
         istr.seekg(fieldAreaStart + (*i).getPosition());

         // add a new field of the size that the directory says it has to the
         // field area
         getFieldArea_().push_back(sio_8211Field((*i).getFieldLength() - 1));

         istr >> getFieldArea_().back();

         if ( ! istr ) { return istr ; } // bail immediately if problem with stream

         // set the directory's link to its corresponding field.
         (*i).setField(&getFieldArea_().back());
      }


   if ( ! reuseLeaderAndDir_ )
      {
         getLeader_() = tmpLeader;
      }

   return istr;

} // streamExtract()



ostream&
sio_8211DR::streamInsert(ostream& ostr) const
{
   ostr << setfill('0');


   // Don't blat out the leader and directory if we're in
   // dropped leader and directory mode and we already wrote
   // the dropped leader and directory.

   if ( ! (isReusingLeaderAndDirectory() && wroteDroppedLeaderAndDir_) )
   {
     // insure that the leader is up to date with the record

     sio_8211DR* nonconst_dr = const_cast<sio_8211DR*>(this);
     nonconst_dr->synchLeaderWithRecord_( );

     ostr << getLeader();
     ostr << getDirectory();
   }

   ostr << getFieldArea();

   return ostr;

} // sio_8211DR::streamInsert



sio_8211Leader const&
sio_8211DR::getLeader() const
{
   return leader_;
}


sio_8211Leader &
sio_8211DR::getLeader_()
{
   return leader_;
}

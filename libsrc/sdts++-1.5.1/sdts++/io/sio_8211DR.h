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
// sio_8211DR.h
//

#ifndef INCLUDED_SIO_8211DR_H
#define INCLUDED_SIO_8211DR_H


#ifndef INCLUDED_SIO_8211RECORD_H
#include <sdts++/io/sio_8211Record.h>
#endif

#ifndef INCLUDED_SIO_8211DRLEADER_H
#include <sdts++/io/sio_8211DRLeader.h>
#endif


#include <iostream>



class sio_8211Directory;

/// This corresponds to an 8211 Data Record (DR)
class sio_8211DR : public sio_8211Record
{

   public:

      ///
      sio_8211DR();

      ///
      sio_8211DR( sio_8211DR const& rhs );

      ///
      sio_8211DR& operator=( sio_8211DR const& rhs );

      ///
      sio_8211DR(sio_8211DRLeader const& leader,
                 sio_8211Directory const& dir);


      /**
       If we're going to be reading DR's with dropped leaders, we can
       create an sio_8211DR with the leader and directory to be used.
       Sets the reuseLeaderAndDirectory flag to 'true'.
      */
      void reuseLeaderAndDirectory(bool flag);


      /**
       If flag == true, will reuse the current leader and directory.
       If the leader and directory haven't been valued (either by a
       previous read operation or through a constructor), subsequent
       read operations will fail.

       The default behavior is to NOT reuse the leader and directory.
      */
      void wroteDroppedLeaderAndDir( bool flag );


      /**
       Tells the record that a subsequent record put() wrote the
       dropped leader and directory so that it's not necessary to
       write anything but the record's field area.
      */
      bool isReusingLeaderAndDirectory() const;


      /**
       Returns true if we're reusing the leader and directory for each
       read operation.

       Append the given field to the last field (i.e., it's a repeating
       field).  The corresponding directory entry will automatically
       increase by that field's size.  Also must specify whether field
       is variable or fixed; if it's fixed, then the repeating fields
       don't need to be delimited by a a field terminator, otherwise
       they have to have one.
      */
      bool addRepeatingField(sio_Buffer const& field_data, bool is_variable);


      ///
      sio_8211Leader const& getLeader() const;


   protected:
      ///
      virtual sio_8211Leader& getLeader_();


   private:

      ///
      std::istream& streamExtract(std::istream& istr);

      ///
      std::ostream& streamInsert(std::ostream& ostr) const;

      ///
      sio_8211DRLeader  leader_;

      ///
      bool              reuseLeaderAndDir_;

      ///
      bool              wroteDroppedLeaderAndDir_;

}; // sio_8211DR

#endif  // INCLUDED_SIO_8211DR_H

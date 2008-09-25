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
// sio_8211Record.h
//


#ifndef INCLUDED_SIO_8211RECORD_H
#define INCLUDED_SIO_8211RECORD_H

#include <iostream>



#ifndef INCLUDED_SIO_8211DIRECTORY_H
#include <sdts++/io/sio_8211Directory.h>
#endif

#ifndef INCLUDED_SIO_8211FIELDAREA_H
#include <sdts++/io/sio_8211FieldArea.h>
#endif

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif


class sio_8211Field;
class sio_Buffer;



/// Abstract class. Represents the commonality between a DDR and a DR.
class sio_8211Record
{
   public:

      ///
      sio_8211Record();

      ///
      sio_8211Record( sio_8211Record const & );

      ///
      sio_8211Record& operator=( sio_8211Record const & );

      ///
      virtual ~sio_8211Record();

      /**
       Pure virtual because child classes define their own leaders.  That
       is, sio_8211DR has an sio_8211DRLeader and an sio_8211DDR has an
       sio_8211DDRLeader.
      */
      virtual sio_8211Leader const& getLeader() const = 0;

      ///
      sio_8211Directory const& getDirectory() const;

      ///
      sio_8211FieldArea const& getFieldArea() const;

      /// Add a field to the record that has the given tag.
      bool addField(std::string const & tag, sio_8211Field const& field);

      ///
      bool addField(std::string const & tag, sio_Buffer const& field_data);

      friend std::istream& operator>>(std::istream& istr, sio_8211Record& record);
      friend std::ostream& operator<<(std::ostream& ostr, sio_8211Record const & record);

   protected:

      ///
      virtual std::istream& streamExtract(std::istream& istr) = 0;

      ///
      virtual std::ostream& streamInsert(std::ostream& ostr) const;

      ///
      virtual void setLeader( sio_8211Leader const& );

      ///
      void setDirectory( sio_8211Directory const& );

      ///
      void setFieldArea( sio_8211FieldArea const& );

      /// non-const version for streamExtract()
      sio_8211Directory& getDirectory_();

      ///
      sio_8211FieldArea& getFieldArea_();

      ///
      virtual sio_8211Leader& getLeader_() = 0;

      /**
       Utility for making sure that the given leader accurately
       reflects the current state of the record.  That is, the
       various lengths are all updated appropriately.
      */
      virtual void synchLeaderWithRecord_( );

   private:

      ///
      sio_8211Directory    dir_;

      ///
      sio_8211FieldArea    fieldArea_;

}; // class sio_8211Record


///
std::istream&
operator>>(std::istream& istr, sio_8211Record& record);

///
std::ostream&
operator<<(std::ostream& ostr, sio_8211Record const & record);

#endif  // INCLUDED_SIO_8211RECORD_H

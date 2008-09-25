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

#ifndef INCLUDED_SIO_8211FIELDAREA_H
#define INCLUDED_SIO_8211FIELDAREA_H

#include <list>


#ifndef INCLUDED_SIO_8211FIELD_H
#include <sdts++/io/sio_8211Field.h>
#endif


///
typedef std::list<sio_8211Field> sio_8211FieldAreaContainer;



/// This corresponds to an 8211 field area
class sio_8211FieldArea : public std::list<sio_8211Field>
{
   public:

      friend std::ostream& operator<<(std::ostream& ostr, sio_8211FieldArea const& area);

   protected:

      ///
      std::ostream& streamInsert(std::ostream& ostr) const;
};

///
std::ostream&
operator<<(std::ostream& ostr, sio_8211FieldArea const& area);

#endif  // INCLUDED_SIO_8211FIELDAREA_H


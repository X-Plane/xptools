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
// sio_Error.h
//

#ifndef INCLUDED_SIO_ERROR_H
#define INCLUDED_SIO_ERROR_H



/**
 Mix-in error reporting class.  Patterned after ios error handling.
 
 XXX Consider adding another state, one that contains specific error
 XXX information.  I.e., like the various system call error codes.
 XXX May even want to augment with a string that can be set to
 XXX diagnostic text.
*/
class sio_Error
{
   public:

      ///
      enum { goodbit = 0x0, badbit = 0x1, failbit = 0x2 };

      ///
      sio_Error();

      /// true if object has no problems
      virtual bool good() const;

      /// true if object is in an unusable state
      virtual bool bad() const;

      /// true if the object is in a fail state; it's still usable if not bad()
      virtual bool fail() const;

      /// returns the error flags
      virtual int  rdstate() const;

      /// sets the error state to the given bit mask; defaults to good()
      virtual int  setstate( int state = goodbit );

      /// resets to good state
      virtual int  clear();

   private:

      ///
      int state_;

}; // sio_Error


#endif

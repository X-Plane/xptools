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
// sio_Buffer.h
//


#ifndef INCLUDED_SIO_BUFFER_H
#define INCLUDED_SIO_BUFFER_H


#include <vector>



class sio_BufferImp;


/**
 We needed something that would allow us to accumulate raw binary
 data in a convenient way.  That is, we'd like to add raw data a
 piece at a time to some sort of container.  This container would
 handle all the memory management for us.  This is that class.

 In a way, the name "sio_Buffer" is mis-leading.  We imply by using
 "buffer" that the intent is to arbitrarily add and
 remove data in the container -- much like a stream.  However, we'll
 always be adding data to an sio_Buffer; and then asking its value
 in a non-destructive way.  That is, add and query, never add and remove.
*/
class sio_Buffer
{
   public:

      ///
      sio_Buffer();

      ///
      sio_Buffer( std::vector<char> const& );

      ///
      sio_Buffer( char const* data, long length );

      ///
      ~sio_Buffer();

      ///
      sio_Buffer( sio_Buffer const& );

      /// flush the buffer
      bool reset();

      // append new data to the buffer; returns true if added ok
      bool addData( const char * pos, long length );

      // append new data to the buffer; returns true if added ok
      bool addData( char data );

      // return a pointer to the current data
      std::vector<char> const& data() const;

      // return a pointer to the current data
      std::vector<char> & data() ;

      ///
      long length() const;

   private:

      ///
      sio_BufferImp* imp_;

}; // class sio_Buffer


#endif // INCLUDED_SIO_BUFFER_H

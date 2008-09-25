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
// sio_Buffer.cpp
//

#include <sdts++/io/sio_Buffer.h>


#include <algorithm>


//
// Implementation for sio_Buffer.
//


class sio_BufferImp
{
public:

  sio_BufferImp( )
    {
    }

  sio_BufferImp( sio_BufferImp const & rhs )
    : buffer_( rhs.buffer_ )
    {
    }

  sio_BufferImp( std::vector<char> const& data )
    : buffer_( data )
    {
    }

  sio_BufferImp( const char* data, long length )
    {
      buffer_.resize( length );
      std::copy( &data[0], &data[length], buffer_.begin() );
    }


  bool reset()
    {
      buffer_.clear();
      return true;
    }

  std::vector<char> const& data() const { return buffer_; }
  std::vector<char>      & data()       { return buffer_; }

  long length() const { return buffer_.size(); }

  // append ``data'' of ``length'' to the buffer

  bool write( const char * pos, long length )
    {
      buffer_.insert( buffer_.end(), pos, pos + length );
      return true;
    }


  bool write( char data )
    {
      return write( &data, 1 );
    }

private:

  sio_BufferImp& operator=( sio_BufferImp const & ); // NOT NEEDED

  std::vector<char> buffer_;

}; // sio_BufferImp








sio_Buffer::sio_Buffer()
  : imp_( new sio_BufferImp() )
{
} // sio_Buffer ctor


sio_Buffer::sio_Buffer( std::vector<char> const& data )
  : imp_( new sio_BufferImp( data ) )
{
} // sio_Buffer ctor


sio_Buffer::sio_Buffer( const char* data, long length )
  : imp_( new sio_BufferImp( data, length ) )
{
} // sio_Buffer ctor


sio_Buffer::sio_Buffer( sio_Buffer const& rhs )
  : imp_( new sio_BufferImp( *rhs.imp_ ) )
{
} // sio_Buffer copy ctor



sio_Buffer::~sio_Buffer()
{
  delete imp_;
} //sio_Buffer dtor





bool
sio_Buffer::addData(char data)
{
  return imp_->write( data );
} // sio_Buffer::addData



bool
sio_Buffer::addData( const char * pos, long length)
{
  return imp_->write( pos, length );
} // sio_Buffer::addData



bool
sio_Buffer::reset()
{
  return imp_->reset();
} // sio_Buffer::reset


std::vector<char> const&
sio_Buffer::data() const
{
  return imp_->data();
} // sio_Buffer::data


std::vector<char> &
sio_Buffer::data()
{
  return imp_->data();
} // sio_Buffer::data



long
sio_Buffer::length() const
{
  return imp_->length();
} // sio_Buffer::length

//
// sio_8211Field.cpp
//

#include <sdts++/io/sio_8211Field.h>


#include <algorithm>

#ifndef INCLUDED_SIO_BUFFER_H
#include <sdts++/io/sio_Buffer.h>
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211DDRLEADER_H
#include <sdts++/io/sio_8211DDRLeader.h>
#endif

#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif


using namespace std;


sio_8211Field::sio_8211Field()
  : size_( 0 ), data_()
{
}


sio_8211Field::sio_8211Field( sio_8211Field const & rhs)
  : size_( rhs.size_ ), data_( rhs.data_ )
{
}



sio_8211Field::sio_8211Field( long fieldSize )
  : size_( fieldSize )
{
}


sio_8211Field::sio_8211Field( sio_Buffer const& buffer )
  : data_( buffer.data() ), size_( buffer.length() )
{
}




// return how much data is in the field; not that we're careful not to
// return capacity() as what's reserved may in fact be larger than the
// actual amount of data assigned to the field
long
sio_8211Field::getDataLength() const
{
   return size_;
}



void
sio_8211Field::setDataLength( int l )
{
  size_ = l;
}



vector<char> const&
sio_8211Field::getData() const
{
   return data_;
}



vector<char> &
sio_8211Field::getData()
{
   return data_;
}



bool
sio_8211Field::setData( vector<char> const& data )
{
  data_ = data;
  size_ = data.size();

  return true;
}



bool
sio_8211Field::addData( vector<char> const& data )
{
  data_.insert( data_.end(), data.begin(), data.end() );
  size_ = data_.size();

  return true;
}



sio_Buffer
sio_8211Field::getField() const
{
    return sio_Buffer( data_ );
} // sio_8211Field



bool
sio_8211Field::getVariableSubfield( vector<char>& value,
                                    long& startPos,
                                    char delim ) const
{
   // Attempt to extract a character subfield into ``value''.  We will
   // stop once we get to ``delim'' in our subfield, or when we reach
   // the length of data. The delimiter is omitted.


				// If the start position isn't within
				// the data, then something's wrong;
				// so bail.

  if ( ! ((startPos >= 0) && (startPos < data_.size() )) )
    {
      return false;
    }

                                // copy all characters from the data
                                // buffer up to, but not including,
                                // the unit terminator/delim

  value.clear();                // start with clean slate

  for ( vector<char>::const_iterator i = data_.begin() + startPos;
        i != data_.end() && (*i) != delim;
        i++, startPos++ )
    {
      value.push_back( *i );    // append string to value
    }

  startPos++;                   // increment past delimiter

   return true;
} // sio_8211Field::getVariableSubfield.





istream&
sio_8211Field::streamExtract( istream& istr )
{
  data_.resize( size_ );        // insure that the buffer's large enough;
                                // naturally we're presuming that size_
                                // has been properly set ahead of time

                                // then read it in and we're done

  istr.read( &data_[0], size_ );

                                // gobble the field terminator

  if ( sio_8211FieldTerminator != istr.get() )
    {
      istr.setstate( ios::badbit );
    }

  return istr;
}  // sio_8211Field::streamExtract





ostream&
sio_8211Field::streamInsert(ostream& ostr) const
{

   // Assume istr is positioned at the start of where this field
   // should be written.

   ostr.write( &data_[0], data_.size() );

   return ostr;
} // sio_8211Field::streamInsert




istream&
operator>>(istream& istr, sio_8211Field& field)
{
   return field.streamExtract(istr);
}




ostream&
operator<<(ostream& ostr, sio_8211Field const& field)
{
   return field.streamInsert(ostr);
}




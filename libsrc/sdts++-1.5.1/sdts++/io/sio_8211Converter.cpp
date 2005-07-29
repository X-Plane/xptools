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
//  sio_8211Converter.cpp
//


#include <strstream>
#include <iomanip>

#include <algorithm>

// BAS CHANGED 6/24/04 - On Mac onn-unix platforms, netinet is not available
// for byte swapping....just #define out the macros since all Carbon-capable macs
// are network-endian.

#if defined(_MSC_VER) || (defined(__MWERKS__) && defined(__INTEL__))
	#include <winsock2.h>
#elif defined(__POWERPC__) && !defined(__MACH__)
	#define htonl(x) (x)
	#define htons(x) (x)
	#define ntohl(x) (x)
	#define ntohs(x) (x)
#else
	#include <sys/types.h>
	#include <netinet/in.h>
#endif

#ifndef INCLUDED_SIO_CONVERTER_H
#include <sdts++/io/sio_Converter.h>
#endif

#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif

static const char* ident_ = "$Id: sio_8211Converter.cpp,v 1.8 2002/11/24 22:07:43 mcoletti Exp $";

static const long TwoToTheEigth = 256;
static const long TwoToTheSixteenth = 65536;
static const long TwoToTheTwentyFourth = 16777216;


using namespace std;


// Notes: makeSubfield(...) and makeFixedSubfield(...) take 
// information from an array of characters ("data"), and stores the 
// appropriate value in the subfield. addSubfield(...) gets the value 
// from the subfield, and adds it to a buffer, where the subfield 
// gets built. 

// If you just want to access the datum stored in a subfield, use the 
// subfield accessor methods, like getBI24(...) and setBFP32(...). See 
// container/sc_Subfield.h for the specifics.

sio_8211Converter::~sio_8211Converter()
{
}


long
sio_8211Converter::makeVarSubfield(sc_Subfield& subfield,
                                   char const* data,
                                   long maxLength,
                                   char delimiter) const
{
  // This is the default implementation. Subclasses can override to make
  // it more efficient.

  long length = findVariableSubfieldLength(data,maxLength,delimiter);
  return makeFixedSubfield(subfield, data, length);
}

long
sio_8211Converter::addEmptySubfield(sio_Buffer& buffer) const
{
  // As the name suggests, this adds an empty subfield.
  char foo[1];
  char const* foo2;
  long retval;

  foo[0] = sio_8211UnitTerminator;
  foo2 = foo;

  if(buffer.addData(foo,1))
    retval = 1;
  else
    retval = 0;

  return retval;
}

long
sio_8211Converter::findVariableSubfieldLength(char const* data,
                                              long maxLength,
                                              char delimiter) const
{
  // This is useful for any field where the data is stored as an
  // ASCII (or ISO8859-1, or whatever) string, such as types A, I, 
  // R, and S.
  // The length returned does not include the delimiter that follows
  // the field.
  char const* pos = data;
  long count = 0;

  while ((count < maxLength) && (*pos != delimiter))
  {
    pos++;
    count++;
  }

  return count;

}


// sio_8211Converter_A
// Type "A" data is a character string, typically in ASCII or ISO8859
// format (one octet per character encodings), such as "Foo bar baz".
long
sio_8211Converter_A::makeFixedSubfield(sc_Subfield& subfield,
                                       char const* data,
                                       long length) const
{
  // Convert the raw data into a string and create an sc_Subfield.
  // ASCII/ISO8859 -> subfield.

  if ( length )
  {
    char* tmpBuf = new char[length + 1];
    memcpy(tmpBuf,data,length);
    tmpBuf[length] = '\0';
    
    subfield.setA(tmpBuf);
    delete [] tmpBuf;
  }
  else                          // this is an empty subfield
  {
    subfield.setA("");         // set the type
    subfield.setUnvalued();    // state that it has no value
  }

  return length;
}

long
sio_8211Converter_A::addSubfield(sc_Subfield const& subfield,
                                 sio_Buffer& buffer) const
{
  string val;

  if ( ! subfield.getA(val) ) { return 0; } // is null
  buffer.addData( val.c_str(), val.length() ); 
  return 0;
}

long
sio_8211Converter_A::addFixedSubfield(sc_Subfield const& subfield,
                                      long length,
                                      sio_Buffer& buffer) const
{
  string val;
  subfield.getA(val);

  buffer.addData( val.c_str(), min( length, static_cast<long>(val.length()) ) ); 

  return 0;
}


// sio_Converter_I
// Type "I" data is a character representation of integer data.
// (Such as "123456")
long
sio_8211Converter_I::makeFixedSubfield(sc_Subfield& subfield,
                                       char const* data,
                                       long length) const
{
  // Convert the raw data into a long and create an sc_Subfield.

  if ( length )
  {
    strstream ss;
    long foo = 0;
    char* tmpBuf = new char[length + 1];
    memcpy(tmpBuf,data,length);
    tmpBuf[length] = '\0';
    ss << tmpBuf;
    ss >> foo;
   
    subfield.setI(foo);
    delete [] tmpBuf;
  }
  else
  {
    subfield.setI( static_cast<long>(0) ); // lock type
    subfield.setUnvalued();    // show that it has no value

  }
  
return length;

}

long
sio_8211Converter_I::addSubfield(sc_Subfield const& subfield,
				 sio_Buffer& buffer) const
{
  // Turn the stored value back into an character string.
  strstream val;
  string foo;
  long tempint;

  if ( ! subfield.getI(tempint) ) { return 0; } // is null

  val << tempint;
  val >> foo;

  buffer.addData(foo.c_str(), foo.length()); 

  return 0;
}

long
sio_8211Converter_I::addFixedSubfield(sc_Subfield const& subfield,
				      long length,
				      sio_Buffer& buffer) const
{
  strstream val;
  val.width(length);
  val.fill( '0' );

  long tempint;
  if ( ! subfield.getI(tempint) ) return -1; // could be null, but fixed
                                // so this is an error
  val << tempint;

  string tmp_str;

  getline( val, tmp_str );

  buffer.addData( tmp_str.c_str(), length ); 

  return 0;
}

// sio_Converter_R
// Type "R" data is character representation of unscaled floating 
// point data. (Such as "123.456")

long
sio_8211Converter_R::makeFixedSubfield(sc_Subfield& subfield,
				       char const* data,
				       long length) const
{
  // Convert the raw data into a double and create an sc_Subfield.

  if ( length )
  {
    char* tmpBuf = new char[length + 1];
    strstream ss;
    double foo = 0.0;
    memcpy(tmpBuf,data,length);
    tmpBuf[length] = '\0';
    ss << tmpBuf;
    ss >> foo;
    subfield.setR(foo);
    delete [] tmpBuf;
  }
  else
  {
    subfield.setR( 0.0 );
    subfield.setUnvalued();
  }

  return length;
}

long
sio_8211Converter_R::addSubfield(sc_Subfield const& subfield,
				 sio_Buffer& buffer) const
{
  //Use this version if you want all the stored digits.
  double tempdouble;
  strstream val;
  string foo;

  if ( ! subfield.getR(tempdouble) ) { return 0; } // could be null

  val.setf( ios::fixed ); 
  val.precision(8);

  val << tempdouble;
  val >> foo;

  buffer.addData(foo.c_str(), foo.length()); 
  return 0;
}


long
sio_8211Converter_R::addFixedSubfield(sc_Subfield const& subfield,
				      long length,
				      sio_Buffer& buffer) const
{
  //Use this version if you only want length digits worth of answer.
  double tempdouble;
  strstream val;

  if ( ! subfield.getR(tempdouble) ) { return -1; } // could be null, but fixed
                                // so this is an error

  val.setf( ios::left | ios::fixed );
  val.fill('0');               // XXX for S types
  val << setw(length) << tempdouble;

  buffer.addData(val.str(), length); 

  val.freeze(0);               // make sure that we free up memory

  return 0;
}

// sio_Converter_S
// Type "S" data is character representation of scaled floating point
// data (such as "1.23456+E002")

long
sio_8211Converter_S::makeFixedSubfield(sc_Subfield& subfield,
				       char const* data,
				       long length) const
{
  // Convert the raw data into a double and create an sc_Subfield.

  if ( length )
  {
    char* tmpBuf = new char[length + 1];
    strstream ss;
    double foo = 0.0;
    memcpy(tmpBuf,data,length);
    tmpBuf[length] = '\0';
    ss << tmpBuf;
    ss >> foo;
    
    subfield.setS(foo);
    delete [] tmpBuf;
  }
  else
  {
    subfield.setS( 0 );
    subfield.setUnvalued();
  }

  return length;
}

long
sio_8211Converter_S::addSubfield(sc_Subfield const& subfield,
				 sio_Buffer& buffer) const
{  // Use this method when you want all the digits of precision.
  double tempdouble;
  strstream val;
  string foo;

  if ( ! subfield.getS(tempdouble) ) { return 0; } // could be null
  val.flags(ios::scientific | ios::uppercase);
  val << tempdouble;
  val >> foo;
  buffer.addData(foo.c_str(), foo.length()); 
  return 0;
}


long
sio_8211Converter_S::addFixedSubfield(sc_Subfield const& subfield,
				      long length,
				      sio_Buffer& buffer) const
{  // Use this version when you only want 'length' digits of precision.
  double tempdouble;

  if ( ! subfield.getS(tempdouble) ) { return -1;} // could be null, but
                                // fixed, so this is an error

  strstream val;

  val.flags(ios::scientific | ios::uppercase);
  val << setw(length) << tempdouble;

  buffer.addData(val.str(), length); 

  val.freeze(0);

  return 0;
}



// sio_8211Converter_C
//
// Type "C" data is a character string comprised only of '1's and '0's
//  that represents binary data.
long
sio_8211Converter_C::makeFixedSubfield(sc_Subfield& subfield,
                                       char const* data,
                                       long length) const
{
  // Convert the raw data into a string and create an sc_Subfield.
  // ASCII/ISO8859 -> subfield.

  if ( length )
  {
    char* tmpBuf = new char[length + 1];
    memcpy(tmpBuf,data,length);
    tmpBuf[length] = '\0';
    
    subfield.setA(tmpBuf);
    delete [] tmpBuf;
  }
  else                          // this is an empty subfield
  {
    subfield.setA("");         // set the type
    subfield.setUnvalued();    // state that it has no value
  }

  return length;
}

long
sio_8211Converter_C::addSubfield(sc_Subfield const& subfield,
                                 sio_Buffer& buffer) const
{
  string val;

  if ( ! subfield.getA(val) ) { return 0; } // could be null
  buffer.addData( val.c_str(), val.length() ); 
  return 0;
}

long
sio_8211Converter_C::addFixedSubfield(sc_Subfield const& subfield,
                                      long length,
                                      sio_Buffer& buffer) const
{
  string val;
  if ( ! subfield.getA(val) ) { return -1; } // could be null, but fixed so
                                // this is an error

  buffer.addData( val.c_str(), min( length, static_cast<long>(val.length()) ) ); 

  return 0;
}


// sio_8211Converter_BIx
// Type "BIx" (where x is a number) is integer binary data stored in
// x bits.

long
sio_8211Converter_BI8::makeFixedSubfield(sc_Subfield& subfield,
					 char const* data,
					 long bitLength) const
{
  // Convert a single character from the input data to an 8-bit signed integer
  long length = bitLength / 8; // bitLength in bit units, not bytes
  if (length < 1)
    return 0;
  
  long val = static_cast<long>(data[0]);
  subfield.setBI8(val);

  return length;
}

long
sio_8211Converter_BI8::addSubfield(sc_Subfield const& subfield,
				   sio_Buffer& buffer) const
{
  long val;
  char charval[1];
   
  // Get the value...
  subfield.getBI8(val);

  // Convert to char...
  charval[0] = val;

  val = buffer.addData(charval,1);
  return val;
}

// sio_8211Converter_BUIx
// Type "BUIx" (where x is a number) is unsigned integer binary data 
// stored in x bits.

long
sio_8211Converter_BUI8::makeFixedSubfield(sc_Subfield& subfield,
					  char const* data,
					  long bitLength) const
{  // Convert a single character from the input data to an 8-bit unsigned integer
  long length = bitLength / 8; // bitLength in bit units, not bytes
  if (length < 1)
    return 0;
  
  unsigned long val = static_cast<unsigned long>(data[0]);
  subfield.setBUI8(val);

  return length;
}

long
sio_8211Converter_BUI8::addSubfield(sc_Subfield const& subfield,
				    sio_Buffer& buffer) const
{
  unsigned long val;
  char charval[1];

  subfield.getBUI8(val);
  charval[0] = val & 0xFF;
  val = buffer.addData(charval, 1);
   
  return val;
}

long
sio_8211Converter_BI16::makeFixedSubfield(sc_Subfield& subfield,
					  char const* data,
					  long bitLength) const
{  // Convert 2 characters from the input data to a 16-bit signed integer
  long length = bitLength / 8; // bitLength in bit units, not bytes

  if (length < 2)
    return 0;

  union
  {
    short   short_val;
    char    char_val[4];
  } val;

  val.char_val[0] = data[0];
  val.char_val[1] = data[1];

  val.short_val = ntohs(val.short_val);

  subfield.setBI16(val.short_val);
  return length ;
} // sio_8211Converter_BI16::makeFixedSubfield()


long
sio_8211Converter_BI16::addSubfield(sc_Subfield const& subfield,
				    sio_Buffer& buffer) const
{
  long long_val;
  if(!subfield.getBI16( long_val ))
    return 0;

  union
  {
    short int  short_val;
    char       char_val[2];
  } val;

  val.short_val = htons( static_cast<unsigned short int>(long_val) );

  return buffer.addData( val.char_val, 2);
} // sio_8211Converter_BI16::addSubfield()

long
sio_8211Converter_BUI16::makeFixedSubfield(sc_Subfield& subfield,
					   char const* data,
					   long bitLength) const
{  // Convert 2 characters from the input data to a 16-bit unsigned integer
  long length = bitLength / 8; // bitLength in bit units, not bytes

  if (length < 2)
    return 0;

  union
  {
    unsigned short int short_val;
    char               char_val[4];
  } val;

  val.char_val[0] = data[0];
  val.char_val[1] = data[1];

  val.short_val = ntohs(val.short_val);

  subfield.setBUI16(val.short_val);
  return length ;
} // sio_8211Converter_BUI16::makeFixedSubfield()


long
sio_8211Converter_BUI16::addSubfield(sc_Subfield const& subfield,
				     sio_Buffer& buffer) const
{
  unsigned long long_val;
  if(!subfield.getBUI16( long_val ))
    return 0;

  union
  {
    unsigned short int short_val;
    char               char_val[2];
  } val;

  val.short_val = htons( static_cast<unsigned short int>(long_val) );

  return buffer.addData( val.char_val, 2);
} // sio_8211Converter_BUI16::addSubfield()


long
sio_8211Converter_BI24::makeFixedSubfield(sc_Subfield& subfield,
					  char const* data,
					  long bitLength) const
{  // Convert three characters from the input data to a 24-bit signed integer
  long length = bitLength / 8; // bitLength in bit units, not bytes
  if (length < 3)
    return 0;
   
  signed char MSB = static_cast<signed char>(data[0]);
  unsigned char LSB = static_cast<unsigned char>(data[2]);
  unsigned char OSB = static_cast<unsigned char>(data[1]);

  long val = static_cast<long>((MSB * TwoToTheSixteenth) + 
			       (OSB * TwoToTheEigth) +
			       LSB);
  subfield.setBI24(val);
   
  return length;
}                                                                            

long
sio_8211Converter_BI24::addSubfield(sc_Subfield const& subfield,
				    sio_Buffer& buffer) const
{
  long otherval;
  unsigned long val;
  char sigbytes[3];

  // Pretend to be a signed value.
  subfield.getBI24(otherval);
  val = static_cast<unsigned long>(otherval);

  // Dice it and store it just like the unsigned value.
  sigbytes[0] = ((val / TwoToTheSixteenth) & 0xFF);
  sigbytes[1] = ((val / TwoToTheEigth) & 0xFF);
  sigbytes[2] = (val & 0xFF);
  val = buffer.addData(sigbytes, 3);
   
  return val;
}

long
sio_8211Converter_BUI24::makeFixedSubfield(sc_Subfield& subfield,
					   char const* data,
					   long bitLength) const
{  // Convert three characters from the input data to a 24-bit unsigned integer
  long length = bitLength / 8; // bitLength in bit units, not bytes
  if (length < 3)
    return 0;
   
  unsigned char MSB = static_cast<unsigned char>(data[0]);
  unsigned char LSB = static_cast<unsigned char>(data[2]);
  unsigned char OSB = static_cast<unsigned char>(data[1]);

  unsigned long val = static_cast<unsigned long>((MSB * TwoToTheSixteenth) + 
						 (OSB * TwoToTheEigth) +
						 LSB);
  subfield.setBUI24(val);
   
  return length;
}

long
sio_8211Converter_BUI24::addSubfield(sc_Subfield const& subfield,
				     sio_Buffer& buffer) const
{   
  unsigned long val;
  char sigbytes[3];

  // Unsigned conversions. Slice and dice.
  subfield.getBUI24(val);
  sigbytes[0] = ((val / TwoToTheSixteenth) & 0xFF);
  sigbytes[1] = ((val / TwoToTheEigth) & 0xFF);
  sigbytes[2] = (val & 0xFF);
  val = buffer.addData(sigbytes, 3);
   
  return val;
}

long
sio_8211Converter_BI32::makeFixedSubfield(sc_Subfield& subfield,
					  char const* data,
					  long bitLength) const
{  // Convert 4 characters from the input data to a 32-bit unsigned integer
  long length = bitLength / 8; // bitLength in bit units, not bytes

  if (length < 4)
    return 0;

  union
  {
    unsigned long int  long_val;
    char    char_val[4];
  } val;

  val.char_val[0] = data[0];
  val.char_val[1] = data[1];
  val.char_val[2] = data[2];
  val.char_val[3] = data[3];

  long int new_val = (ntohl(val.long_val));

  subfield.setBI32(new_val);
  return length ;
} // sio_8211Converter_BI32::makeFixedSubfield()


long
sio_8211Converter_BI32::addSubfield(sc_Subfield const& subfield,
				    sio_Buffer& buffer) const
{
  long long_val;
  subfield.getBI32( long_val );

  union
  {
    unsigned long int  long_val;
    char               char_val[4];
  } val;

  val.long_val = htonl(long_val);

  return buffer.addData( val.char_val, 4);
} // sio_8211Converter_BI32::addSubfield()


long
sio_8211Converter_BUI32::makeFixedSubfield(sc_Subfield& subfield,
					   char const* data,
					   long bitLength) const
{  // Convert 4 characters from the input data to a 32-bit unsigned integer

  long length = bitLength / 8; // bitLength in bit units, not bytes

  if (length < 4)
    return 0;

  union
  {
    unsigned long int  long_val;
    char    char_val[4];
  } val;

  memcpy(val.char_val, (char *)data, 4);

  val.long_val = ntohl(val.long_val);

  subfield.setBUI32(val.long_val);
  return length ;
} // sio_8211Converter_BUI32::makeFixedSubfield()

long
sio_8211Converter_BUI32::addSubfield(sc_Subfield const& subfield,
				     sio_Buffer& buffer) const
{
  unsigned long long_val;
  subfield.getBUI32( long_val );

  union
  {
    unsigned long int  long_val;
    char    char_val[4];
  } val;

  val.long_val = htonl(long_val);

  return buffer.addData( val.char_val, 4);
} // sio_8211Converter_BI32::addSubfield()

long
sio_8211Converter_BFP32::makeFixedSubfield(sc_Subfield& subfield,
					   char const* data,
					   long bitLength) const
{
  long length = bitLength/8;

  if (length < 4)
    return 0;

  union
  {
    long  debugval;
    char  char_val[4];
    float fpval;
  } val;

  memcpy( val.char_val, data, 4 );

  val.debugval = ntohl(val.debugval);

  subfield.setBFP32(val.fpval);

  return length;
}

long
sio_8211Converter_BFP32::addSubfield(sc_Subfield const& subfield,
				     sio_Buffer& buffer) const
{
  union
  {
    long  debugval;
    char  char_val[4];
    float fpval;
  } val;

  subfield.getBFP32(val.fpval);
  val.debugval = htonl(val.debugval);

  return buffer.addData(val.char_val, 4);
}

long
sio_8211Converter_BFP64::makeFixedSubfield(sc_Subfield& subfield,
					   char const* data,
					   long bitLength) const
{  
  long length = bitLength/8;

  if (length < 8)
    return 0;

  union
  {
    double fpval;
    char   char_val[8];
  } val;

  // Check endienness.
  if( 4 == htonl(4))
  {
    memcpy(val.char_val, data, 8);
  }
  else
  {
    val.char_val[7] = data[0];
    val.char_val[6] = data[1];
    val.char_val[5] = data[2];
    val.char_val[4] = data[3];
    val.char_val[3] = data[4];
    val.char_val[2] = data[5];
    val.char_val[1] = data[6];
    val.char_val[0] = data[7];
  }

  subfield.setBFP32(val.fpval);  
  return length;
}

long
sio_8211Converter_BFP64::addSubfield(sc_Subfield const& subfield,
				     sio_Buffer& buffer) const
{
  char data[8];
  union
  {
    double fpval;
    char   char_val[8];
  } val;

  subfield.getBFP64(val.fpval);  
  // Do we need to change byte ordering? 
  // 4 is an arbitrary number, BTW.
  if( 4 == htonl(4))
  {
    memcpy(data, val.char_val, 8);
  }   
  else
  {
    data[0] = val.char_val[7];
    data[1] = val.char_val[6];
    data[2] = val.char_val[5];
    data[3] = val.char_val[4];
    data[4] = val.char_val[3];
    data[5] = val.char_val[2];
    data[6] = val.char_val[1];
    data[7] = val.char_val[0];
  }

  return buffer.addData(data, 8);
}

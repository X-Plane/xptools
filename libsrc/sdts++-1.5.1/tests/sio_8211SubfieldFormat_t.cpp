//
// sio_8211SubfieldFormat_t.cpp
//


#include <cassert>

#include <iostream>


#ifdef WIN32
using namespace std;
#endif

#include <sdts++/io/sio_8211SubfieldFormat.h>
#include <sdts++/io/sio_Converter.h>

int
main(int argc, char** argv)
{
  sio_8211SubfieldFormat sf;

  const sio_8211Converter* bogus_converter = reinterpret_cast<sio_8211Converter*> (0xbad);

  sf.setLabel("FOO");
  sf.setConverter(bogus_converter);
  sf.setDelimiter('&');
  sf.setType(sio_8211SubfieldFormat::A);

  assert(sf.getLabel() == "FOO");
  assert(sf.getConverter() == bogus_converter);
  assert(sf.getDelimiter() == '&');
  assert(sf.getType() == sio_8211SubfieldFormat::A);
  assert(sf.getFormat() == sio_8211SubfieldFormat::variable);

  return 0;
}

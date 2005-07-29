//
// $Id: emit.cpp,v 1.1 1999/02/11 22:08:44 mcoletti Exp $
//

#include "emit.h"

#include <iostream>

#include <sdts++/container/sc_Record.h>
#include <sdts++/io/sio_Writer.h>
#include <sdts++/builder/sb_Module.h>


static const char* _ident = "$Id: emit.cpp,v 1.1 1999/02/11 22:08:44 mcoletti Exp $";


extern bool verbose;


void
emit( sb_Module const & module, 
      sio_8211Writer& writer )
{

  sc_Record record;
      
  if ( ! module.getRecord( record ) )
    {
      cerr << "unable to build record\n";
      exit( 1 );
    }

  if ( ! writer.put( record ) )
    {
      cerr << "problem writing module record\n";
      exit( 1 );
    }

  if ( verbose )
    {
      cout << record << "\n";
    }

} // emit

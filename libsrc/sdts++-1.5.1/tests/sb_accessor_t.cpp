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


#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>

#include <sdts++/builder/sb_Catd.h>
#include <sdts++/builder/sb_Cats.h>
#include <sdts++/builder/sb_Ddom.h>
#include <sdts++/builder/sb_Ddsh.h>
#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Ldef.h>
#include <sdts++/builder/sb_Stat.h>
#include <sdts++/builder/sb_Xref.h>
#include <sdts++/builder/sb_Iden.h>
#include <sdts++/builder/sb_Accessor.h>


extern int EF_ALIGNMENT;
extern int EF_PROTECT_BELOW;
extern int EF_PROTECT_FREE;


using namespace std;



// just blats out the given module to cout
void
dumpModuleRecord( sb_Accessor & accessor, sb_Module & module )
{
   if ( ! accessor.get( module ) )
   {
      cerr << "unable to read " << module.getModuleName() << "\n";

      return;
   }

   do 
   {
      cout << module << endl;
   }
   while ( accessor.get( module ) );

} // dumpModuleRecord





int
main( int argc, char** argv )
{

//   EF_PROTECT_BELOW = 1;
//     EF_PROTECT_FREE = 1;
//     EF_ALIGNMENT = 1;

   if ( 1 == argc )
   {
      cerr << argv[0] << ": must specify a CATD module\n";
      exit( 1 );
   }


   sb_Accessor accessor;


   if ( ! accessor.readCatd( argv[1] ) )
   {
      cerr << argv[0] << ": unable to read CATD module\n";
      exit( 2 );
   }


   // now check that the accessor's idea of the CATD module and what
   // we gave it are in synch

   assert( std::string(argv[1]) == accessor.fileName() );


   // now try to fetch a variety of module records dumping each one of
   // them to cout

   sb_Iden iden_module;
   dumpModuleRecord( accessor, iden_module );

   sb_Catd catd_module;
   dumpModuleRecord( accessor, catd_module );

   sb_Cats cats_module;
   dumpModuleRecord( accessor, cats_module );

   sb_Ddsh ddsh_module;
   dumpModuleRecord( accessor, ddsh_module );

   sb_Ddom ddom_module;
   dumpModuleRecord( accessor, ddom_module );

   sb_Iref iref_module;
   dumpModuleRecord( accessor, iref_module );

   sb_Ldef ldef_module;
   dumpModuleRecord( accessor, ldef_module );

   sb_Stat stat_module;
   dumpModuleRecord( accessor, stat_module );

   sb_Xref xref_module;
   dumpModuleRecord( accessor, xref_module );

   exit( 0 );

}

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


#include "sb_Accessor.h"


#include <utility>
#include <algorithm>
#include <functional>

#include <iostream>
#include <fstream>

#include <string>

#include <cctype>
#include <cstring>

#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>


#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif

#ifndef INCLUDED_SIO_READER_H
#include <sdts++/io/sio_Reader.h>
#endif

#ifndef INCLUDED_SB_CATD_H
#include <sdts++/builder/sb_Catd.h>
#endif


using namespace std;


static const char* ident_ = 
   "$Id: sb_Accessor.cpp,v 1.25 2003/06/10 20:51:40 mcoletti Exp $";





//
// The accessor will keep a map of these things keyed by module type.
// sb_Accessor::catd() is used to populate the map; after that
// function is called each module found in the CATD module will have a
// corresponding ``module''.  The only thing set will be the file
// name.  The stream should be null; the reader and iterator won't be
// set to anything meaningful.  Only when the user requests a specific
// module do these things get populated.  So the first invocation of
// sb_Accessor::get() will create a stream and open it to the given
// ``file_name''.  It will then attach the ``reader'' to the stream
// which, in turn, will have ``curr_record'' set to the first record.
// The stream is deleted only when the encapsulating accessor is as we
// us the stream's null-ness to determine whether we've already tried
// to read it or not.
//

struct moduleDescriptor
{
  boost::shared_ptr<boost::filesystem::ifstream> stream;    // stream pointing to an SDTS module file
                                // boost::shared_ptr to keep a canonical copy once all
                                // the dust settles from the ctors and copy
                                // ctors that the STL map::insert() is gonna do

  boost::filesystem::path       file_path; // the corresponding file path for the given 
                                // stream;  to be used to open the stream
                                // only if the user wants to access the module


                                // its corresponding reader;
                                // boost::shared_ptr<> insures that we've got a
                                // canonical copy; also we have to
                                // have dynamically allocate a reader
                                // as it doesn't support a copy ctor,
                                // and so can't be inserted into a STL
                                // container
  boost::shared_ptr<sio_8211Reader> reader; 
 

  sio_8211ForwardIterator curr_record;

  moduleDescriptor() {}

  moduleDescriptor( moduleDescriptor const& rhs) 
    : stream( rhs.stream ), 
      file_path( rhs.file_path ),
      reader( rhs.reader ),
      curr_record( rhs.curr_record )
    {}

}; // struct moduleDescriptor



struct sb_Accessor_Imp
{                               // dictionary of module contexts keyed by
                                // their respective module type

      map<string,moduleDescriptor> modules;

      /// CATD file name
      std::string fileName;


      sb_Accessor_Imp( )
      {}

      sb_Accessor_Imp( std::string const & fn )
         : fileName( fn )
      {}

}; // struct sb_Accessor_imp




sb_Accessor::sb_Accessor( )
  : imp_( new sb_Accessor_Imp )
{

} // sb_Accessor ctor




sb_Accessor::sb_Accessor( string const& catd_fn )
  : imp_( new sb_Accessor_Imp( catd_fn ) )
{
  readCatd( catd_fn );
} // sb_Accessor ctor




sb_Accessor::~sb_Accessor( )
{
  delete imp_;
} // sb_Accessor dtor



static const char* module_mnemonics_[] =
{ 
  {"CATS"},
  {"CATD"},
  {"DDOM"},
  {"DDSH"},
  {"MDOM"},
  {"MDEF"},
  {"DQHL"},
  {"DQPA"},
  {"DQAA"},
  {"DQLC"},
  {"DQCG"},
  {"IDEN"},
  {"IREF"},
  {"LDEF"},
  {"RSDF"},
  {"STAT"},
  {"XREF"},
  {""}
}; // module_mnemonics



// returns true if the mnemonic corresponds to a valid SDTS module mnemonic
//
inline
static
bool
_isValid( string const& mnemonic )
{
   int i(0);

   while ( "" != module_mnemonics_[i] )
      {
         if ( mnemonic == module_mnemonics_[i] )
            {
               return true;
            }
         ++i;
      }
   return false;
} // isValid_






// later used in a STL transform() to upcase a string
//
inline
char
toupper_( char c ) 
{ 
    return toupper( c ); 
} // toupper_




bool
sb_Accessor::readCatd( std::string const& catd_fn )
{
  imp_->fileName = catd_fn;

  boost::filesystem::ifstream catd_stream( catd_fn );

                                // bail if we can't open the CATD 
                                // module

  if ( ! catd_stream ) { return false; } 


  imp_->modules.clear();        // blow away any old entries


  sio_8211Reader          reader( catd_stream );
  sio_8211ForwardIterator curr_record( reader );

                                // bail if the reader's input stream
                                // is wedged before we're even ready
                                // to read
  if ( ! catd_stream )
  {
     return false;
  }
                                // similarly, something is wrong if
                                // the iterator starts out empty
  if ( ! curr_record )
  {
     return false;
  }

  sc_Record catd_record;

  sb_Catd   catd_module;
                                // assume that all the
                                // modules we're going to look at are
                                // in the same directory as the CATD
                                // module; so, strip out the directory
                                // part of the path name so we can
                                // later prepend that to the file
                                // names we find in the CATD module

  boost::filesystem::path catd_path( catd_fn );
  catd_path = catd_path.branch_path();


                                // grind through the CATD record,
                                // appending module records for each
                                // entry

  string      module_name;
  string      file_name;

  moduleDescriptor dummy_descriptor; // a starter module descriptor
                                // that is assigned to new module entries

  while ( curr_record )
    {
                                // fetch the current CATD record and pull out 
                                // the file name

      if ( ! curr_record.get( catd_record ) ) { break; }

      if ( ! catd_module.setRecord( catd_record ) )
        {
          return false;         // not a valid CATD module
        }

      if ( ! catd_module.getNAME( module_name ) ) { return false; }


                                // insure that the module mnemonic is
                                // in all upper case; mind that it
                                // _should_ already be this way, but
                                // it doesn't hurt to make sure

      transform( module_name.begin(), module_name.end(), 
                 module_name.begin(), toupper_ );



      pair< map<string,moduleDescriptor >::iterator, bool > p =
        imp_->modules.insert( make_pair(module_name, moduleDescriptor() ) );

                                // the insertion will have failed if
                                // the module already has an entry;
                                // XXX add support later for multiple
                                // XXX modules
      if ( ! p.second )
        {
           ++curr_record;
           continue;
        }

                                // stick the path back on and store it
                                // with the module record

      if ( ! catd_module.getFILE( file_name ) ) { return false; }

      (*p.first).second.file_path = catd_path / file_name; 


      ++curr_record;            // go on to next CATD record entry
                                
    }

  return true;
} //sb_Accessor::readCatd



 

// This returns an iterator to a record for the module that has the
// given mnemonic.  Return false if there are either no more records
// or a module of that corresponds to the given mnemonic doesn't
// exist.
static
bool
getModuleIterator_( sb_Accessor_Imp & accessor, 
                    string const& module_mnemonic,
                    sio_8211ForwardIterator & fi,
                    sio_8211_converter_dictionary* cv )
{
                                // first see if we already have a
                                // module record for the given
                                // mnemnonic; if not, then we create one

  map<string,moduleDescriptor>::iterator module_itr = 
    accessor.modules.find( module_mnemonic );

  if ( module_itr == accessor.modules.end() ) // *bzzt* Not there.  Bail.
    {
       // Actually, if we're dealing with a DDSH or DDOM module, then
       // there's a chance that these are _master_ DDSH or DDOM
       // modules, which use different mnemonics.  DDSH becomes MDEF
       // and DDOM becomes MDOM.  So if the current module_mnemonic is
       // either of those, make the appropriate correction and try
       // again.
       // XXX probably should add checks for the other alternative names
       // XXX mentioned in the TVP spec.

       string tmp_string;

       if ( "DDSH" == module_mnemonic )
       {
          tmp_string = "MDEF";
          module_itr = 
             accessor.modules.find( tmp_string );

          if ( module_itr == accessor.modules.end() )
          {
             // ok, now we REALLY give up
             return false;
          }
       }
       else if ( "DDOM" == module_mnemonic )
       {
          tmp_string = "MDOM";
          module_itr = 
             accessor.modules.find( tmp_string );

          if ( module_itr == accessor.modules.end() )
          {
             // ok, now we REALLY give up
             return false;
          }
       }
       else
       {
          return false;
       }
    }

                                // if the stream isn't open, make it so!

  if ( ! module_itr->second.stream.get() )
    {
      module_itr->second.stream = 
         boost::shared_ptr<boost::filesystem::ifstream>( new boost::filesystem::ifstream( module_itr->second.file_path, 
                                                                                          std::ios::in )  );

                                // bail if we fail to either get a new
                                // stream or the new stream itself is
                                // already wedged

      if ( ! ((*module_itr).second.stream.get() && 
              (*module_itr).second.stream->good() ) )
        {
          return false;
        }
                                // now let's insure that there's a
                                // reader; if not, make a new one

      if ( ! module_itr->second.reader.get() )
        {
          module_itr->second.reader = 
            boost::shared_ptr<sio_8211Reader>( new sio_8211Reader( *module_itr->second.stream, cv ) );
        }
      else                      // we already have a reader, so just attach
        {                       // it to the stream

          module_itr->second.reader->attach( *module_itr->second.stream, cv );
        }

                                // attach the iterator, in turn, to
                                // the reader; return since the
                                // iterator will be pointing to the
                                // first record in the module

      fi = module_itr->second.curr_record =
        (*module_itr).second.reader->begin();

      return true;
    }
                                // if we get here, then we've already
                                // read at least one record; if there
                                // are no more records, close the
                                // associated stream and return false

  if ( (*module_itr).second.curr_record.done() )
    {
      (*module_itr).second.stream->close();
      return false;
    }
                                // increment to the next record, if any
                                // exist

  ++(*module_itr).second.curr_record; 

  fi = (*module_itr).second.curr_record;

  return true;

} // getModuleIterator_




bool
sb_Accessor::get( sb_Module& module, sio_8211_converter_dictionary* cv )
{
                                // first we find the iterator for the
                                // given module

  sio_8211ForwardIterator curr_record_itr;

  if ( ! getModuleIterator_( *imp_, module.getMnemonic(), 
                             curr_record_itr,
                             cv )  )
    {
      return false;
    }                           // now interpret the content of the
                                // current record by passing it to the
                                // module; if the record is bogus in
                                // some way, return false (e.g., an
                                // IDEN record is passed to a sb_Catd
                                // object

  sc_Record curr_record;

  if ( curr_record_itr.done() || 
       (! curr_record_itr.get( curr_record )) )
    {
      return false;
    }

  return module.setRecord( curr_record );

} // sb_Accessor::get




std::string const & 
sb_Accessor::fileName() const
{
   return imp_->fileName;
} // sb_Accessor::fileName() const

//
// sio_Reader.h
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
#ifndef INCLUDED_SIO_READER_H
#define INCLUDED_SIO_READER_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <list>
#include <iostream>


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


typedef std::list<sio_8211FieldFormat> field_format_ctr;


class sc_Record;
class sio_8211Converter;
class sio_8211DR;



struct sio_8211ForwardIteratorImp;
class  sio_8211Reader;


/// Class used to walk through records found in a corresponding reader
class sio_8211ForwardIterator
{
   public:
        
      ///
      sio_8211ForwardIterator( );

      ///
      sio_8211ForwardIterator( sio_8211ForwardIterator const & );

      ///
      sio_8211ForwardIterator( sio_8211Reader& reader ) ;

      ///
      ~sio_8211ForwardIterator();

      ///
      sio_8211ForwardIterator& operator=( sio_8211ForwardIterator const & );


      /**
       This could've been an STL style operator*().  However, this
       implies that this class keeps its own copy of an sc_Record that
       it passes a reference from.  In some cases, this additional copy
       is inefficient.  This member allows for the use of a single
       sc_Record.
      */
      bool get( sc_Record& record ) ;

      ///
      void operator++();

      ///
      bool done() const;

      ///
      operator void*() const;

   private:

      ///
      sio_8211ForwardIteratorImp* imp_;

}; // class sio_8211ForwardIterator





struct sio_8211Reader_Imp;


///  A SDTS reader for 8211 files.
class
sio_8211Reader
{
   public:

      // if this is used, attach() must__ be called before it can be used
      sio_8211Reader();

      ///
      sio_8211Reader( std::istream & is, 
                      sio_8211_converter_dictionary const * const converters = 0);


      ///
      ~sio_8211Reader();

      /**
         attach the reader to the given stream opened on a valid SDTS
         module; return false if there's a problem; also use the given
         converter dictionary for hints on how to read binary data
      */
      bool attach( std::istream & is, 
                   sio_8211_converter_dictionary const * const converters = 0);

      /// similar to STL begin
      sio_8211ForwardIterator begin();

      ///
      field_format_ctr & getSchema();


   private:

      /// NOT ALLOWED
      sio_8211Reader( sio_8211Reader const& );

      /// NOT ALLOWED
      sio_8211Reader& operator=( const sio_8211Reader& );

      /// implementation guts
      sio_8211Reader_Imp* imp_;


      /**
       helper function to fill the given sc_Record with the contents
       found in the 8211 DR
      */
      bool fillScRecord_( sio_8211DR const & dr, sc_Record& record );


      /**
       same function as above, but the DR found at the given offset
       is found first -- the the previous function is called
      */
      bool fillScRecord_( long DRoffset, sc_Record& record );


      friend class  sio_8211ForwardIterator;
      friend struct sio_8211ForwardIteratorImp;

}; // class sio_8211Reader

#endif //INCLUDED_SIO_READER_H

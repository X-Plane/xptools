//
// sio_Writer.h
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


#ifndef INCLUDED_SIO_WRITER_H
#define INCLUDED_SIO_WRITER_H


class sio_8211DR;
class sio_8211DDR;
class sc_Record;


#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <list>



#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


///  This provides a prototypical interface for all SDTS writers.
class sio_Writer
{
   public:

      // sio_Writer( ) NOT NEEDED
      // sio_Writer( const sio_Writer& ) NOT NEEDED
      // operator=(  const sio_Writer& ) NOT NEEDED

      ///
      virtual ~sio_Writer() = 0;

      /// Will write the given record out to the stream.
      virtual bool put( sc_Record& ) = 0;

      /// Returns true if the writer is in a usable state.
      virtual bool good( ) const = 0;

}; // class sio_Writer




/// forward declaration for hidden sio_8211Writer internals
struct sio_8211Writer_Imp;




/// A writer for ISO 8211 files.
class sio_8211Writer : public sio_Writer
{
   public:

      ///
      sio_8211Writer( std::ofstream & ofs, 
                      const char* title,
                      sio_8211Schema const & schema );

      ///
      sio_8211Writer( std::ofstream & ofs, 
                      const char* title );

      ///
      sio_8211Writer( std::ofstream & ofs, 
                      std::string const& title,
                      sio_8211Schema const & schema );

      /**
       8211 written to ``ofs''.  ``title'' is used in the 8211 file
       identifier field. ``schema'' not only is used to build a proper
       DDR, but also is used to emit proper DR's.  
      */
      sio_8211Writer( std::ofstream & ofs, 
                      std::string const& title );

      // sio_8211Writer( const sio_8211Writer& ) NOT NEEDED
      // operator=(  const sio_8211Writer& ) NOT NEEDED

      ///
      ~sio_8211Writer();

      ///
      void setFileTitle( const char* fn );

      ///
      void setSchema( sio_8211Schema const & schema );


      /// Writes the DDR to ``ofs''.
      bool emitDDR();


      /**
       Writes the contents of the given record to the ``ofs'' given
       to the ctor.
      */
      bool put( sc_Record& record );


      /**
       Returns true if the writer can write a record and if
       the last operation worked ok.
      */
      bool good( ) const;


      /**
       The next put() will emit a special leader and a directory.  All
       subsequent put() invocations will emit only field data areas --
       the last leader and directory will be 're-used' to save space.
       (C.f., ISO/IEC 8211:1994(E), page 44, C.1.5.2, "repeating leaders
       and directories").
      
       PLEASE NOTE THAT THIS ASSUMES THAT ALL SUBSEQUENT RECORDS HAVE
       IDENTICAL RECORD LENGTHS AND FORMATS.  The behavior of put()
       is undefined for records that do not match the format found in
       the last leader and directory.
      */
      void reuseLeaderAndDirectory();

   private:

      /// made private because of embedded reference
      sio_8211Writer();

      ///
      sio_8211Writer_Imp* imp_;

      ///
      bool makeDDR_();

}; // class sio_Writer


#endif

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

#ifndef SB_ACCESSOR_H
#define SB_ACCESSOR_H

// $Id: sb_Accessor.h,v 1.9 2002/11/24 22:07:42 mcoletti Exp $

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <string>


// for converter_dictionary

#ifndef INCLUDED_SIO_8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


class sb_Module;

struct sb_Accessor_Imp;

/*!
 * \class sb_Accessor
 *
 * \todo Currently will only handle single instances of any given module. Transfers that have more instances of a type of module are not (yet) handled by this class.
 *
 * \brief Convenience class for accessing SDTS modules
 *
 * Convenience class for accessing arbitrary SDTS modules and records
 * without having to open up each module by hand.  The CATD module is
 * used to find a dataset's modules.
 *
 */
class sb_Accessor
{
   public:

      ///
      sb_Accessor();

      ///
      sb_Accessor( std::string const & catd_fn );

      ///
      ~sb_Accessor();

      /**

          Set up the accessor object for use; this must be called
          before get().  This member will use CATD information to find
          the remaining modules to populate modules for each get()
          call.  It will fail if the given file name didn't resolve to
          a valid CATD module.
      */
      bool readCatd( std::string const & catd_fn );


      /// return the CATD module file name
      std::string const & fileName() const;

      /*!
       * \fn  bool get( sb_Module & module, sio_8211_converter_dictionary* cv = 0x0 )
       *
       *  \note
       *  Admittedly this is not an optimal design.  That
       *  is, it suffers the same problems as any container that has a
       *  single internal iterator instead of allowing for separate,
       *  external iterators.  However, I recognized that users of
       *  this class (or even generally the toolkit) intend to open
       *  SDTS files once, translate them to a meaningful format, and
       *  then forget about the original SDTS files.  It's "read once,
       *  and only once."  Given that paradigm, I felt confident to go
       *  ahead with this design.  MAC.
       *
       *  \brief read a record into a builder module
       *
       *  Use the CATD information to find the corresponding module
       *  file, open it, read in the first record, and then use that
       *  record to populate the given module.  This will return false
       *  if there are no more records, the SDTS module file didn't
       *  exist, or there were some I/O or resource problems.  (E.g.,
       *  out of memory or a corrupted module.)  Will return true if
       *  the module was successfully populated.  This can be invoked
       *  multiple times for modules with more than one record; again
       *  get() will return false if all the module records have been
       *  read. The optional converter parameter is used to provide
       *  appropriate hints for reading binary data.
       *
       */
      bool get( sb_Module & module, sio_8211_converter_dictionary* cv = 0x0 );


   private:

      /// NOT NEEDED
      sb_Accessor( sb_Accessor const & );

      /// NOT NEEDED
      sb_Accessor& operator=( sb_Accessor const & );

      /// hook to hidden internal data structure
      sb_Accessor_Imp* imp_;

}; // sb_Module



#endif

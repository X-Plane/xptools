#ifndef INCLUDED_SB_UTILS_H
#define INCLUDED_SB_UTILS_H


#ifdef WIN32
#pragma warning( disable : 4786 )
#endif


#ifndef INCLUDED_SC_MODULE_H
#include <sdts++/container/sc_Module.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif

#ifndef INCLUDED_SC_FIELD_H
#include <sdts++/container/sc_Field.h>
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#include <set>


class sb_Iref;
class sb_Ddsh;


/// Several useful utility functions for use with the sb_* layer classes.
namespace sb_Utils
{
   /**
      Searches 'rec' for the first field with a mnemonic matching
      'mnemonic'.  If found, sets 'thefield' to point it and returns
      true. Returns false if not found.  
      */
   bool getFieldByMnem( sc_Record const& rec,
                        std::string const& mnemonic,
                        sc_Record::const_iterator& thefield );

   /** 
          Searches 'field' for the first subfield with a mnemonic
          matching 'mnemonic'.  If found, sets 'thesubf' to point to it and
          returns true. Returns false if not found.  
      */
   bool getSubfieldByMnem( sc_Field const& field,
                           std::string const& mnemonic,
                           sc_Field::const_iterator& thesubf );

   /**
       Searches 'field' for the first subfield with a name matching 'name'.
       If found, sets 'thesubf' to point to it and returns true. Returns false
       if not found.
      */
   bool getSubfieldByName( sc_Field const& field,
                           std::string const& name,
                           sc_Field::const_iterator& thesubf );

   /**
       Tries to convert a subfield into a double.
       If it succeeds returns True, else False.
       Place the convert value into the dataTo passed in parameter
      */
   bool getDoubleFromSubfield( sc_SubfieldCntr::const_iterator const& subf,
                               double& dataOut );

   /// Add field with given name and mnemonic to the record
   void add_field( sc_Record & record, 
                   std::string const & name,
                   std::string const & mnemonic );

   /// add the given foreign identifier to the record
   /**
      \note

      This is NOT for packed foreign identifiers.

      \todo XXX maybe we should add a member to support packed identifiers?
   */
   void add_foreignID( sc_Record & record, 
                       sb_ForeignID const & frid );

   //@{
   /** 
       convenience functions for adding subfields to a field
   */
   void add_subfield( sc_Field& field, 
                      std::string const& mnemonic, 
                      std::string const& value );

   void add_subfield( sc_Field& field, 
                      std::string const& mnemonic, 
                      int value );

   void add_subfield( sc_Field& field, 
                      std::string const& mnemonic, 
                      long value );

   void add_subfield( sc_Field& field, 
                      std::string const& mnemonic, 
                      double value );
   //@}


      /// for adding empty subfields
   void add_empty_subfield( sc_Field& field, 
                            std::string const& mnemonic, 
                            sc_Subfield::SubfieldType type );


   //@{
   /**
      determine if a value is within a set of values expressing a domain
   
      \note

       Will return true if the char value in ``str'' is within the set of 
       characters found in ``values''.

      */
   bool valid_domain( std::string const & str, 
                      std::string const & domain_values );

   bool valid_domain( std::string const & str, 
                      std::set<std::string> const & domain_values );

   bool valid_domain( long val, 
                      std::set<long> const& domain_values );
   //@}


   // find module records that match the given field and subfield
   /**

         Search the module records in the range (begin,end) for
         field's with the given subfield value; any matches are
         appended to ``matches''.

   */
   void find( sc_Module::const_iterator begin,
              sc_Module::const_iterator end,
              std::string const & field_name,
              sc_Subfield const & subfield,
              sc_Module & matches );

   /// add an appropriate binary converter based on an IREF record
   /**

      This is populated from the IREF::HFMT subfield _iff_ it's for a
      binary subfield; otherwise we don't add anything to ``dictionary''.

    */
   bool addConverter( sb_Iref const & iref,
                      sio_8211_converter_dictionary & dictionary );


   /// add an appropriate binary converter based on a DDSH record
   /**

      This is populated from the DDSH::FMT subfield _iff_ it's for a
      binary subfield; otherwise we don't add anything to ``dictionary''.

    */
   bool addConverter( sb_Ddsh const & ddsh,
                      sio_8211_converter_dictionary & dictionary );


   /// add converters for all relevent DDSH and IREF records
   /**

      This works by opening the CATD file and grinding through to find
      all the IREF and DDSH modules.  (There should be only one of the
      former and can be N of the latter.)  For each hit, the
      addConverter() for that module (i.e., IREF or DDSH) is called
      for each record adding to the ``dictionary''.  Natch the modules
      have to be opened first.

      \note

      There is a possible complication that this function doesn't
      address.  That is, it is possible for there to be DDSH entries
      for different files for the SAME subfield.  For example, if a
      transfer has five CELL modules, then the DDSH might have five
      different DDSH entries for each module to describe the same
      subfield that shows up in all the modules.  Now, when this is a
      case, the subfield values are typically the same.  So specifying
      the value for all the modules is superfluous; you can just read
      the first one, add the entry to the converter dictionary, and be
      golden.  However, there's the off-chance that there might be a
      transfer that violates this and does indeed have different
      values for each of the modules; in which case, this will only
      get the binary converter for the LAST module.

    */
   bool addConverters( std::string const & catd_filename,
                       sio_8211_converter_dictionary & dictionary );

} // namespace sb_Utils


#endif  // INCLUDED_SB_UTILS_H


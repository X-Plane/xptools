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
// $Id: sb_At.h,v 1.7 2002/11/24 22:07:42 mcoletti Exp $
//
// TODO:
//
//  - ATSC module child class
//  - Atpr::setRecord()
//
#ifndef INCLUDED_SB_AT_H
#define INCLUDED_SB_AT_H

#include <list>
#include <utility>
#include <string>


#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


class  sc_Record;


/// An attribute type is a name bound to a subfield type.
typedef std::pair<std::string, sc_Subfield::SubfieldType> attribute_type;

///
typedef std::list<attribute_type> attribute_types;


///
struct sb_At_Imp;

/**
 This class provides a convenient access to AT records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_At object.

 This ABC has two child classes, one each for primary and secondary
 attributes respectively.

*/
class sb_At : public sb_Module
{
   public:

      ///
      sb_At();

      ///
      ~sb_At();


      /// return the list of attribute types
      bool getAttributeTypes( attribute_types& val ) const;


      /** used to get individual attribute values; returns false if they don't
          exist, the wrong type was used, or it's unvalued. */
      bool getAttribute( std::string const& attribute_name, long&   val ) const;
      ///
      bool getAttribute( std::string const& attribute_name, unsigned long&   val ) const;
      ///
      bool getAttribute( std::string const& attribute_name, float&  val ) const;
      ///
      bool getAttribute( std::string const& attribute_name, double& val ) const;
      ///
      bool getAttribute( std::string const& attribute_name, std::string& val ) const;


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const = 0;


      /**
       set the object with values found in the record; if not a valid
       AT record, this will return false
      */
      bool setRecord( sc_Record const& val ) = 0;

      /**
       Use these members to set subfield/field values.  Pass in an appropriate
       value for the particular subfield/field to be set to.  They will return
       false if you try to assign a value outside the domain of the given 
       subfield/field.  (Note that this is not too pedantic; for example, we
       do not check to see if a conditionally mandatory or optional field has
       been set.)
      */
      bool setAttributeTypes( attribute_types const& val );

      /**
       used to set individual attribute values; returns false if they don't
       exist or the wrong type was used
      */
      bool setAttribute( std::string const& attribute_name, long   const& val ) ;
      ///
      bool setAttribute( std::string const& attribute_name, unsigned long   const& val ) ;
      ///
      bool setAttribute( std::string const& attribute_name, float  const& val ) ;
      ///
      bool setAttribute( std::string const& attribute_name, double const& val ) ;
      ///
      bool setAttribute( std::string const& attribute_name, std::string const& val ) ;


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.

       Returns false if attriubte of that name not found.
      */
      bool unDefineAttribute( std::string const& attribute_name );

   protected:

      ///
      std::list<sc_Subfield> const& getAttributes( void ) const;

      /// returns reference to schema
      virtual sio_8211Schema& schema_() = 0;

      ///
      virtual void buildSpecificSchema_() = 0;


   private:

      /// NOT NEEDED
      sb_At(sb_At const& right);

      /// NOT NEEDED
      sb_At const& operator=(sb_At const& right);

      /// pointer to hidden internal data structure
      sb_At_Imp* imp_;

}; // sb_At





///
struct sb_Atpr_Imp;

/**
 */
class sb_Atpr : public sb_At
{
   public:

      sb_Atpr();

      ~sb_Atpr();

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;

      /**
       set the object with values found in the record; if not a valid
       AT record, this will return false
      */
      bool setRecord( sc_Record const& val ) 
      { /* XXX NOT IMPLEMENTED YET */ return false; }


   private:


      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

      /// NOT NEEDED
      sb_Atpr(sb_Atpr const& right);

      /// NOT NEEDED
      sb_Atpr const& operator=(sb_Atpr const& right);

      /// pointer to hidden, internal data structure
      sb_Atpr_Imp* imp_;

}; // sb_Atpr


#endif // INCLUDED_SB_AT_H


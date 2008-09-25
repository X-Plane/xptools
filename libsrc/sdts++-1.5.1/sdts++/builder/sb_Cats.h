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
// $Id: sb_Cats.h,v 1.8 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_CATS_H
#define INCLUDED_SB_CATS_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <list>
#include <string>


#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

class  sb_ForeignID;
class  sc_Record;


///
struct sb_Cats_Imp;


/**
 This class provides a convenient access to CATS records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Cats object.
*/
class sb_Cats : public sb_Module
{
   public:

      sb_Cats();

      ~sb_Cats();

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the
       corresponding value is not set.  (It may not be set because a value
       was not assigned to it, or because you previously tried to assign
       an invalid value.)  Otherwise they will return true.
      */
      bool getName( std::string& val ) const;
      bool getNAME( std::string& val ) const { return getName( val ); }

      bool getType( std::string& val ) const;
      bool getTYPE( std::string& val ) const { return getType( val ); }

      bool getDomain( std::string& val ) const;
      bool getDOMN( std::string& val ) const { return getDomain( val ); }

      bool getMap( std::string& val ) const;
      bool getMAP( std::string& val ) const { return getMap( val ); }

      bool getThem( std::string& val ) const;
      bool getTHEM( std::string& val ) const { return getThem( val ); }

      bool getAggregateObject( std::string& val ) const;
      bool getAGOB( std::string& val ) const { return getAggregateObject( val ); }

      bool getAggregateObjectType( std::string& val ) const;
      bool getAGTP( std::string& val ) const { return getAggregateObjectType( val ); }

      bool getComment( std::string& val ) const;
      bool getCOMT( std::string& val ) const { return getComment( val ); }

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       CATS record, this will return false
      */
      bool setRecord( sc_Record const& val );


      /**
       Use these members to set subfield/field values.  Pass in an appropriate
       value for the particular subfield/field to be set to.  They will return
       false if you try to assign a value outside the domain of the given
       subfield/field.  (Note that this is not too pedantic; for example, we
       do not check to see if a conditionally mandatory or optional field has
       been set.)
      */
      bool setName( std::string const& val );
      bool setNAME( std::string const& val ) { return setName( val ); }

      bool setType( std::string const& val );
      bool setTYPE( std::string const& val ) { return setType( val ); }

      bool setDomain( std::string const& val );
      bool setDOMN( std::string const& val ) { return setDomain( val ); }

      bool setMap( std::string const& val );
      bool setMAP( std::string const& val ) { return setMap( val ); }

      bool setThem( std::string const& val );
      bool setTHEM( std::string const& val ) { return setThem( val ); }

      bool setAggregateObject( std::string const& val );
      bool setAGOB( std::string const& val ) { return setAggregateObject( val ); }

      bool setAggregateObjectType( std::string const& val );
      bool setAGTP( std::string const& val ) { return setAggregateObjectType( val ); }

      bool setComment( std::string const& val );
      bool setCOMT( std::string const& val ) { return setComment( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineName( );
      void unDefineNAME( ) { unDefineName( ); }

      void unDefineType( );
      void unDefineTYPE( ) { unDefineType( ); }

      void unDefineDomain( );
      void unDefineDOMN( ) { unDefineDomain( ); }

      void unDefineMap( );
      void unDefineMAP( ) { unDefineMap( ); }

      void unDefineThem( );
      void unDefineTHEM( ) { unDefineThem( ); }

      void unDefineAggregateObject( );
      void unDefineAGOB( ) { unDefineAggregateObject( ); }

      void unDefineAggregateObjectType( );
      void unDefineAGTP( ) { unDefineAggregateObjectType( ); }

      void unDefineComment( );
      void unDefineCOMT( ) { unDefineComment( ); }



   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();


      /// NOT NEEDED
      sb_Cats(sb_Cats const& right);

      /// NOT NEEDED
      sb_Cats const& operator=(sb_Cats const& right);

      /// pointer to opaque data structure
      sb_Cats_Imp* imp_;

}; // sb_Cats


#endif // INCLUDED_SB_CATS_H


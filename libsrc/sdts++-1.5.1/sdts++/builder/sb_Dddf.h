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
// $Id: sb_Dddf.h,v 1.3 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_DDDF_H
#define INCLUDED_SB_DDDF_H


#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif


#include <list>
#include <string>

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif

#ifndef INCLUDED_SB_SPATIAL_H
#include <sdts++/builder/sb_Spatial.h>
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


class  sc_Record;


// This class provides a convenient access to DDDF records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Dddf object.


struct sb_Dddf_Imp;

class sb_Dddf : public sb_Module
{
   public:

      sb_Dddf();

      ~sb_Dddf();


//@{
/**
   Use these members to get subfield/field values.  Pass in an appropriate
   type to receive the value.  These members will return false if the
   corresponding value is not set.  (It may not be set because a value
   was not assigned to it, or because you previously tried to assign
   an invalid value.)  Otherwise they will return true.
*/
      bool getEntityOrAttribute( std::string& val ) const;
      bool getEORA( std::string& val ) const { return getEntityOrAttribute( val ); }

      bool getLabel( std::string& val ) const;
      bool getEALB( std::string& val ) const { return getLabel( val ); }

      bool getSource( std::string& val ) const;
      bool getSRCE( std::string& val ) const { return getSource( val ); }

      bool getDefinition( std::string& val ) const;
      bool getDFIN( std::string& val ) const { return getDefinition( val ); }

      bool getAttributeAuthority( std::string& val ) const;
      bool getAUTH( std::string& val ) const { return getAttributeAuthority( val ); }

      bool getAttributeAuthorityDescription( std::string& val ) const;
      bool getADSC( std::string& val ) const { return getAttributeAuthorityDescription( val ); }

//@}

/**
   fill the given record based on the builder's object field/subfield
   values -- return false if in a wedged state. (E.g., a mandatory
   field isn't set or was assigned a value outside its proper
   domain.
*/
      bool getRecord( sc_Record& val ) const;


/** fills the given schema with one appropriate for DDDF
    modules; returns false if unable to create the schema for some
    bizarre reason.  (Like maybe running out of memory.)  Note that
    an sio_Writer instance will need a schema generated from this
    member.
*/
      bool getSchema( sio_8211Schema& schema ) const;


/** set the object with values found in the record; if not a valid
    DDDF record, this will return false
*/
      bool setRecord( sc_Record const& val );


//@{
/**
   Use these members to set subfield/field values.  Pass in an appropriate
   value for the particular subfield/field to be set to.  They will return
   false if you try to assign a value outside the domain of the given
   subfield/field.  (Note that this is not too pedantic; for example, we
   do not check to see if a conditionally mandatory or optional field has
   been set.)
*/
      bool setEntityOrAttribute( std::string const& val );
      bool setEORA( std::string const& val ) { return setEntityOrAttribute( val ); }

      bool setLabel( std::string const& val );
      bool setEALB( std::string const& val ) { return setLabel( val ); }

      bool setSource( std::string const& val );
      bool setSRCE( std::string const& val ) { return setSource( val ); }

      bool setDefinition( std::string const& val );
      bool setDFIN( std::string const& val ) { return setDefinition( val ); }

      bool setAttributeAuthority( std::string const& val );
      bool setAUTH( std::string const& val ) { return setAttributeAuthority( val ); }

      bool setAttributeAuthorityDescription( std::string const& val );
      bool setADSC( std::string const& val ) { return setAttributeAuthorityDescription( val ); }

//@}

//@{
/**
   Since builder objects will be frequently 'recycled' (i.e., used for
   more than one record), it might be convenient to 'unset' a previously
   assigned value.  So:
*/

      void unDefineEntityOrAttribute( );
      void unDefineEORA( ) { unDefineEntityOrAttribute( ); }

      void unDefineLabel( );
      void unDefineEALB( ) { unDefineLabel( ); }

      void unDefineSource( );
      void unDefineSRCE( ) { unDefineSource( ); }

      void unDefineDefinition( );
      void unDefineDFIN( ) { unDefineDefinition( ); }

      void unDefineAttributeAuthority( );
      void unDefineAUTH( ) { unDefineAttributeAuthority( ); }

      void unDefineAttributeAuthorityDescription( );
      void unDefineADSC( ) { unDefineAttributeAuthorityDescription( ); }

//@}

   private:

// Returns reference to schema
      virtual sio_8211Schema& schema_();

      virtual void buildSpecificSchema_();

      sb_Dddf(sb_Dddf const& right); // NOT NEEDED
      sb_Dddf const& operator=(sb_Dddf const& right); // NOT NEEDED


      sb_Dddf_Imp* _imp;

}; // sb_Dddf


#endif // INCLUDED_SB_DDDF_H


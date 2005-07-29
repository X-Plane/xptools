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
// $Id: sb_Spdm.h,v 1.3 2002/11/24 22:07:43 mcoletti Exp $
//
#ifndef INCLUDED_SB_SPDM_H
#define INCLUDED_SB_SPDM_H


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


// This class provides a convenient access to SPDM records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Spdm object.


struct sb_Spdm_Imp;

class sb_Spdm : public sb_Module
{
 public:

  sb_Spdm();

  ~sb_Spdm();


  //@{
  /**
     Use these members to get subfield/field values.  Pass in an appropriate
     type to receive the value.  These members will return false if the 
     corresponding value is not set.  (It may not be set because a value 
     was not assigned to it, or because you previously tried to assign 
     an invalid value.)  Otherwise they will return true.
  */
  bool getSpatialDomainType( std::string& val ) const;
  bool getDTYP( std::string& val ) const { return getSpatialDomainType( val ); }

  bool getDomainSpatialAddressType( std::string& val ) const;
  bool getDSTP( std::string& val ) const { return getDomainSpatialAddressType( val ); }

  bool getComment( std::string& val ) const;
  bool getCOMT( std::string& val ) const { return getComment( val ); }

  bool getDomainSpatialAddress(  sb_Spatials& val ) const;
  bool getDMSA( sb_Spatials& val ) const { return getDomainSpatialAddress( val ); }

  //@}

  /**
     fill the given record based on the builder's object field/subfield
     values -- return false if in a wedged state. (E.g., a mandatory
     field isn't set or was assigned a value outside its proper
     domain.
  */
  bool getRecord( sc_Record& val ) const;


  /** fills the given schema with one appropriate for SPDM
      modules; returns false if unable to create the schema for some
      bizarre reason.  (Like maybe running out of memory.)  Note that
      an sio_Writer instance will need a schema generated from this
      member.
  */
  bool getSchema( sio_8211Schema& schema ) const;


  /** set the object with values found in the record; if not a valid
      SPDM record, this will return false
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
  bool setSpatialDomainType( std::string const& val );
  bool setDTYP( std::string const& val ) { return setSpatialDomainType( val ); }

  bool setDomainSpatialAddressType( std::string const& val );
  bool setDSTP( std::string const& val ) { return setDomainSpatialAddressType( val ); }

  bool setComment( std::string const& val );
  bool setCOMT( std::string const& val ) { return setComment( val ); }

  bool setDomainSpatialAddress( sb_Spatials const& val );
  bool setDMSA( sb_Spatials const& val ) { return setDomainSpatialAddress( val ); }

  //@}

  //@{
  /**
     Since builder objects will be frequently 'recycled' (i.e., used for
     more than one record), it might be convenient to 'unset' a previously
     assigned value.  So:
  */

  void unDefineSpatialDomainType( );
  void unDefineDTYP( ) { unDefineSpatialDomainType( ); }

  void unDefineDomainSpatialAddressType( );
  void unDefineDSTP( ) { unDefineDomainSpatialAddressType( ); }

  void unDefineComment( );
  void unDefineCOMT( ) { unDefineComment( ); }

  void unDefineDomainSpatialAddress( );
  void unDefineDMSA( ) { unDefineDomainSpatialAddress( ); }

  //@}

 private:

  // Returns reference to schema
  virtual sio_8211Schema& schema_();

  virtual void buildSpecificSchema_();

  sb_Spdm(sb_Spdm const& right); // NOT NEEDED
  sb_Spdm const& operator=(sb_Spdm const& right); // NOT NEEDED


  sb_Spdm_Imp* _imp;

}; // sb_Spdm


#endif // INCLUDED_SB_SPDM_H


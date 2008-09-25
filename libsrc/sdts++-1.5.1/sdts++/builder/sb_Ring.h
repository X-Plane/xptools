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
// $Id: sb_Ring.h,v 1.4 2002/11/27 00:21:33 mcoletti Exp $
//
#ifndef INCLUDED_SB_RING_H
#define INCLUDED_SB_RING_H

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


// This class provides a convenient access to RING records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Ring object.


struct sb_Ring_Imp;

class sb_Ring : public sb_Module
{
 public:

  sb_Ring();

  ~sb_Ring();


  //@{
  /**
     Use these members to get subfield/field values.  Pass in an appropriate
     type to receive the value.  These members will return false if the
     corresponding value is not set.  (It may not be set because a value
     was not assigned to it, or because you previously tried to assign
     an invalid value.)  Otherwise they will return true.
  */
  bool getObjectRepresentation( std::string& val ) const;
  bool getOBRP( std::string& val ) const { return getObjectRepresentation( val ); }

  bool getLineorArcForeignID( sb_ForeignID& val ) const;
  bool getLineorArcForeignID( std::string& val ) const;
  bool getLAID( sb_ForeignID& val ) const { return getLineorArcForeignID( val ); }

  bool getPolyID( sb_ForeignID& val ) const;
  bool getPolyID( std::string& val ) const;
  bool getPLID( sb_ForeignID& val ) const { return getPolyID( val ); }

  //@}

  /**
     fill the given record based on the builder's object field/subfield
     values -- return false if in a wedged state. (E.g., a mandatory
     field isn't set or was assigned a value outside its proper
     domain.
  */
  bool getRecord( sc_Record& val ) const;


  /** fills the given schema with one appropriate for RING
      modules; returns false if unable to create the schema for some
      bizarre reason.  (Like maybe running out of memory.)  Note that
      an sio_Writer instance will need a schema generated from this
      member.
  */
  bool getSchema( sio_8211Schema& schema ) const;


  /** set the object with values found in the record; if not a valid
      RING record, this will return false
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
  bool setObjectRepresentation( std::string const& val );
  bool setOBRP( std::string const& val ) { return setObjectRepresentation( val ); }

  bool setLineorArcForeignID( sb_ForeignID const& val );
  bool setLAID( sb_ForeignID const& val ) { return setLineorArcForeignID( val ); }

  bool setPolyID( sb_ForeignID const& val );
  bool setPLID( sb_ForeignID const& val ) { return setPolyID( val ); }

  //@}

  //@{
  /**
     Since builder objects will be frequently 'recycled' (i.e., used for
     more than one record), it might be convenient to 'unset' a previously
     assigned value.  So:
  */

  void unDefineObjectRepresentation( );
  void unDefineOBRP( ) { unDefineObjectRepresentation( ); }

  void unDefineLineorArcForeignID( );
  void unDefineLAID( ) { unDefineLineorArcForeignID( ); }

  void unDefinePolyID( );
  void unDefinePLID( ) { unDefinePolyID( ); }

  //@}

 private:

  // Returns reference to schema
  virtual sio_8211Schema& schema_();

  virtual void buildSpecificSchema_();

  sb_Ring(sb_Ring const& right); // NOT NEEDED
  sb_Ring const& operator=(sb_Ring const& right); // NOT NEEDED


  sb_Ring_Imp* _imp;

}; // sb_Ring


#endif // INCLUDED_SB_RING_H


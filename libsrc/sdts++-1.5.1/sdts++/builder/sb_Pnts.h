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
// $Id: sb_Pnts.h,v 1.2 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_PNTS_H
#define INCLUDED_SB_PNTS_H



#include <memory>
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


// This class provides a convenient access to PNTS records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Pnts object.


struct sb_Pnts_Imp;

class sb_Pnts : public sb_Module
{
 public:

  sb_Pnts();

  ~sb_Pnts();


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

  bool getSpatialAddress(  sb_Spatial& val ) const;
  bool getSADR( sb_Spatial& val ) const { return getSpatialAddress( val ); }

  bool getAttributeID( std::list<std::string> & val ) const;
  bool getAttributeID( sb_AttributeIDs& val ) const;
  bool getATID( sb_AttributeIDs& val ) const { return getAttributeID( val ); }

  bool getLineID(  sb_ForeignIDs& val ) const;
  bool getLineID( std::list<std::string> & val ) const;
  bool getLNID( sb_ForeignIDs& val ) const { return getLineID( val ); }

  bool getAreaID(  sb_ForeignIDs& val ) const;
  bool getAreaID( std::list<std::string> & val ) const;
  bool getARID( sb_ForeignIDs& val ) const { return getAreaID( val ); }

  bool getCompositeID(  sb_ForeignIDs& val ) const;
  bool getCompositeID( std::list<std::string> & val ) const;
  bool getCPID( sb_ForeignIDs& val ) const { return getCompositeID( val ); }

  bool getRepresentationModuleID(  sb_ForeignIDs& val ) const;
  bool getRepresentationModuleID( std::list<std::string> & val ) const;
  bool getRPID( sb_ForeignIDs& val ) const { return getRepresentationModuleID( val ); }

  bool getOrientationSpatialAddress(  sb_ForeignIDs& val ) const;
  bool getOrientationSpatialAddress( std::list<std::string> & val ) const;
  bool getOSAD( sb_ForeignIDs& val ) const { return getOrientationSpatialAddress( val ); }

  bool getAttributePrimaryForeignID(  sb_ForeignIDs& val ) const;
  bool getAttributePrimaryForeignID( std::list<std::string> & val ) const;
  bool getPAID( sb_ForeignIDs& val ) const { return getAttributePrimaryForeignID( val ); }

  bool getArrtibuteLabel( std::list<std::string> & val ) const;
  bool getArrtibuteLabel( sb_AttributeIDs& val ) const;
  bool getATLB( sb_AttributeIDs& val ) const { return getArrtibuteLabel( val ); }

  bool getSymbolOrientationSpatialAddress(  sb_Spatial& val ) const;
  bool getSSAD( sb_Spatial& val ) const { return getSymbolOrientationSpatialAddress( val ); }

  //@}

  /**
     fill the given record based on the builder's object field/subfield
     values -- return false if in a wedged state. (E.g., a mandatory
     field isn't set or was assigned a value outside its proper
     domain.
  */
  bool getRecord( sc_Record& val ) const;


  /** fills the given schema with one appropriate for PNTS
      modules; returns false if unable to create the schema for some
      bizarre reason.  (Like maybe running out of memory.)  Note that
      an sio_Writer instance will need a schema generated from this
      member.
  */
  bool getSchema( sio_8211Schema& schema ) const;


  /** set the object with values found in the record; if not a valid
      PNTS record, this will return false
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

  bool setSpatialAddress( sb_Spatial const& val );
  bool setSADR( sb_Spatial const& val ) { return setSpatialAddress( val ); }

  bool setAttributeID( sb_AttributeIDs const& val );
  bool setATID( sb_AttributeIDs const& val ) { return setAttributeID( val ); }

  bool setLineID( sb_ForeignIDs const& val );
  bool setLNID( sb_ForeignIDs const& val ) { return setLineID( val ); }

  bool setAreaID( sb_ForeignIDs const& val );
  bool setARID( sb_ForeignIDs const& val ) { return setAreaID( val ); }

  bool setCompositeID( sb_ForeignIDs const& val );
  bool setCPID( sb_ForeignIDs const& val ) { return setCompositeID( val ); }

  bool setRepresentationModuleID( sb_ForeignIDs const& val );
  bool setRPID( sb_ForeignIDs const& val ) { return setRepresentationModuleID( val ); }

  bool setOrientationSpatialAddress( sb_ForeignIDs const& val );
  bool setOSAD( sb_ForeignIDs const& val ) { return setOrientationSpatialAddress( val ); }

  bool setAttributePrimaryForeignID( sb_ForeignIDs const& val );
  bool setPAID( sb_ForeignIDs const& val ) { return setAttributePrimaryForeignID( val ); }

  bool setArrtibuteLabel( sb_AttributeIDs const& val );
  bool setATLB( sb_AttributeIDs const& val ) { return setArrtibuteLabel( val ); }

  bool setSymbolOrientationSpatialAddress( sb_Spatial const& val );
  bool setSSAD( sb_Spatial const& val ) { return setSymbolOrientationSpatialAddress( val ); }

  //@}

  //@{
  /**
     Since builder objects will be frequently 'recycled' (i.e., used for
     more than one record), it might be convenient to 'unset' a previously
     assigned value.  So:
  */

  void unDefineObjectRepresentation( );
  void unDefineOBRP( ) { unDefineObjectRepresentation( ); }

  void unDefineSpatialAddress( );
  void unDefineSADR( ) { unDefineSpatialAddress( ); }

  void unDefineAttributeID( );
  void unDefineATID( ) { unDefineAttributeID( ); }

  void unDefineLineID( );
  void unDefineLNID( ) { unDefineLineID( ); }

  void unDefineAreaID( );
  void unDefineARID( ) { unDefineAreaID( ); }

  void unDefineCompositeID( );
  void unDefineCPID( ) { unDefineCompositeID( ); }

  void unDefineRepresentationModuleID( );
  void unDefineRPID( ) { unDefineRepresentationModuleID( ); }

  void unDefineOrientationSpatialAddress( );
  void unDefineOSAD( ) { unDefineOrientationSpatialAddress( ); }

  void unDefineAttributePrimaryForeignID( );
  void unDefinePAID( ) { unDefineAttributePrimaryForeignID( ); }

  void unDefineArrtibuteLabel( );
  void unDefineATLB( ) { unDefineArrtibuteLabel( ); }

  void unDefineSymbolOrientationSpatialAddress( );
  void unDefineSSAD( ) { unDefineSymbolOrientationSpatialAddress( ); }

  //@}

 private:

  // Returns reference to schema
  virtual sio_8211Schema& schema_();

  virtual void buildSpecificSchema_();
      
  sb_Pnts(sb_Pnts const& right); // NOT NEEDED
  sb_Pnts const& operator=(sb_Pnts const& right); // NOT NEEDED


  std::auto_ptr<sb_Pnts_Imp> _imp;

}; // sb_Pnts


#endif // INCLUDED_SB_PNTS_H


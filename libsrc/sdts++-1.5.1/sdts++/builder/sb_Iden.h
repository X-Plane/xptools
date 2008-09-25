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

#ifndef INCLUDED_SB_IDEN_H
#define INCLUDED_SB_IDEN_H

// $Id: sb_Iden.h,v 1.9 2002/11/24 22:07:42 mcoletti Exp $

#include <string>

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


//class  sb_ForeignID;
struct sb_Iden_Imp;
class  sc_Record;


/**
 This class provides a convenient access to IDEN records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Iden object.
*/
class sb_Iden : public sb_Module
{
   public:

      sb_Iden();

      ~sb_Iden();

      sb_Iden(sc_Record const & recprox);

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the
       corresponding value is not set.  (It may not be set because a value
       was not assigned to it, or because you previously tried to assign
       an invalid value.)  Otherwise they will return true.
      */
      bool getStandardIdentification( std::string& str ) const;
      bool getSTID( std::string& str ) const { return getStandardIdentification( str ); }

      bool getStandardVersion( std::string& str ) const;
      bool getSTVS( std::string& str ) const { return getStandardVersion( str ); }

      bool getStandardDocumentationReference( std::string& str ) const;
      bool getDOCU( std::string& str ) const
      { return getStandardDocumentationReference( str ); }

      bool getProfileIdentification( std::string& str ) const;
      bool getPRID( std::string& str ) const { return getProfileIdentification( str ); }

      bool getProfileVersion( std::string& str ) const;
      bool getPRVS( std::string& str ) const { return getProfileVersion( str ); }

      bool getProfileDocumentationReference( std::string& str ) const;
      bool getPDOC( std::string& str ) const
      { return getProfileDocumentationReference( str ); }

      bool getTitle( std::string& str ) const;
      bool getTITL( std::string& str ) const { return getTitle( str ); }

      bool getDataID( std::string& str ) const;
      bool getDAID( std::string& str ) const { return getDataID( str ); }

      bool getDataStructure( std::string& str ) const;
      bool getDAST( std::string& str ) const { return getDataStructure( str ); }

      bool getMapDate( std::string& str ) const;
      bool getMPDT( std::string& str ) const { return getMapDate( str ); }

      bool getDataSetCreationDate( std::string& str ) const;
      bool getDCDT( std::string& str ) const { return getDataSetCreationDate( str ); }

      bool getScale( long& val ) const;
      bool getSCAL( long& val ) const { return getScale( val ); }

      bool getComment( std::string& str ) const;
      bool getCOMT( std::string& str ) const { return getComment( str ); }


      //  Conformance field

      bool getComposites( std::string& str ) const;
      bool getFFYN( std::string& str ) const { return getComposites( str ); }

      bool getVectorGeometry( std::string& str ) const;
      bool getVGYN( std::string& str ) const { return getVectorGeometry( str ); }

      bool getVectorTopology( std::string& str ) const;
      bool getGTYN( std::string& str ) const { return getVectorTopology( str ); }

      bool getRaster( std::string& str ) const;
      bool getRCYN( std::string& str ) const { return getRaster( str ); }

      bool getExternalSpatialReference( long& val ) const;
      bool getEXSP( long& val ) const { return getExternalSpatialReference( val ); }

      bool getFeaturesLevel( long& val ) const;
      bool getFTLV( long& val ) const { return getFeaturesLevel( val ); }

      bool getCodingLevel( long& val ) const;
      bool getCDLV( long& val ) const { return getCodingLevel( val ); }

      bool getNonGeoSpatialDimensions( std::string& str ) const;
      bool getNGDM( std::string& str ) const
      { return getNonGeoSpatialDimensions( str ); }


      // Attribute

#ifdef NOT_IMPLEMENTED
      bool getAttributeID( sb_ForeignID& fid ) const;
      bool getATID( sb_ForeignID& fid ) const { return getAttributeID( fid ); }
#endif

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside it's proper
       domain.
      */
      bool getRecord( sc_Record& record ) const;


      /**
       Use these members to set subfield/field values.  Pass in an appropriate
       value for the particular subfield/field to be set to.  They will return
       false if you try to assign a value outside the domain of the given
       subfield/field.  (Note that this is not too pedantic; for example, we
       do not check to see if a conditionally mandatory or optional field has
       been set.)
      */
      void setStandardIdentification( std::string const & );
      void setSTID( std::string const & str ) { setStandardIdentification( str ); }

      void setStandardVersion( std::string const & );
      void setSTVS( std::string const & str ) {  setStandardVersion( str ); }

      void setStandardDocumentationReference( std::string const & );
      void setDOCU( std::string const & str ) {  setStandardDocumentationReference( str ); }

      void setProfileIdentification( std::string const & );
      void setPRID( std::string const & str ) {  setProfileIdentification( str ); }

      void setProfileVersion( std::string const & );
      void setPRVS( std::string const & str ) {  setProfileVersion( str ); }

      void setProfileDocumentationReference( std::string const & );
      void setPDOC( std::string const & str ) {  setProfileDocumentationReference( str ); }

      void setTitle( std::string const & );
      void setTITL( std::string const & str ) {  setTitle( str ); }

      void setDataID( std::string const & );
      void setDAID( std::string const & str ) {  setDataID( str ); }

      void setDataStructure( std::string const & );
      void setDAST( std::string const & str ) {  setDataStructure( str ); }

      void setMapDate( std::string const & );
      void setMPDT( std::string const & str ) {  setMapDate( str ); }

      void setDataSetCreationDate( std::string const & );
      void setDCDT( std::string const & str ) {  setDataSetCreationDate( str ); }

      void setScale( long );
      void setSCAL( long val ) {  setScale( val ); }

      void setComment( std::string const & );
      void setCOMT( std::string const & str ) {  setComment( str ); }


      // Conformance Field

      void setComposites( std::string const & );
      void setFFYN( std::string const & str ) {  setComposites( str ); }

      void setVectorGeometry( std::string const & );
      void setVGYN( std::string const & str ) {  setVectorGeometry( str ); }

      void setVectorTopology( std::string const & );
      void setGTYN( std::string const & str ) {  setVectorTopology( str ); }

      void setRaster( std::string const & );
      void setRCYN( std::string const & str ) {  setRaster( str ); }

      void setExternalSpatialReference( long );
      void setEXSP( long val ) {  setExternalSpatialReference( val ); }

      void setFeaturesLevel( long );
      void setFTLV( long val ) {  setFeaturesLevel( val ); }

      void setCodingLevel( long );
      void setCDLV( long val ) {  setCodingLevel( val ); }

      void setNonGeoSpatialDimensions( std::string const & );
      void setNGDM( std::string const & str ) {  setNonGeoSpatialDimensions( str ); }


#ifdef NOT_IMPLEMENTED
      void setAttributeID( sb_ForeignID const & );
      void setATID( sb_ForeignID const & fid ) {  setAttributeID( fid ); }
#endif


      /**
       set the object with values found in the record; if not a valid
       IDEN record, this will return false
      */
      bool setRecord( sc_Record const& record );


   private:

      /// returns reference to schema
      virtual sio_8211Schema& schema_();

      ///
      virtual void buildSpecificSchema_();


      /// NOT NEEDED
      sb_Iden( sb_Iden const & );

      /// hidden opaque data type containing internals
      sb_Iden_Imp*  imp_;

}; // sb_Iden



#endif

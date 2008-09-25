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
// $Id: sb_Rsdf.h,v 1.11 2002/11/25 19:16:13 mcoletti Exp $
//
// TODO:
//
//    Add domain checking in the set*() members
//
#ifndef INCLUDED_SB_RSDF_H
#define INCLUDED_SB_RSDF_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <list>
#include <string>


#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

class  sc_Record;



struct sb_Rsdf_Imp;

/**
 This class provides a convenient access to RSDF records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Rsdf object.

 XXX NOTE THAT THIS IS NOT A COMPLETE RSDF IMPLEMENTATION.  I only
 XXX supported fields as I needed them.

*/
class sb_Rsdf : public sb_Module
{
   public:

      sb_Rsdf();

      ~sb_Rsdf();

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the
       corresponding value is not set.  (It may not be set because a value
       was not assigned to it, or because you previously tried to assign
       an invalid value.)  Otherwise they will return true.
      */
      bool getObjectRepresentation( std::string& val ) const;
      bool getOBRP( std::string& val ) const { return getObjectRepresentation( val ); }

      bool getCellSequencingCode( std::string& val ) const;
      bool getCSCD( std::string& val ) const { return getCellSequencingCode( val ); }

      bool getCompression( std::string& val ) const;
      bool getCMPR( std::string& val ) const { return getCompression( val ); }

      bool getEncodingMethod( std::string& val ) const;
      bool getMETH( std::string& val ) const { return getEncodingMethod( val ); }

      bool getAcquisitionDeviceMethod( std::string& val ) const;
      bool getAQMD( std::string& val ) const { return getAcquisitionDeviceMethod( val ); }

      bool getAcquisitionDate( std::string& val ) const;
      bool getAQDT( std::string& val ) const { return getAcquisitionDate( val ); }

      bool getComments( std::string& val ) const;
      bool getCOMT( std::string& val ) const { return getComments( val ); }

      bool getDefaultImplementation( std::string& val ) const;
      bool getDEFI( std::string& val ) const { return getDefaultImplementation( val ); }

      bool getRowExtent( long& val ) const;
      bool getRWXT( long& val ) const { return getRowExtent( val ); }

      bool getColumnExtent( long& val ) const;
      bool getCLXT( long& val ) const { return getColumnExtent( val ); }

      // Phy - unneeded
      // bool getPlaneExtent( long& val ) const;
      // bool getPLXT( long& val ) const { return getPlaneExtent( val ); }

      bool getScanOrigin( std::string& val ) const;
      bool getSCOR( std::string& val ) const { return getScanOrigin( val ); }

      bool getTesseralIndexing( std::string& val ) const;
      bool getTIDX( std::string& val ) const { return getTesseralIndexing( val ); }

      bool getScanPattern( std::string& val ) const;
      bool getSCPT( std::string& val ) const { return getScanPattern( val ); }

// Phy - unneeded
//  bool getTesseralIndexFormat( std::string& val ) const;
//  bool getTIFT( std::string& val ) const { return getTesseralIndexFormat( val ); }
//
//  bool getTesseralIndexingDescription( std::string& val ) const;
//  bool getTIDS( std::string& val ) const { return getTesseralIndexingDescription( val ); }

      bool getNumberLinesAlternation( long& val ) const;
      bool getALTN( long& val ) const { return getNumberLinesAlternation( val ); }

      bool getFirstScanDirection( std::string& val ) const;
      bool getFSCN( std::string& val ) const { return getFirstScanDirection( val ); }

      bool getAspectRation( double& val ) const;
      bool getASPR( double& val ) const { return getAspectRation( val ); }

      bool getNumberLayers( long& val ) const;
      bool getNLAY( long& val ) const { return getNumberLayers( val ); }

      bool getSpatialAddress( double& x, double& y ) const;
      bool getSADR( double& x, double& y ) const { return getSpatialAddress( x, y ); }


      // Phy - add gets for the new fields
      bool getInternalSpatialId( sb_ForeignID& fid ) const;
      bool getISID( sb_ForeignID& fid ) const
      { return getInternalSpatialId( fid ); }

      bool getLayerIds( sb_ForeignIDs& fids ) const;
      bool getLYIDs( sb_ForeignIDs& fids ) const { return getLayerIds( fids ); }

      bool getRasterAttributeIds( sb_ForeignIDs& fids ) const;
      bool getRATPs( sb_ForeignIDs& fids ) const
      { return getRasterAttributeIds( fids ); }


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.

       XXX NOTE THAT ONLY THE FIRST FIELD IS CURRENTLY SUPPORTED
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       RSDF record, this will return false

       XXX NOTE THAT ONLY THE FIRST FIELD IS CURRENTLY SUPPORTED
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
      bool setObjectRepresentation( std::string const& val );
      bool setOBRP( std::string const& val ) { return setObjectRepresentation( val ); }

      bool setCellSequencingCode( std::string const& val );
      bool setCSCD( std::string const& val ) { return setCellSequencingCode( val ); }

      bool setCompression( std::string const& val );
      bool setCMPR( std::string const& val ) { return setCompression( val ); }

      bool setEncodingMethod( std::string const& val );
      bool setMETH( std::string const& val ) { return setEncodingMethod( val ); }

      bool setAcquisitionDeviceMethod( std::string const& val );
      bool setAQMD( std::string const& val ) { return setAcquisitionDeviceMethod( val ); }

      bool setAcquisitionDate( std::string const& val );
      bool setAQDT( std::string const& val ) { return setAcquisitionDate( val ); }

      bool setComments( std::string const& val );
      bool setCOMT( std::string const& val ) { return setComments( val ); }

      bool setDefaultImplementation( std::string const& val );
      bool setDEFI( std::string const& val ) { return setDefaultImplementation( val ); }

      bool setRowExtant( long val );
      bool setRWXT( long val ) { return setRowExtant( val ); }

      bool setColumnExtent( long val );
      bool setCLXT( long val ) { return setColumnExtent( val ); }

      // Phy - unneeded
      //bool setPlaneExtent( long val );
      //bool setPLXT( long val ) { return setPlaneExtent( val ); }

      bool setScanOrigin( std::string const& val );
      bool setSCOR( std::string const& val ) { return setScanOrigin( val ); }

      bool setTesseralIndexing( std::string const& val );
      bool setTIDX( std::string const& val ) { return setTesseralIndexing( val ); }

      bool setScanPattern( std::string const& val );
      bool setSCPT( std::string const& val ) { return setScanPattern( val ); }

      // Phy - unneeded
      //bool setTesseralIndexFormat( std::string const& val );
      //bool setTIFT( std::string const& val ) { return setTesseralIndexFormat( val ); }
      //
      //bool setTesseralIndexingDescription( std::string const& val );
      //bool setTIDS( std::string const& val ) { return setTesseralIndexingDescription( val ); }

      bool setNumberLinesAlternation( long val );
      bool setALTN( long val ) { return setNumberLinesAlternation( val ); }

      bool setFirstScanDirection( std::string const& val );
      bool setFSCN( std::string const& val ) { return setFirstScanDirection( val ); }

      bool setAspectRation( double val );
      bool setASPR( double val ) { return setAspectRation( val ); }

      bool setNumberLayers( long val );
      bool setNLAY( long val ) { return setNumberLayers( val ); }

      bool setSpatialAddress( double x, double y );
      bool setSADR( double x, double y ) { return setSpatialAddress( x, y ); }


      // Phy - Add sets for new fields
      bool setInternalSpatialId( sb_ForeignID const & fid );
      bool setISID( sb_ForeignID const & fid ) { return setInternalSpatialId( fid ); }

      bool setLayerId( sb_ForeignIDs const & fids );
      bool setLYID( sb_ForeignIDs const & fids ) { return setLayerId( fids ); }

      bool setRasterAttributeId( sb_ForeignIDs const & fids );
      bool setRATP( sb_ForeignIDs const & fids ) { return setRasterAttributeId( fids ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineObjectRepresentation( );
      void unDefineOBRP( ) { unDefineObjectRepresentation( ); }

      void unDefineCellSequencingCode( );
      void unDefineCSCD( ) { unDefineCellSequencingCode( ); }

      void unDefineCompression( );
      void unDefineCMPR( ) { unDefineCompression( ); }

      void unDefineEncodingMethod( );
      void unDefineMETH( ) { unDefineEncodingMethod( ); }

      void unDefineAcquisitionDeviceMethod( );
      void unDefineAQMD( ) { unDefineAcquisitionDeviceMethod( ); }

      void unDefineAcquisitionDate( );
      void unDefineAQDT( ) { unDefineAcquisitionDate( ); }

      void unDefineComments( );
      void unDefineCOMT( ) { unDefineComments( ); }

      void unDefineDefaultImplementation( );
      void unDefineDEFI( ) { unDefineDefaultImplementation( ); }

      void unDefineRowExtant( );
      void unDefineRWXT( ) { unDefineRowExtant( ); }

      void unDefineColumnExtent( );
      void unDefineCLXT( ) { unDefineColumnExtent( ); }

      //Phy - unneeded
      //void unDefinePlaneExtent( );
      //void unDefinePLXT( ) { unDefinePlaneExtent( ); }

      void unDefineScanOrigin( );
      void unDefineSCOR( ) { unDefineScanOrigin( ); }

      void unDefineTesseralIndexing( );
      void unDefineTIDX( ) { unDefineTesseralIndexing( ); }

      void unDefineScanPattern( );
      void unDefineSCPT( ) { unDefineScanPattern( ); }

      // phy - unneeded
      //void unDefineTesseralIndexFormat( );
      //void unDefineTIFT( ) { unDefineTesseralIndexFormat( ); }
      //
      //void unDefineTesseralIndexingDescription( );
      //void unDefineTIDS( ) { unDefineTesseralIndexingDescription( ); }

      void unDefineNumberLinesAlternation( );
      void unDefineALTN( ) { unDefineNumberLinesAlternation( ); }

      void unDefineFirstScanDirection( );
      void unDefineFSCN( ) { unDefineFirstScanDirection( ); }

      void unDefineAspectRation( );
      void unDefineASPR( ) { unDefineAspectRation( ); }

      void unDefineNumberLayers( );
      void unDefineNLAY( ) { unDefineNumberLayers( ); }

      void unDefineSpatialAddress( );
      void unDefineSADR( ) { unDefineSpatialAddress( ); }

      void unDefineInternalSpatialId();
      void unDefineISID() { unDefineInternalSpatialId(); }

      void unDefineRasterAttributeIds();
      void unDefineRATPs() { unDefineRasterAttributeIds(); }

      void unDefineLayerIds();
      void unDefineLAIDs() { unDefineLayerIds(); }

   private:


      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();

      /// NOT NEEDED
      sb_Rsdf(sb_Rsdf const& right);

      /// NOT NEEDED
      sb_Rsdf const& operator=(sb_Rsdf const& right);

      /// pointer to internal, hidden opaque data structure
      sb_Rsdf_Imp* imp_;

}; // sb_Rsdf


#endif // INCLUDED_SB_RSDF_H


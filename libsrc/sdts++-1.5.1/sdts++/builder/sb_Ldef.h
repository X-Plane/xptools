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
// $Id: sb_Ldef.h,v 1.8 2002/11/24 22:07:42 mcoletti Exp $
//
// TODO:
//
//   - set*() should do checking
//   - getRecord() should do mandatory subfield checking
//
#ifndef INCLUDED_SB_LDEF_H
#define INCLUDED_SB_LDEF_H


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


struct sb_Ldef_Imp;


/**
 This class provides a convenient access to LDEF records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Ldef object.
*/
class sb_Ldef : public sb_Module
{
   public:

      sb_Ldef();

      ~sb_Ldef();


      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the 
       corresponding value is not set.  (It may not be set because a value 
       was not assigned to it, or because you previously tried to assign 
       an invalid value.)  Otherwise they will return true.
      */
      bool getCellModuleName( std::string& val ) const;
      bool getCMNM( std::string& val ) const { return getCellModuleName( val ); }

      bool getLayerLabel( std::string& val ) const;
      bool getLLBL( std::string& val ) const { return getLayerLabel( val ); }

      bool getCellCode( std::string& val ) const;
      bool getCODE( std::string& val ) const { return getCellCode( val ); }

//  bool getBitmask( std::string& val ) const;
//  bool getBMSK( std::string& val ) const { return getBitmask( val ); }

      bool getNumberRows( long& val ) const;
      bool getNROW( long& val ) const { return getNumberRows( val ); }

      bool getNumberColumns( long& val ) const;
      bool getNCOL( long& val ) const { return getNumberColumns( val ); }

//  bool getNumberPlanes( long& val ) const;
//  bool getNPLA( long& val ) const { return getNumberPlanes( val ); }

      bool getScanOriginRow( long& val ) const;
      bool getSORI( long& val ) const { return getScanOriginRow( val ); }

      bool getScanOriginColumn( long& val ) const;
      bool getSOCI( long& val ) const { return getScanOriginColumn( val ); }

//  bool getScanOriginPlane( long& val ) const;
//  bool getSOPI( long& val ) const { return getScanOriginPlane( val ); }

      bool getRowOffsetOrigin( long& val ) const;
      bool getRWOO( long& val ) const { return getRowOffsetOrigin( val ); }

      bool getColumnOffsetOrigin( long& val ) const;
      bool getCLOO( long& val ) const { return getColumnOffsetOrigin( val ); }

//  bool getPlaneOffsetOrigin( long& val ) const;
//  bool getPLOO( long& val ) const { return getPlaneOffsetOrigin( val ); }

      bool getIntracellReferenceLocation( std::string& val ) const;
      bool getINTR( std::string& val ) const { return getIntracellReferenceLocation( val ); }

//  bool getComment( std::string& val ) const;
//  bool getCOMT( std::string& val ) const { return getComment( val ); }


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       LDEF record, this will return false
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
      bool setCellModuleName( std::string const& val );
      bool setCMNM( std::string const& val ) { return setCellModuleName( val ); }

      bool setLayerLabel( std::string const& val );
      bool setLLBL( std::string const& val ) { return setLayerLabel( val ); }

      bool setCellCode( std::string const& val );
      bool setCODE( std::string const& val ) { return setCellCode( val ); }

//  bool setBitmask( std::string const& val );
//  bool setBMSK( std::string const& val ) { return setBitmask( val ); }

      bool setNumberRows( long val );
      bool setNROW( long val ) { return setNumberRows( val ); }

      bool setNumberColumns( long val );
      bool setNCOL( long val ) { return setNumberColumns( val ); }

//  bool setNumberPlanes( long val );
//  bool setNPLA( long val ) { return setNumberPlanes( val ); }

      bool setScanOriginRow( long val );
      bool setSORI( long val ) { return setScanOriginRow( val ); }

      bool setScanOriginColumn( long val );
      bool setSOCI( long val ) { return setScanOriginColumn( val ); }

//  bool setScanOriginPlane( long val );
//  bool setSOPI( long val ) { return setScanOriginPlane( val ); }

      bool setRowOffsetOrigin( long val );
      bool setRWOO( long val ) { return setRowOffsetOrigin( val ); }

      bool setColumnOffsetOrigin( long val );
      bool setCLOO( long val ) { return setColumnOffsetOrigin( val ); }

//  bool setPlaneOffsetOrigin( long val );
//  bool setPLOO( long val ) { return setPlaneOffsetOrigin( val ); }

      bool setIntracellReferenceLocation( std::string const& val );
      bool setINTR( std::string const& val ) { return setIntracellReferenceLocation( val ); }

//  bool setComment( std::string const& val );
//  bool setCOMT( std::string const& val ) { return setComment( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineCellModuleName( );
      void unDefineCMNM( ) { unDefineCellModuleName( ); }

      void unDefineLayerLabel( );
      void unDefineLLBL( ) { unDefineLayerLabel( ); }

      void unDefineCellCode( );
      void unDefineCODE( ) { unDefineCellCode( ); }

//  void unDefineBitmask( );
//  void unDefineBMSK( ) { unDefineBitmask( ); }

      void unDefineNumberRows( );
      void unDefineNROW( ) { unDefineNumberRows( ); }

      void unDefineNumberColumns( );
      void unDefineNCOL( ) { unDefineNumberColumns( ); }

//  void unDefineNumberPlanes( );
//  void unDefineNPLA( ) { unDefineNumberPlanes( ); }

      void unDefineScanOriginRow( );
      void unDefineSORI( ) { unDefineScanOriginRow( ); }

      void unDefineScanOriginColumn( );
      void unDefineSOCI( ) { unDefineScanOriginColumn( ); }

//  void unDefineScanOriginPlane( );
//  void unDefineSOPI( ) { unDefineScanOriginPlane( ); }

      void unDefineRowOffsetOrigin( );
      void unDefineRWOO( ) { unDefineRowOffsetOrigin( ); }

      void unDefineColumnOffsetOrigin( );
      void unDefineCLOO( ) { unDefineColumnOffsetOrigin( ); }

//  void unDefinePlaneOffsetOrigin( );
//  void unDefinePLOO( ) { unDefinePlaneOffsetOrigin( ); }

      void unDefineIntracellReferenceLocation( );
      void unDefineINTR( ) { unDefineIntracellReferenceLocation( ); }

//  void unDefineComment( );
//  void unDefineCOMT( ) { unDefineComment( ); }



   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();

      /// NOT NEEDED
      sb_Ldef(sb_Ldef const& right);

      /// NOT NEEDED
      sb_Ldef const& operator=(sb_Ldef const& right);


      /// pointer to internal, hidden data structure
      sb_Ldef_Imp* imp_;

}; // sb_Ldef


#endif // INCLUDED_SB_LDEF_H


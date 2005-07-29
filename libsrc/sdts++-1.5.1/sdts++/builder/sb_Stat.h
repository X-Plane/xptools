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
// $Id: sb_Stat.h,v 1.8 2002/11/24 22:07:43 mcoletti Exp $
//
#ifndef INCLUDED_SB_STAT_H
#define INCLUDED_SB_STAT_H

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



struct sb_Stat_Imp;


/**
 This class provides a convenient access to STAT records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Stat object.
*/
class sb_Stat : public sb_Module
{
   public:

      ///
      sb_Stat();

      ///
      ~sb_Stat();

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the 
       corresponding value is not set.  (It may not be set because a value 
       was not assigned to it, or because you previously tried to assign 
       an invalid value.)  Otherwise they will return true.
      */
      bool getModuleTypeReferred( std::string& val ) const;
      bool getMNTF( std::string& val ) const { return getModuleTypeReferred( val ); }

      bool getModuleNameReferred( std::string& val ) const;
      bool getMNRF( std::string& val ) const { return getModuleNameReferred( val ); }

      bool getModuleRecordCount( long& val ) const;
      bool getNREC( long& val ) const { return getModuleRecordCount( val ); }

      bool getSpatialAddressCount( long& val ) const;
      bool getNSAD( long& val ) const { return getSpatialAddressCount( val ); }


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       STAT record, this will return false
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
      bool setModuleTypeReferred( std::string const& val );
      bool setMNTF( std::string const& val ) { return setModuleTypeReferred( val ); }

      bool setModuleNameReferred( std::string const& val );
      bool setMNRF( std::string const& val ) { return setModuleNameReferred( val ); }

      bool setModuleRecordCount( long val );
      bool setNREC( long val ) { return setModuleRecordCount( val ); }

      bool setSpatialAddressCount( long val );
      bool setNSAD( long val ) { return setSpatialAddressCount( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineModuleTypeReferred( );
      void unDefineMNTF( ) { unDefineModuleTypeReferred( ); }

      void unDefineModuleNameReferred( );
      void unDefineMNRF( ) { unDefineModuleNameReferred( ); }

      void unDefineModuleRecordCount( );
      void unDefineNREC( ) { unDefineModuleRecordCount( ); }

      void unDefineSpatialAddressCount( );
      void unDefineNSAD( ) { unDefineSpatialAddressCount( ); }



   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();


      /// NOT NEEDED
      sb_Stat(sb_Stat const& right);

      /// NOT NEEDED
      sb_Stat const& operator=(sb_Stat const& right);

      /// pointer to internal, opaque data structure
      sb_Stat_Imp* imp_;

}; // sb_Stat


#endif // INCLUDED_SB_STAT_H


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
// $Id: sb_Ddom.h,v 1.9 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_DDOM_H
#define INCLUDED_SB_DDOM_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <list>
#include <string>

#ifndef SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

class  sb_ForeignID;
class  sc_Record;


struct sb_Ddom_Imp;

/**
 This class provides a convenient access to DDOM records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Ddom object.
*/
class sb_Ddom : public sb_Module
{
   public:

      sb_Ddom();

      ~sb_Ddom();

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

      bool getAttributeLabel( std::string& val ) const;
      bool getATLB( std::string& val ) const { return getAttributeLabel( val ); }

      bool getAttributeAuthority( std::string& val ) const;
      bool getAUTH( std::string& val ) const { return getAttributeAuthority( val ); }

      bool getAttributeDomainType( std::string& val ) const;
      bool getATYP( std::string& val ) const { return getAttributeDomainType( val ); }

      bool getAttributeDomainValueFormat( sc_Subfield::SubfieldType& val ) const;
      bool getADVF( sc_Subfield::SubfieldType& val ) const { return getAttributeDomainValueFormat( val ); }

      bool getAttributeDomainValueMeasurementUnit( std::string& val ) const;
      bool getADMU( std::string& val ) const { return getAttributeDomainValueMeasurementUnit( val ); }

      bool getRangeOrValue( std::string& val ) const;
      bool getRAVA( std::string& val ) const { return getRangeOrValue( val ); }

      bool getDomainValue( sc_Subfield& val ) const;
      bool getDVAL( sc_Subfield& val ) const { return getDomainValue( val ); }

      bool getDomainValueDefinition( std::string& val ) const;
      bool getDVDF( std::string& val ) const { return getDomainValueDefinition( val ); }

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DDOM record, this will return false
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

      bool setAttributeLabel( std::string const& val );
      bool setATLB( std::string const& val ) { return setAttributeLabel( val ); }

      bool setAttributeAuthority( std::string const& val );
      bool setAUTH( std::string const& val ) { return setAttributeAuthority( val ); }

      bool setAttributeDomainType( std::string const& val );
      bool setATYP( std::string const& val ) { return setAttributeDomainType( val ); }

      bool setAttributeDomainValueFormat( sc_Subfield::SubfieldType const& val );
      bool setADVF( sc_Subfield::SubfieldType const& val ) { return setAttributeDomainValueFormat( val ); }

      bool setAttributeDomainValueMeasurementUnit( std::string const& val );
      bool setADMU( std::string const& val ) { return setAttributeDomainValueMeasurementUnit( val ); }

      bool setRangeOrValue( std::string const& val );
      bool setRAVA( std::string const& val ) { return setRangeOrValue( val ); }

      bool setDomainValue( sc_Subfield const& val );
      bool setDVAL( sc_Subfield const& val ) { return setDomainValue( val ); }

      bool setDomainValueDefinition( std::string const& val );
      bool setDVDF( std::string const& val ) { return setDomainValueDefinition( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineName( );
      void unDefineNAME( ) { unDefineName( ); }

      void unDefineType( );
      void unDefineTYPE( ) { unDefineType( ); }

      void unDefineAttributeLabel( );
      void unDefineATLB( ) { unDefineAttributeLabel( ); }

      void unDefineAttributeAuthority( );
      void unDefineAUTH( ) { unDefineAttributeAuthority( ); }

      void unDefineAttributeDomainType( );
      void unDefineATYP( ) { unDefineAttributeDomainType( ); }

                                // as a side-effect, also unvalues the DVAL
      void unDefineAttributeDomainValueFormat( );
      void unDefineADVF( ) { unDefineAttributeDomainValueFormat( ); }

      void unDefineAttributeDomainValueMeasurementUnit( );
      void unDefineADMU( ) { unDefineAttributeDomainValueMeasurementUnit( ); }

      void unDefineRangeOrValue( );
      void unDefineRAVA( ) { unDefineRangeOrValue( ); }

      void unDefineDomainValue( );
      void unDefineDVAL( ) { unDefineDomainValue( ); }

      void unDefineDomainValueDefinition( );
      void unDefineDVDF( ) { unDefineDomainValueDefinition( ); }



   private:


      /// returns reference to schema
      virtual sio_8211Schema& schema_();

      ///
      virtual void buildSpecificSchema_();


      /// NOT NEEDED
      sb_Ddom(sb_Ddom const& right);

      /// NOT NEEDED
      sb_Ddom const& operator=(sb_Ddom const& right);

      /// pointer to opaque data structure
      sb_Ddom_Imp* imp_;

}; // sb_Ddom


#endif // INCLUDED_SB_DDOM_H


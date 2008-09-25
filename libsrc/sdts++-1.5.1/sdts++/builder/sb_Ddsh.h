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
// $Id: sb_Ddsh.h,v 1.6 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_DDSH_H
#define INCLUDED_SB_DDSH_H

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


struct sb_Ddsh_Imp;


/**
 This class provides a convenient access to DDSH records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Ddsh object.
*/
class sb_Ddsh : public sb_Module
{
   public:

      sb_Ddsh();

      ~sb_Ddsh();

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

      bool getEntityLabel( std::string& val ) const;
      bool getETLB( std::string& val ) const { return getEntityLabel( val ); }

      bool getEntityAuthority( std::string& val ) const;
      bool getEUTH( std::string& val ) const { return getEntityAuthority( val ); }

      bool getAttributeLabel( std::string& val ) const;
      bool getATLB( std::string& val ) const { return getAttributeLabel( val ); }

      bool getAttributeAuthority( std::string& val ) const;
      bool getAUTH( std::string& val ) const { return getAttributeAuthority( val ); }

      bool getFormat( std::string& val ) const;
      bool getFMT( std::string& val ) const { return getFormat( val ); }

      bool getUnit( std::string& val ) const;
      bool getUNIT( std::string& val ) const { return getUnit( val ); }

      bool getPrecision( double& val ) const;
      bool getPREC( double& val ) const { return getPrecision( val ); }

      bool getMaximumSubfieldLength( long& val ) const;
      bool getMXLN( long& val ) const { return getMaximumSubfieldLength( val ); }

      bool getKey( std::string& val ) const;
      bool getKEY( std::string& val ) const { return getKey( val ); }


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DDSH record, this will return false
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

      bool setEntityLabel( std::string const& val );
      bool setETLB( std::string const& val ) { return setEntityLabel( val ); }

      bool setEntityAuthority( std::string const& val );
      bool setEUTH( std::string const& val ) { return setEntityAuthority( val ); }

      bool setAttributeLabel( std::string const& val );
      bool setATLB( std::string const& val ) { return setAttributeLabel( val ); }

      bool setAttributeAuthority( std::string const& val );
      bool setAUTH( std::string const& val ) { return setAttributeAuthority( val ); }

      bool setFormat( std::string const& val );
      bool setFMT( std::string const& val ) { return setFormat( val ); }

      bool setUnit( std::string const& val );
      bool setUNIT( std::string const& val ) { return setUnit( val ); }

      bool setPrecision( double val );
      bool setPREC( double val ) { return setPrecision( val ); }

      bool setMaximumSubfieldLength( long val );
      bool setMXLN( long val ) { return setMaximumSubfieldLength( val ); }

      bool setKey( std::string const& val );
      bool setKEY( std::string const& val ) { return setKey( val ); }



      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineName( );
      void unDefineNAME( ) { unDefineName( ); }

      void unDefineType( );
      void unDefineTYPE( ) { unDefineType( ); }

      void unDefineEntityLabel( );
      void unDefineETLB( ) { unDefineEntityLabel( ); }

      void unDefineEntityAuthority( );
      void unDefineEUTH( ) { unDefineEntityAuthority( ); }

      void unDefineAttributeLabel( );
      void unDefineATLB( ) { unDefineAttributeLabel( ); }

      void unDefineAttributeAuthority( );
      void unDefineAUTH( ) { unDefineAttributeAuthority( ); }

      void unDefineFormat( );
      void unDefineFMT( ) { unDefineFormat( ); }

      void unDefineUnit( );
      void unDefineUNIT( ) { unDefineUnit( ); }

      void unDefinePrecision( );
      void unDefinePREC( ) { unDefinePrecision( ); }

      void unDefineMaximumSubfieldLength( );
      void unDefineMXLN( ) { unDefineMaximumSubfieldLength( ); }

      void unDefineKey( );
      void unDefineKEY( ) { unDefineKey( ); }


   private:


      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

      /// NOT NEEDED
      sb_Ddsh(sb_Ddsh const& right);

      /// NOT NEEDED
      sb_Ddsh const& operator=(sb_Ddsh const& right);


      /// pointer to hidden opaque data structure
      sb_Ddsh_Imp* imp_;

}; // sb_Ddsh


#endif // INCLUDED_SB_DDSH_H


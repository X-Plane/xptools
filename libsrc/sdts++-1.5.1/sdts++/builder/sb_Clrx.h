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
// $Id: sb_Clrx.h,v 1.2 2001/05/22 05:58:34 mcoletti Exp $
//
#ifndef INCLUDED_SB_CLRX_H
#define INCLUDED_SB_CLRX_H

#include <list>
#include <string>

#ifndef SB_MODULE_H
#include "builder/sb_Module.h"
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include "io/sio_8211FieldFormat.h"
#endif

class  sb_ForeignID;
class  sc_Record;


// This class provides a convenient access to CLRX records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Clrx object.


struct sb_Clrx_Imp;

class sb_Clrx : public sb_Module
{
   public:

      sb_Clrx();

      ~sb_Clrx();


//@{
/**
   Use these members to get subfield/field values.  Pass in an appropriate
   type to receive the value.  These members will return false if the 
   corresponding value is not set.  (It may not be set because a value 
   was not assigned to it, or because you previously tried to assign 
   an invalid value.)  Otherwise they will return true.
*/
      bool getRedComponent( double& val ) const;
      bool getRED( double& val ) const { return getRedComponent( val ); }

      bool getGreenComponent( double& val ) const;
      bool getGREN( double& val ) const { return getGreenComponent( val ); }

      bool getBlueComponent( double& val ) const;
      bool getBLUE( double& val ) const { return getBlueComponent( val ); }

      bool getBlackComponent( double& val ) const;
      bool getBLCK( double& val ) const { return getBlackComponent( val ); }

//@}

/**
   fill the given record based on the builder's object field/subfield
   values -- return false if in a wedged state. (E.g., a mandatory
   field isn't set or was assigned a value outside its proper
   domain.
*/
      bool getRecord( sc_Record& val ) const;


/** fills the given schema with one appropriate for CLRX
    modules; returns false if unable to create the schema for some
    bizarre reason.  (Like maybe running out of memory.)  Note that
    an sio_Writer instance will need a schema generated from this
    member.
*/
      bool getSchema( sio_8211Schema& schema ) const;


/** set the object with values found in the record; if not a valid
    CLRX record, this will return false
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
      bool setRedComponent( double val );
      bool setRED( double val ) { return setRedComponent( val ); }

      bool setGreenComponent( double val );
      bool setGREN( double val ) { return setGreenComponent( val ); }

      bool setBlueComponent( double val );
      bool setBLUE( double val ) { return setBlueComponent( val ); }

      bool setBlackComponent( double val );
      bool setBLCK( double val ) { return setBlackComponent( val ); }

//@}

//@{
/**
   Since builder objects will be frequently 'recycled' (i.e., used for
   more than one record), it might be convenient to 'unset' a previously
   assigned value.  So:
*/

      void unDefineRedComponent( );
      void unDefineRED( ) { unDefineRedComponent( ); }

      void unDefineGreenComponent( );
      void unDefineGREN( ) { unDefineGreenComponent( ); }

      void unDefineBlueComponent( );
      void unDefineBLUE( ) { unDefineBlueComponent( ); }

      void unDefineBlackComponent( );
      void unDefineBLCK( ) { unDefineBlackComponent( ); }

//@}

   private:

      /// returns reference to schema
      sio_8211Schema& schema_();


      ///
      void buildSpecificSchema_();

      sb_Clrx(sb_Clrx const& right); // NOT NEEDED
      sb_Clrx const& operator=(sb_Clrx const& right); // NOT NEEDED


      sb_Clrx_Imp* _imp;

}; // sb_Clrx


#endif // INCLUDED_SB_CLRX_H


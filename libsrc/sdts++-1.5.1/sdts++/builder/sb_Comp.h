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
// $Id: sb_Comp.h,v 1.4 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_COMP_H
#define INCLUDED_SB_COMP_H


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


// This class provides a convenient access to COMP records.  It provides
// members to access or set various module field and subfield values.
// It also provides a mechanism for populating an object of this class with
// values found in a valid sc_Record of this module, and for filling a
// sc_Record with the contents of a sb_Comp object.


struct sb_Comp_Imp;

class sb_Comp : public sb_Module
{
   public:

      sb_Comp();

      ~sb_Comp();


      //@{
      /**
         Use these members to get subfield/field values.  Pass in an
         appropriate type to receive the value.  These members will
         return false if the corresponding value is not set.  (It may
         not be set because a value was not assigned to it, or because
         you previously tried to assign an invalid value.)  Otherwise
         they will return true.
      */
      bool getObjectRepresentation( std::string& val ) const;
      bool getOBRP( std::string& val ) const { return getObjectRepresentation( val ); }

      bool getAttributeID( std::list<std::string> & val ) const;
      bool getAttributeID( sb_AttributeIDs& val ) const;
      bool getATID( sb_AttributeIDs& val ) const { return getAttributeID( val ); }

      bool getForeignID(  sb_ForeignIDs& val ) const;
      bool getForeignID( std::list<std::string> & val ) const;
      bool getFRID( sb_ForeignIDs& val ) const { return getForeignID( val ); }

      bool getCompositeID(  sb_ForeignIDs& val ) const;
      bool getCompositeID( std::list<std::string> & val ) const;
      bool getCPID( sb_ForeignIDs& val ) const { return getCompositeID( val ); }

      //@}

      /**
         fill the given record based on the builder's object
         field/subfield values -- return false if in a wedged
         state. (E.g., a mandatory field isn't set or was assigned a
         value outside its proper domain.
      */
      bool getRecord( sc_Record& val ) const;


      /** fills the given schema with one appropriate for COMP
          modules; returns false if unable to create the schema for
          some bizarre reason.  (Like maybe running out of memory.)
          Note that an sio_Writer instance will need a schema
          generated from this member.  */
      bool getSchema( sio_8211Schema& schema ) const;


      /** set the object with values found in the record; if not a
          valid COMP record, this will return false */
      bool setRecord( sc_Record const& val );


      //@{
      /**
         Use these members to set subfield/field values.  Pass in an
         appropriate value for the particular subfield/field to be set
         to.  They will return false if you try to assign a value
         outside the domain of the given subfield/field.  (Note that
         this is not too pedantic; for example, we do not check to see
         if a conditionally mandatory or optional field has been set.)  */
      bool setObjectRepresentation( std::string const& val );
      bool setOBRP( std::string const& val ) { return setObjectRepresentation( val ); }

      bool setAttributeID( sb_AttributeIDs const& val );
      bool setATID( sb_AttributeIDs const& val ) { return setAttributeID( val ); }

      bool setForeignID( sb_ForeignIDs const& val );
      bool setFRID( sb_ForeignIDs const& val ) { return setForeignID( val ); }

      bool setCompositeID( sb_ForeignIDs const& val );
      bool setCPID( sb_ForeignIDs const& val ) { return setCompositeID( val ); }

      //@}

      //@{
      /**
         Since builder objects will be frequently 'recycled' (i.e.,
         used for more than one record), it might be convenient to
         'unset' a previously assigned value.  So: */

      void unDefineObjectRepresentation( );
      void unDefineOBRP( ) { unDefineObjectRepresentation( ); }

      void unDefineAttributeID( );
      void unDefineATID( ) { unDefineAttributeID( ); }

      void unDefineForeignID( );
      void unDefineFRID( ) { unDefineForeignID( ); }

      void unDefineCompositeID( );
      void unDefineCPID( ) { unDefineCompositeID( ); }

      //@}

   private:

      // Returns reference to schema
      virtual sio_8211Schema& schema_();

      virtual void buildSpecificSchema_();

      sb_Comp(sb_Comp const& right); // NOT NEEDED
      sb_Comp const& operator=(sb_Comp const& right); // NOT NEEDED


      sb_Comp_Imp* _imp;

}; // sb_Comp


#endif // INCLUDED_SB_COMP_H


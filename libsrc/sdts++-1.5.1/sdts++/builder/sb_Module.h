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

#ifndef SB_MODULE_H
#define SB_MODULE_H

// $Id: sb_Module.h,v 1.9 2002/11/24 22:07:42 mcoletti Exp $

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <string>


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


class sc_Record;
class sb_ForeignID;


/** 
    Consolidates common module attributes for the rest of the builder
    classes
*/

class sb_Module
{
   public:

      sb_Module() : mnemonic_(""), id_(1), emitRecIdenField_(true) {}

      virtual ~sb_Module() {}

      ///
      void getMnemonic( std::string & mnemonic ) const
      {
         mnemonic = mnemonic_;
      }

      ///
      const std::string & getMnemonic(  ) const
      {
         return mnemonic_;
      }

      ///
      int getID( ) const
      {
         return id_;
      }

      /// maintained for backward compatibility
      std::string const & getModuleName(  ) const
      {
         return mnemonic_;
      }

      /// maintained for backward compatibility
      int getRecordID( ) const
      {
         return id_;
      }

      ///
      void setMnemonic( std::string const & mnemonic ) 
      {
         mnemonic_ = mnemonic;
      }

      ///
      void setID( int id )
      {
         id_ = id;
      }

      ///
      void emitRecIdenField( bool v ) { emitRecIdenField_ = v; }

      /**
       returns true if the module will emit records with the ISO
       8211 record identifier
      */
      bool willEmitRecIdenField( ) const { return emitRecIdenField_; }

      /// returns the schema associated with the module
      bool getSchema( sio_8211Schema& schema );

      /**
       fills the given record with proper fields and subfields for
       them module; returns false if a mandatory field or subfield
       hasn't been given a proper value
      */
      virtual bool getRecord( sc_Record& ) const = 0;

      /**
       sets the module with the fields and subfields found in the
       given sc_Record; returns false if improper record for the
       module
      */
      virtual bool setRecord( sc_Record const& ) = 0;


      /// return a foreign id representing the current module and record name
      sb_ForeignID foreignID() const ;

   protected:

      /// returns reference to schema
      virtual sio_8211Schema& schema_() = 0;

      /**
       Each builder module has its own static copy of a relevent
       schema.  For example, sb_Iden will define a schema that
       describes the field formats necessary for creating an IDEN
       module.  However, these schemas are built only if they're
       needed; think of it as a form of lazy evauluation.  Moreover,
       there are some "shared" attributes common to all schemas,
       such as the format for the 0001 reserved field if
       willEmitRecIdenField() is true.  This function builds those
       parts.  The child sb_Module classes will define their local
       behavior in the buildSpecificSchema_(), which is called by
       this member.
      */
      void buildSchema_();

      ///
      virtual void buildSpecificSchema_() = 0;

   private:

      /// module mnemonic
      std::string mnemonic_;

      /// module record number
      int    id_;

      /// true if to write ISO 8211 record identifierfield
      bool   emitRecIdenField_; 

      friend std::ostream& operator<<( std::ostream&, sb_Module const& );

}; // sb_Module

#endif

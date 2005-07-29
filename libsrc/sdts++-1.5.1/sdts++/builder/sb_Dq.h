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
// $Id: sb_Dq.h,v 1.7 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_DQ_H
#define INCLUDED_SB_DQ_H

#include <list>
#include <string>

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

class  sc_Record;


struct sb_Dq_Imp;

/**
 This class provides a convenient access to DQ records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_dq object.
*/
class sb_Dq : public sb_Module
{
   public:

      sb_Dq();

      ~sb_Dq();


      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the 
       corresponding value is not set.  (It may not be set because a value 
       was not assigned to it, or because you previously tried to assign 
       an invalid value.)  Otherwise they will return true.
      */
      bool getComment( std::string& val ) const;
      bool getCOMT( std::string& val ) const { return getComment( val ); }

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const = 0;


      /**
       set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val ) = 0;


      /**
       Use these members to set subfield/field values.  Pass in an appropriate
       value for the particular subfield/field to be set to.  They will return
       false if you try to assign a value outside the domain of the given 
       subfield/field.  (Note that this is not too pedantic; for example, we
       do not check to see if a conditionally mandatory or optional field has
       been set.)
      */
      bool setComment( std::string const& val );
      bool setCOMT( std::string const& val ) { return setComment( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineComment( );
      void unDefineCOMT( ) { unDefineComment( ); }

   protected:

      ///
      sb_Dq_Imp& getImp() { return *imp_; }

      
      /// returns reference to schema
      virtual sio_8211Schema& schema_() = 0;

      ///
      virtual void buildSpecificSchema_() = 0;


   private:


      /// NOT NEEDED 
      sb_Dq(sb_Dq const& right);

      /// NOT NEEDED
      sb_Dq const& operator=(sb_Dq const& right);


      /// pointer to hidden, opaque data structure
      sb_Dq_Imp* imp_;

}; // sb_Dq



/** */
class sb_Dqhl : public sb_Dq
{
   public:

      sb_Dqhl();

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
         set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val );

   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

}; // sb_Dqhl


/** */
class sb_Dqpa : public sb_Dq
{
   public:

      sb_Dqpa();


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val );


   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

}; // sb_Dqpa



/** */
class sb_Dqaa : public sb_Dq
{
   public:

      sb_Dqaa();

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val );

   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

}; // sb_Dqaa


/** */
class sb_Dqlc : public sb_Dq
{
   public:

      sb_Dqlc();

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val );

   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

}; // sb_Dqlc



/** */
class sb_Dqcg : public sb_Dq
{
   public:

      sb_Dqcg();

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       DQ record, this will return false
      */
      bool setRecord( sc_Record const& val );

   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      ///
      void buildSpecificSchema_();

}; // sb_Dqcg


#endif // INCLUDED_SB_DQ_H


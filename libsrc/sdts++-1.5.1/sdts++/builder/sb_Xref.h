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

#ifndef INCLUDED_SB_XREF_H
#define INCLUDED_SB_XREF_H

// $Id: sb_Xref.h,v 1.7 2002/11/24 22:07:43 mcoletti Exp $ 

#include <list>
#include <string>


#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

//class  sb_ForeignID;
class  sc_Record;


struct sb_Xref_Imp;


/**
 This class provides a convenient access to XREF records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Xref object.
*/
class sb_Xref : public sb_Module
{
   public:

      sb_Xref();

      ~sb_Xref();

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the 
       corresponding value is not set.  (It may not be set because a value 
       was not assigned to it, or because you previously tried to assign 
       an invalid value.)  Otherwise they will return true.
      */
      bool getComment( std::string & ) const;
      bool getCOMT( std::string& val ) const { return getComment( val ); }

      bool getReferenceDocumentation( std::string & val ) const;
      bool getRDOC( std::string& val ) const { return getReferenceDocumentation( val ); }

      bool getReferenceSystemName( std::string & val ) const;
      bool getRSNM( std::string& val ) const { return getReferenceSystemName( val ); }

#ifdef NOT_RASTER_PROFILE
      bool getVerticalDatum( std::string & val ) const;
      bool getVDAT( std::string& val ) const { return getVerticalDatum( val ); }

      bool getSoundingDatum( std::string & val ) const;
      bool getSDAT( std::string& val ) const { return getSoundingDatum( val ); }
#endif

      bool getHorizontalDatum( std::string & val ) const;
      bool getHDAT( std::string& val ) const { return getHorizontalDatum( val ); }

      bool getZoneReferenceNumber( std::string & val ) const;
      bool getZONE( std::string& val ) const { return getZoneReferenceNumber( val ); }

      bool getProjection( std::string & val ) const;
      bool getPROJ( std::string& val ) const { return getProjection( val ); }


      // 

#ifdef NOT_IMPLEMENTED
      bool getAttID( list<sb_ForeignID> & ) const;

      bool getVerticalAttributes() const;

      bool getSoundingAttributes() const;
#endif


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside it's proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       XREF record, this will return false
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
      bool setComment( std::string const & ) ;
      bool setCOMT( std::string const & val ) { return setComment( val ); }

      bool setReferenceDocumentation( std::string  const & val ) ;
      bool setRDOC( std::string const & val ) { return setReferenceDocumentation( val ); }

      bool setReferenceSystemName( std::string  const & val ) ;
      bool setRSNM( std::string const & val ) { return setReferenceSystemName( val ); }

#ifdef NOT_RASTER_PROFILE
      bool setVerticalDatum( std::string  const & val ) ;
      bool setVDAT( std::string const & val ) { return setVerticalDatum( val ); }

      bool setSoundingDatum( std::string  const & val ) ;
      bool setSDAT( std::string const & val ) { return setSoundingDatum( val ); }
#endif

      bool setHorizontalDatum( std::string  const & val ) ;
      bool setHDAT( std::string const & val ) { return setHorizontalDatum( val ); }

      bool setZoneReferenceNumber( std::string  const & val ) ;
      bool setZONE( std::string const & val ) { return setZoneReferenceNumber( val ); }
      bool setZoneReferenceNumber( int val );
      bool setZONE( int val ) { return setZoneReferenceNumber( val ); }

      bool setProjection( std::string  const & val ) ;
      bool setPROJ( std::string const & val ) { return setProjection( val ); }


      // 

#ifdef NOT_IMPLEMENTED
      bool setAttID( list<sb_ForeignID>  const & ) ;

      bool setVerticalAttributes() ;

      bool setSoundingAttributes() ;
#endif




   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();

      
      /// NOT NEEDED
      sb_Xref(sb_Xref const& right);

      /// NOT NEEDED
      sb_Xref const& operator=(sb_Xref const& right);

      /// pointer to hidden opaque data structure
      sb_Xref_Imp* imp_;

}; // sb_Xref


#endif // INCLUDED_SB_XREF_H


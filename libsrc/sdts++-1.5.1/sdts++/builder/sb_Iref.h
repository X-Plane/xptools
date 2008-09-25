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
// $Id: sb_Iref.h,v 1.8 2002/11/24 22:07:42 mcoletti Exp $


#ifndef INCLUDED_SB_IREF_H
#define INCLUDED_SB_IREF_H

#include <vector>

#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif


//class sb_ForeignID;
class sc_Record;

const char* const TWO_TUPLE   = "2-TUPLE";
const char* const THREE_TUPLE = "3-TUPLE";
const char* const LATITUDE    = "LATITUDE";
const char* const LONGITUDE   = "LONGITUDE";
const char* const EASTING     = "EASTING";
const char* const NORTHING    = "NORTHING";


struct sb_Iref_Imp;


/**
 This class provides a convenient access to IREF records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Iref object.
*/
class sb_Iref : public sb_Module
{
   public:

      sb_Iref();
      ~sb_Iref();

      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the
       corresponding value is not set.  (It may not be set because a value
       was not assigned to it, or because you previously tried to assign
       an invalid value.)  Otherwise they will return true.
      */
      bool getComment( std::string& ) const;
      bool getCOMT( std::string& val ) const { return getComment( val ); }

      bool getSpatialAddressType( std::string& ) const;
      bool getSATP( std::string& val ) const
      { return getSpatialAddressType( val ); }

      bool getSpatialAddressXLabel( std::string& ) const;
      bool getXLBL( std::string& val ) const
      { return getSpatialAddressXLabel( val ); }

      bool getSpatialAddressYLabel( std::string& ) const;
      bool getYLBL( std::string& val ) const
      { return getSpatialAddressYLabel( val ); }

      bool getHorizontalComponentFormat( std::string& ) const;
      bool getHFMT( std::string& val ) const
      { return getHorizontalComponentFormat( val ); }

#ifdef NOT_RASTER_PROFILE
      bool getVerticalComponentFormat( std::string& ) const;
      bool getVFMT( std::string& val ) const
      { return getVerticalComponentFormat( val ); }
#endif

      bool getScaleFactorX( double& ) const;
      bool getSFAX( double& val ) const { return getScaleFactorX( val ); }

      bool getScaleFactorY( double& ) const;
      bool getSFAY( double& val ) const { return getScaleFactorY( val ); }

#ifdef NOT_RASTER_PROFILE
      bool getScaleFactorZ( double& ) const;
      bool getSFAZ( double& val ) const { return getScaleFactorZ( val ); }
#endif

      bool getXOrigin( double& ) const;
      bool getXORG( double& val ) const { return getXOrigin( val ); }

      bool getYOrigin( double& ) const;
      bool getYORG( double& val ) const { return getYOrigin( val ); }

#ifdef NOT_RASTER_PROFILE
      bool getZOrigin( double& ) const;
      bool getZORG( double& val ) const { return getZOrigin( val ); }
#endif

      bool getXComponentHorizontalResolution( double& ) const;
      bool getXHRS( double& val ) const
      { return getXComponentHorizontalResolution( val ); }

      bool getYComponentHorizontalResolution( double& ) const;
      bool getYHRS( double& val ) const
      { return getYComponentHorizontalResolution( val ); }

#ifdef NOT_RASTER_PROFILE
      bool getVerticalResolutionComponent( double& ) const;
      bool getVRES( double& val ) const
      { return getVerticalResolutionComponent( val ); }
#endif

#ifdef NOT_IMPLEMENTED
      bool getDimensionID( std::vector<sb_ForeignID>& ) const;
#endif

      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside it's proper
       domain.
      */
      bool getRecord( sc_Record& record ) const;

      /**
       Use these members to set subfield/field values.  Pass in an appropriate
       value for the particular subfield/field to be set to.  They will return
       false if you try to assign a value outside the domain of the given
       subfield/field.  (Note that this is not too pedantic; for example, we
       do not check to see if a conditionally mandatory or optional field has
       been set.)
      */
      void setComment( std::string const & ) ;
      void setCOMT( std::string const & val )  { setComment( val ); }

      void setSpatialAddressType( std::string const & ) ;
      void setSATP( std::string const & val )
      { setSpatialAddressType( val ); }

      void setSpatialAddressXLabel( std::string const & ) ;
      void setXLBL( std::string const & val )
      { setSpatialAddressXLabel( val ); }

      void setSpatialAddressYLabel( std::string const & ) ;
      void setYLBL( std::string const & val )
      { setSpatialAddressYLabel( val ); }

      void setHorizontalComponentFormat( std::string const & ) ;
      void setHFMT( std::string const & val )
      { setHorizontalComponentFormat( val ); }

#ifdef NOT_RASTER_PROFILE
      void setVerticalComponentFormat( std::string const & ) ;
      void setVFMT( std::string const & val )
      { setVerticalComponentFormat( val ); }
#endif

      void setScaleFactorX( double const & ) ;
      void setSFAX( double const & val )  { setScaleFactorX( val ); }

      void setScaleFactorY( double const & ) ;
      void setSFAY( double const & val )  { setScaleFactorY( val ); }

#ifdef NOT_RASTER_PROFILE
      void setScaleFactorZ( double const & ) ;
      void setSFAZ( double const & val )  { setScaleFactorZ( val ); }
#endif

      void setXOrigin( double const & ) ;
      void setXORG( double const & val )  { setXOrigin( val ); }

      void setYOrigin( double const & ) ;
      void setYORG( double const & val )  { setYOrigin( val ); }

#ifdef NOT_RASTER_PROFILE
      void setZOrigin( double const & ) ;
      void setZORG( double const & val )  { setZOrigin( val ); }
#endif

      void setXComponentHorizontalResolution( double const & ) ;
      void setXHRS( double const & val )
      { setXComponentHorizontalResolution( val ); }

      void setYComponentHorizontalResolution( double const & ) ;
      void setYHRS( double const & val )
      { setYComponentHorizontalResolution( val ); }

#ifdef NOT_RASTER_PROFILE
      void setVerticalResolutionComponent( double const & ) ;
      void setVRES( double const & val )
      { setVerticalResolutionComponent( val ); }
#endif

#ifdef NOT_IMPLEMENTED
      void setDimensionID( std::vector<sb_ForeignID> const & ) ;
#endif

      /**
       set the object with values found in the record; if not a valid
       IREF record, this will return false
      */
      bool setRecord( sc_Record const & record );


   private:


      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();

      /// NOT NEEDED
      sb_Iref( sb_Iref const& );

      /// pointer to hidden opaque data structure
      sb_Iref_Imp* imp_;

}; // class sb_Iref


#endif // INCLUDED_SB_IREF_H

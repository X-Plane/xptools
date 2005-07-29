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
// $Id: sb_Cell.h,v 1.13 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_CELL_H
#define INCLUDED_SB_CELL_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <strstream>

#include <limits.h>
#include <float.h>
#include <list>
#include <string>
#include <iterator>


#ifndef SB_MODULE_H
#include <sdts++/builder/sb_Module.h>
#endif

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

#ifndef INCLUDED_SB_UTILS_H
#include <sdts++/builder/sb_Utils.h>
#endif

#ifndef INCLUDED_SB_FOREIGNID_H
#include <sdts++/builder/sb_ForeignID.h>
#endif

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif

#ifndef INCLUDED_SC_FIELD_H
#include <sdts++/container/sc_Field.h>
#endif

#ifndef INCLUDED_SC_SUBFIELD_H
#include <sdts++/container/sc_Subfield.h>
#endif

#ifndef INCLUDED_SIO_8211CONVERTER_H
#include <sdts++/io/sio_8211Converter.h>
#endif


using namespace std;

/**
 Strings and integers are initialized with these values; they are used
 to indicate whether a given module value has been assigned a value or not.

 (XXX I arbitrarily chose 0x4 as the sentinal value.  I hate ad hoc crap.)
*/
const string  UNVALUED_STRING(1, static_cast<string::value_type>(0x4) );

const long    UNVALUED_LONG   = INT_MIN;

const double  UNVALUED_DOUBLE = DBL_MAX;


// XXX kludge!!

extern sio_8211Converter_I converter_I; // XXX should define these in
extern sio_8211Converter_A converter_A; // XXX sio_8211Converter.h
extern sio_8211Converter_R converter_R;
extern sio_8211Converter_C converter_C;


/**
 This class provides a convenient access to CELL records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Cell object.

 WARNING!! This is only set up for very, very basic CELL module
 READ__ support.  Check the XXX comments in the implementation for
 more information on what to do to add WRITE__ support and to handle
 reading bizarre CELL module configurations.
*/
template <class Container>
class sb_Cell : public sb_Module
{
   public:

      sb_Cell( int index = 0 );

      ~sb_Cell();


      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the 
       corresponding value is not set.  (It may not be set because a value 
       was not assigned to it, or because you previously tried to assign 
       an invalid value.)  Otherwise they will return true.
      */
      bool getRowI( long& val ) const;
      bool getROWI( long& val ) const { return getRowI( val ); }

      bool getColI( long& val ) const;
      bool getCOLI( long& val ) const { return getColI( val ); }


      /**
       insert all the elevation postings into the container referenced
       by the given back inserter

      */
      bool loadData( back_insert_iterator<Container> & ) const;
                                


      /**
       fill the given record based on the builder's object field/subfield
       values -- return false if in a wedged state. (E.g., a mandatory
       field isn't set or was assigned a value outside its proper
       domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       CELL record, this will return false
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
      bool setRowI( long val );
      bool setROWI( long val ) { return setRowI( val ); }

      bool setColI( long val );
      bool setCOLI( long val ) { return setColI( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineRowI( );
      void unDefineROWI( ) { unDefineRowI( ); }

      void unDefineColI( );
      void unDefineCOLI( ) { unDefineColI( ); }



   private:

      /// NOT NEEDED
      sb_Cell(sb_Cell const& right);

      /// NOT NEEDED
      sb_Cell const& operator=(sb_Cell const& right);


      /// returns reference to schema
      virtual sio_8211Schema& schema_();

      ///
      virtual void buildSpecificSchema_();

      ///
      long     RowI_;
      ///
      long     ColI_;

      ///
      int      index_;              // what Nth cell are we?

      ///
      sc_Record const* record_;     // pointer to record for last setRecord()

      ///
      static sio_8211Schema cell_schema_; // module specific schema

      ///
      bool ingest_record_( sb_Cell<Container>& cell, 
                           sc_Record const& record );

      ///
      void build_schema_( sio_8211Schema& schema );

}; // sb_Cell


///
template <class T>
sio_8211Schema sb_Cell<T>::cell_schema_;



/** */
template <class T>
inline
sb_Cell<T>::sb_Cell( int index )
   : RowI_( UNVALUED_LONG ),
     ColI_( UNVALUED_LONG ),
     index_( index ),
     record_( 0x0 )
{
                                // CELL modules are rather odd in that
                                // the fourth character differentiates
                                // between N modules.  So we append
                                // the number of the cell module to
                                // the mnemonic.
   std::strstream tmp_ss;

   tmp_ss << "CEL" << index_ << ends;

   std::string tmp_string;

   getline( tmp_ss, tmp_string );

   setMnemonic( tmp_string );
   setID( 1 );


   // insert static initializers

} // Cell ctor



///
template <class T>
inline
sb_Cell<T>::~sb_Cell()
{
} // Cell dtor



///
template <class T>
bool
sb_Cell<T>::ingest_record_( sb_Cell<T>&      cell, 
                            sc_Record const& record )
{
                                // Make sure we have a record from an
                                // External Spatial Reference module.

   sc_FieldCntr::const_iterator curfield;

   if ( ! sb_Utils::getFieldByMnem( record, "CELL", curfield) )
   {
#ifdef SDTSXX_DEBUG
      cerr << "sb_Cell<T>::sb_Cell(sc_Record const&): "
           << "Not an cell record.";
      cerr << endl;
#endif
      return false;
   }


   // We have a primary field from a  module. Start// picking it apart.

   sc_SubfieldCntr::const_iterator cursubfield;

   long   tmp_int;
   std::string tmp_str;

   // MODN
   if (sb_Utils::getSubfieldByMnem(*curfield,"MODN",cursubfield))
   {
      cursubfield->getA( tmp_str );
      cell.setMnemonic( tmp_str );
   }


   // RCID
   if (sb_Utils::getSubfieldByMnem(*curfield,"RCID",cursubfield))
   {
      cursubfield->getI( tmp_int );
      cell.setID( tmp_int );
   }


   // ROWI
   if (sb_Utils::getSubfieldByMnem(*curfield,"ROWI",cursubfield))
   {
      cursubfield->getI( RowI_ );
   }
   else
   {
      return false;
   }


   // COLI
   if (sb_Utils::getSubfieldByMnem(*curfield,"COLI",cursubfield))
   {
      cursubfield->getI( ColI_ );
   }
   else
   {
      return false;
   }


                                // now snarf all the elevation postings

   if ( sb_Utils::getFieldByMnem( record, "CVLS", curfield ) )
   {
   }
   else
   {
      return false;             // no elevation postings?  Um, no__.
   }


   return true;


} // ingest_record_



///
template <class T>
inline
bool
sb_Cell<T>::getRowI( long& val ) const
{
   if ( RowI_ == UNVALUED_LONG )
      return false;

   val = RowI_;

   return true;
} // sb_Cell<T>::getRowI


///
template <class T>
inline
bool
sb_Cell<T>::getColI( long& val ) const
{
   if ( ColI_ == UNVALUED_LONG )
      return false;

   val = ColI_;

   return true;
} // sb_Cell<T>::getColI





/// XXX FIXME: NEEDS TO BE FINISHED FOR WRITING SEMANTICS
template <class T>
inline
bool
sb_Cell<T>::getRecord( sc_Record & record ) const
{
   record.clear();               // start with a clean slate

   // first field, which contains module name and record number

   record.push_back( sc_Field() );

   record.back().setMnemonic( "CELL" );

   record.back().setName( "Cell" );

   std::string tmp_str;
   long   tmp_long;

   getMnemonic( tmp_str );
   sb_Utils::add_subfield( record.back(), "MODN", tmp_str );
   sb_Utils::add_subfield( record.back(), "RCID", getID() );

   if ( getRowI( tmp_long ) )
   {
      sb_Utils::add_subfield( record.back(),"ROWI", tmp_long );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "ROWI", sc_Subfield::is_I );
   }


   if ( getColI( tmp_long ) )
   {
      sb_Utils::add_subfield( record.back(),"COLI", tmp_long );
   }
   else
   {
      sb_Utils::add_empty_subfield( record.back(), "COLI", sc_Subfield::is_I );
   }


   return true;


} // Cell::getRecord



///
template <class T>
inline
bool
sb_Cell<T>::setRowI( long val )
{
   RowI_ = val;

   return true;
} // sb_Cell<T>::setRowI



///
template <class T>
inline
bool
sb_Cell<T>::setColI( long val )
{
   ColI_ = val;

   return true;
} // sb_Cell<T>::setColI



///
template <class T>
inline
bool
sb_Cell<T>::setRecord( sc_Record const& record )
{
   record_ = &record;            // remember this for loadData
   return ingest_record_( *this, record );
} // sb_Cell<T>::setRecord



///
template <class T>
inline
void
sb_Cell<T>::unDefineRowI( )
{
   RowI_ = UNVALUED_LONG;
} // sb_Cell<T>::unDefineRowI



///
template <class T>
inline
void
sb_Cell<T>::unDefineColI( )
{
   ColI_ = UNVALUED_LONG;
} // sb_Cell<T>::unDefineColI



///
template <class T>
bool
sb_Cell<T>::loadData( back_insert_iterator<T> & bi ) const
{
                                // load up all the elevation values

   sc_Record::const_iterator cvls_field;

   if ( record_ && sb_Utils::getFieldByMnem( *record_, 
                                                 "CVLS", 
                                             cvls_field ) )
   {
      long            tmp_long;
      unsigned long   tmp_ulong;
      float           tmp_float;
      double          tmp_double;

      sc_Field::const_iterator elevation_subfield;

      for ( ;
            cvls_field != record_->end();
            cvls_field++ )
      {
                                // each repeating field should contain
                                // only one subfield value

         elevation_subfield = cvls_field->begin();

         switch( elevation_subfield->getSubfieldType() )
         {
            case sc_Subfield::is_I :
               if ( ! elevation_subfield->getI( tmp_long ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_R :
               if ( ! elevation_subfield->getR( tmp_double ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_S :
               if ( ! elevation_subfield->getS( tmp_double ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_BI8 :
               if ( ! elevation_subfield->getBI8( tmp_long ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_BI16 :
               if ( ! elevation_subfield->getBI16( tmp_long ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_BI24 :
               if ( ! elevation_subfield->getBI24( tmp_long ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_BI32 :
               if ( ! elevation_subfield->getBI32( tmp_long ) )
               { return false; }
               *bi = tmp_long;
               break;

            case sc_Subfield::is_BUI8 :
               if ( ! elevation_subfield->getBUI8( tmp_ulong ) )
               { return false; }
               *bi = tmp_ulong;
               break;

            case sc_Subfield::is_BUI16 :
               if ( ! elevation_subfield->getBUI16( tmp_ulong ) )
               { return false; }
               *bi = tmp_ulong;
               break;

            case sc_Subfield::is_BUI24 :
               if ( ! elevation_subfield->getBUI24( tmp_ulong ) )
               { return false; }
               *bi = tmp_ulong;
               break;

            case sc_Subfield::is_BUI32 :
               if ( ! elevation_subfield->getBUI32( tmp_ulong ) )
               { return false; }
               *bi = tmp_ulong;
               break;

            case sc_Subfield::is_BFP32 :
               if ( ! elevation_subfield->getBFP32( tmp_float ) )
               { return false; }
               *bi = tmp_float;
               break;

            case sc_Subfield::is_BFP64 :
               if ( ! elevation_subfield->getBFP64( tmp_double ) )
               { return false; }
               *bi = tmp_double;
               break;

            default :           // bogus elevation fvalues
               return false;

         }
      }
   }
   else
   {
      return false;
   }

   return true;
} // sb_Cell<T>::loadData



///
template <class T>
sio_8211Schema&
sb_Cell<T>::schema_()
{
   return cell_schema_;
} // sb_Cell<T>::schema_



///
template <class T>
void
sb_Cell<T>::buildSpecificSchema_( )
{
   schema_().push_back( sio_8211FieldFormat() );

   sio_8211FieldFormat& field_format = schema_().back();

   field_format.setDataStructCode( sio_8211FieldFormat::vector );
   field_format.setDataTypeCode( sio_8211FieldFormat::mixed_data_type );
   field_format.setName( "Cell" );
   field_format.setTag( "CELL" );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "MODN" );
   field_format.back().setType( sio_8211SubfieldFormat::A );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_A );

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "RCID" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_I );

   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "ROWI" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_I );


   field_format.push_back( sio_8211SubfieldFormat() );

   field_format.back().setLabel( "COLI" );
   field_format.back().setType( sio_8211SubfieldFormat::I );
   field_format.back().setFormat( sio_8211SubfieldFormat::variable );
   field_format.back().setConverter( &converter_I );


   // XXX NEED TO ADD SUPPORT FOR VARIANT CVLS TYPES!!


} // build_schema_







#endif // INCLUDED_SB_CELL_H


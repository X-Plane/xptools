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
// $Id: sb_Catd.h,v 1.12 2002/11/24 22:07:42 mcoletti Exp $
//
#ifndef INCLUDED_SB_CATD_H
#define INCLUDED_SB_CATD_H

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


///
struct sb_Catd_Imp;


/**
 This class provides a convenient access to CATD records.  It provides
 members to access or set various module field and subfield values.
 It also provides a mechanism for populating an object of this class with
 values found in a valid sc_Record of this module, and for filling a
 sc_Record with the contents of a sb_Catd object.
*/
class sb_Catd : public sb_Module
{
   public:

      sb_Catd();

      sb_Catd( sb_Catd const & rhs );

      sb_Catd const& operator=(sb_Catd const& right);


      ~sb_Catd();


      /**
       Use these members to get subfield/field values.  Pass in an appropriate
       type to receive the value.  These members will return false if the
       corresponding value is not set.  (It may not be set because a value
       was not assigned to it, or because you previously tried to assign
       an invalid value.)  Otherwise they will return true.
      */
      bool getName( std::string& val ) const;

      /// \sa getName( string& val )
      bool getNAME( std::string& val ) const { return getName( val ); }

      /// \sa getTYPE( string& val )
      bool getType( std::string& val ) const;
      /// \sa getType( string& val )
      bool getTYPE( std::string& val ) const { return getType( val ); }

      /// \sa getVOLM( string& val )
      bool getVolume( std::string& val ) const;
      /// \sa getVolume( string& val )
      bool getVOLM( std::string& val ) const { return getVolume( val ); }

      /// \sa getFILE( string& val )
      bool getFile( std::string& val ) const;
      /// \sa getFile( string& val )
      bool getFILE( std::string& val ) const { return getFile( val ); }


#ifdef NOT_SUPPORTED
      bool getRecord( std::string& val ) const;
      bool getRECD( std::string& val ) const { return getRecord( val ); }
#endif

      /// \sa getEXTR( string& val )
      bool getExternal( std::string& val ) const;
      /// \sa getExternal( string& val )
      bool getEXTR( std::string& val ) const { return getExternal( val ); }

      /// \sa getMVER( string& val )
      bool getModuleVersion( std::string& val ) const;
      /// \sa getModuleVersion( string& val )
      bool getMVER( std::string& val ) const { return getModuleVersion( val ); }

      /// \sa getCOMT( string& val )
      bool getComment( std::string& val ) const;
      /// \sa getComment( string& val )
      bool getCOMT( std::string& val ) const { return getComment( val ); }


      /**
         fill the given record based on the builder's object
       field/subfield values -- return false if in a wedged
       state. (E.g., a mandatory field isn't set or was assigned a
       value outside its proper domain.
      */
      bool getRecord( sc_Record& val ) const;


      /**
       set the object with values found in the record; if not a valid
       CATD record, this will return false
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
      /// \sa setName( string const& val )
      bool setNAME( std::string const& val ) { return setName( val ); }

      /// \sa setTYPE( string const& val )
      bool setType( std::string const& val );
      /// \sa setType( string const& val )
      bool setTYPE( std::string const& val ) { return setType( val ); }

      /// \sa setVOLM( string const& val )
      bool setVolume( std::string const& val );
      /// \sa setVolume( string const& val )
      bool setVOLM( std::string const& val ) { return setVolume( val ); }

      /// \sa setFILE( string const& val )
      bool setFile( std::string const& val );
      /// \sa setFile( string const& val )
      bool setFILE( std::string const& val ) { return setFile( val ); }

#ifdef NOT_SUPPORTED
      bool setRecord( std::string const& val );
      bool setRECD( std::string const& val ) { return setRecord( val ); }
#endif

      /// \sa setEXTR( string const& val )
      bool setExternal( std::string const& val );
      /// \sa setExternal( string const& val )
      bool setEXTR( std::string const& val ) { return setExternal( val ); }

      /// \sa setMVER( string const& val )
      bool setModuleVersion( std::string const& val );
      /// \sa setModuleVersion( string const& val )
      bool setMVER( std::string const& val ) { return setModuleVersion( val ); }

      /// \sa setCOMT( string const& val )
      bool setComment( std::string const& val );
      /// \sa setComment( string const& val )
      bool setCOMT( std::string const& val ) { return setComment( val ); }


      /**
       Since builder objects will be frequently 'recycled' (i.e., used for
       more than one record), it might be convenient to 'unset' a previously
       assigned value.  So:
      */
      void unDefineName( );
      /// \sa void unDefineName( )
      void unDefineNAME( ) { unDefineName( ); }

      /// \sa void unDefineTYPE( )
      void unDefineType( );
      /// \sa void unDefineType( )
      void unDefineTYPE( ) { unDefineType( ); }

      /// \sa void unDefineVOLM( )
      void unDefineVolume( );
      /// \sa void unDefineVolume( )
      void unDefineVOLM( ) { unDefineVolume( ); }

      /// \sa void unDefineFILE( )
      void unDefineFile( );
      /// \sa void unDefineFile( )
      void unDefineFILE( ) { unDefineFile( ); }

#ifdef NOT_SUPPORTED
      void unDefineRecord( );
      void unDefineRECD( ) { unDefineRecord( ); }
#endif

      /// \sa void unDefineEXTR( )
      void unDefineExternal( );
      /// \sa void unDefineExternal( )
      void unDefineEXTR( ) { unDefineExternal( ); }

      /// \sa void unDefineMVER( )
      void unDefineModuleVersion( );
      /// \sa void unDefineModuleVersion( )
      void unDefineMVER( ) { unDefineModuleVersion( ); }

      /// \sa void unDefineCOMT( )
      void unDefineComment( );
      /// \sa void unDefineComment( )
      void unDefineCOMT( ) { unDefineComment( ); }


   private:

      /// returns reference to schema
      sio_8211Schema& schema_();

      /// used to create internal schema
      void buildSpecificSchema_();



      /// pointer to opaque data structure
      sb_Catd_Imp* imp_;

}; // sb_Catd



/// class for conveniently converting SDTS module names to actual file names
/**

   Some SDTS modules refer to other modules by just a name (e.g.,
   CEL0) that can be cross-referenced in the CATD module to yeild an
   actual file name (e.g., ABCDCEL0.DDF).  This class provides a
   convenient mechanism for performing these kinds of look-ups.

   \todo XXX should this be here or in a separate file space?

 */
class sb_Directory
{
   public:

      sb_Directory();
      sb_Directory( std::string const & catd_filename );

      ~sb_Directory();

      /// looks up the module file name and sets module_info to what it finds
      /**
         Returns false if it cannot find a CATD entry with that module_name,
         or if the sb_Directory is uninitialized (i.e., it doesn't have an
         associated CATD file name yet).
      */
      bool find( std::string const & module_name,
                 sb_Catd & module_info ) const;


      /// return the CATD file name
      std::string const & catdFilename() const ;


      /// set a new CATD file name
      /**
         returns false if unable to read from the given CATD file name

         \note this initializes the internal data structure so that
         find() should now work
      */
      bool catdFilename( std::string const &  catd_filename)  ;

   private:

      struct Imp;

      sb_Directory::Imp * imp_;

}; // class sb_Directory


#endif // INCLUDED_SB_CATD_H


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

#ifndef INCLUDED_SB_FOREIGNID_H
#define INCLUDED_SB_FOREIGNID_H

// $Id: sb_ForeignID.h,v 1.16 2002/11/24 22:07:42 mcoletti Exp $

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <string>
#include <list>

#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

class sc_Field;

/**
 Note that this is for a PACKED foreign id.  The Right Way (tm) to
 do this would be to have a base foreign id class that explicitly
 uses fields and subfields to hold the module name, record id, and
 usage modifier parameters; a packed foreign id class would merely
 be a subclass of that.  However, 99% of the time, we'll be using
 packed foreign id's, so I've only implemented that as the one
 foreign id class.
*/
class sb_ForeignID
{
   public:

      /// foreign identifier usage type
      typedef enum { none,
                     start_node,
                     end_node,
                     left_polygon,
                     right_polygon,
                     forward_orientation,
                     backward_orientation,
                     interior_polygon,
                     exterior_polygon } usage_t;

      sb_ForeignID() ;


      sb_ForeignID( std::string const& name,
                    std::string const& mnemonic );

      sb_ForeignID( std::string const& mn, long id, usage_t um = none );

      virtual ~sb_ForeignID() 
      {}


      /// get foreign identifier name, as shown in the 8211 DDR
      std::string const& name() const { return name_; }

      /// set foreign identifier name, as shown in the 8211 DDR
      void name( std::string const & n )  { name_ = n; }


      /// get foreign identifier mnemonic, as shown in the 8211 DDR
      std::string const& mnemonic() const { return mnemonic_; }

      /// set foreign identifier mnemonic, as shown in the 8211 DDR
      void mnemonic( std::string const & n )  { mnemonic_ = n; }

      

      /// foriegn identifier mnemonic, as shown in the 8211 DDR

      /// get the remote module name
      std::string const& moduleName() const { return moduleName_; }

      /// set the module name
      void moduleName( std::string const& mn ) { moduleName_ = mn; }


      /// get the remote record ID
      long recordID() const { return recordID_; }

      /// set the record id
      void recordID( long id ) { recordID_ = id; }


      /// get the usage modifier
      /**
         \note
          This will be an empty string if one isn't defined for this
          foreign ID
      */
      usage_t const& usageModifier() const { return usageModifier_; }

      /// set the usage modified
      void usageModifier( usage_t um ) { usageModifier_ = um; }


      /**
         convert the foreign id into a packed string value

         assigns a packed foreign identifier string to the given
         string; returns false if the module name, record id, and /or
         usage modifier are bogus 

         @returns false if the foreign id doesn't have a modue name, an inval;id record id, or an improper usage modifier
      */
      bool packedIdentifierString( std::string & ) const;


      
      /**
         Adds the appropriate ISO 8211 field DDR field definitions for
         the given schema.

         @todo XXX adding name and mnemonic, which is separate from the name and mnemonic stored with each external module reference is confusing

         @todo XXX this is more a static member utility function

         @param schema is the schema we'll be adding this foreign id to
         @param name defines the foreign id's full name.
         @param mnemonic defines the foreign id's menmonic.
         @param isRepeating dictates whether this will be a repeating foreign id or not

      */
      void addFieldToSchema( sio_8211Schema    & schema,
                             std::string const & name = "",
                             std::string const & mnemonic = "",
                             bool                isRepeating = false ) const;


      /**
         Take an sc_Field that allegedly contains a foreign
         identifier, and assign it to this.  Return false if the
         sc_Field doesn't contain a foreign identifier.
      */
      bool assign( sc_Field const & field );

      ///
      bool operator==( sb_ForeignID const & rhs )
      {
         if ( &rhs == this ) { return true; }

         return rhs.moduleName_ == moduleName_ &&
            rhs.recordID_       == recordID_ &&
            rhs.usageModifier_  == usageModifier_;
      }

   private:

      /// module name
      std::string moduleName_;

      /// record ID
      long        recordID_;

      /// usage modifier
      usage_t     usageModifier_;

      /// 8211 DDR name
      std::string name_;

      /// 8211 DDR mnemonic
      std::string mnemonic_;

}; // sb_ForeignID


/// typedef for foreign id container
typedef std::list<sb_ForeignID> sb_ForeignIDs;




/**
   This defines an SDTS attribute ID

   An attribute is essentially a foreign ID in structure.  Just the
   field names and tags differ.  
*/
class sb_AttributeID : public sb_ForeignID
{
   public:

      sb_AttributeID();

      sb_AttributeID( std::string const& mn,
                      long id,
                      sb_ForeignID::usage_t um = sb_ForeignID::none );


}; // class sb_AttributeID



/// typedef for attribute id container
typedef std::list<sb_AttributeID> sb_AttributeIDs;



#endif  // INCLUDED_SB_FOREIGNID_H

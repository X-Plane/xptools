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
//
// $Id: sc_Subfield.h,v 1.10 2003/01/03 20:41:05 mcoletti Exp $
//

#ifndef INCLUDED_SCSUBFIELD_HXX
#define INCLUDED_SCSUBFIELD_HXX

#include <iostream>
#include <string>

#ifndef INCLUDED_SCMULTITYPEVALUE_HXX
#include <sdts++/container/sc_MultiTypeValue.h>
#endif



/**
   \note
  
         * Attribute subfields should have their mnemonic set to
         either "ATTP" or "ATTS", and their name set to the actual
         name of the attribute. Non- attribute subfields should have
         their mnemonic set to one of the subfield mnemonics defined
         by the SDTS. The name of a non-attribute subfield does not
         have to be set, and will not be if read from a transfer.

         Also, it's possible for a subfield to have a type, but have
         no value.  A subfield starts out this by default and can be
         set to this null state via the setUnvalued() member.  This
         is useful if you have to have a 'placeholder' subfield in
         an sc_Record that doesn't have any value at all.

   This class represents an SDTS Subfield. A Subfield has a value
   that is one of several types.

   Using one of the 'set' methods sets both the value *and* the
   type.  There is no way to set a value without setting the
   type. There is no way to "lock" the type; setting an instance to
   a new value with a different type will change the type associated
   with the instance.  This models the behavior of a variable whose
   type has run-time binding.  The type of the variable depends on
   the type of the value assigned to it.

   'get'ting the value of an instance will fail unless the member
   function corresponding to the current type of the instance is
   used.  For example, getting a string value for a subfield set as
   a BI32 will fail.

   An SDTS Subfield has the following attributes:
      Type:
         "The Data Type shall indicate the manner in which the subfield
         shall be encoded." -- The SDTS, section 4.2.1 "Specification Layout".
         The data type is one of:
            A: graphic characters, alphanumeric characters, or 
               alphabetic characters.
            I: implicit-point (integer)
            R: explicit-point unscaled (fixed point real)
            S: explicit-point scaled (floating point real)
            B: bitfield data
            C: character mode bitfield (binary in zero and one characters)
  
         The 'B' type may have additional qualification as follows:
            BI8:    8 bit signed integer
            BI16:   16 bit signed integer
            BI24:   24 bit signed integer
            BI32:   32 bit signed integer
            BUI:    unsigned integer, length specified by implementation
            BUI8:   8 bit unsigned integer
            BUI16:  16 bit unsigned integer
            BUI24:  24 bit unsigned integer
            BUI32:  32 bit unsigned integer
            BFP32:  32 bit floating point real
            BFP64:  64 bit floating point real

         In an sc_Subfield, the types listed above are actually
         stored using one of long, unsigned long, double, string or
         vector<char>. The mapping from SDTS data type to C++ is as
         follows:

            A                            string
            I                            long
            R,S                          double
            C                            Not Implemented
            BUI                          Not Implemented
            BI8, BI16, BI24, BI32        long
            BUI8, BUI16, BUI24, BUI32    unsigned long
            BFP32                        float
            BFP64                        double
  
      Value:
         The actual value stored in the Subfield.
  
      Name:
         A string representing the subfield name ("Module Name",
         "Record ID", etc).
  
      Mnemonic:
         A string representing the subfield mnemonic ("MODN", "RCID", etc).
   


*/
class sc_Subfield
{
   public:

      /// Denotes a subfield's type.
      enum SubfieldType { is_A,
                          is_I, 
                          is_R, 
                          is_S, 
                          is_C,
                          is_B,
                          is_BI8,
                          is_BI16,
                          is_BI24,
                          is_BI32,
                          is_BUI,
                          is_BUI8,
                          is_BUI16,
                          is_BUI24,
                          is_BUI32,
                          is_BFP32,
                          is_BFP64
      };

      ///
      sc_Subfield();

      ///
      sc_Subfield( std::string const& name, std::string const& mnemonic );

      ///
      sc_Subfield(sc_Subfield const& right);

      ///
      ~sc_Subfield();

      ///
      sc_Subfield& operator=(sc_Subfield const& right);

      ///
      bool operator==(sc_Subfield const& right) const;

      ///
      bool operator!=(sc_Subfield const& right) const;

      ///
      SubfieldType getSubfieldType() const;

      /// Returns the SDTS Name of this subfield (if one has been set).      
      std::string const& getName() const;

      /// abbreviation for getName()
      std::string const &name() const { return getName(); }

      /// Returns the SDTS Mnemonic of this subfield (if one has been set).
      std::string const& getMnemonic() const;

      /// abbreviation for getMnemonic()
      std::string const& mnemonic() const { return getMnemonic(); }

      ///
      bool getA(std::string& val) const;

      ///
      bool getI(long& val) const;

      ///
      bool getR(double& val) const;

      ///
      bool getS(double& val) const;

      ///
      bool getC(std::string& val) const;

      ///
      bool getBI8(long& val) const;

      ///
      bool getBI16(long& val) const;

      ///
      bool getBI24(long& val) const;

      ///
      bool getBI32(long& val) const;

      ///
      bool getBUI8(unsigned long& val) const;

      ///
      bool getBUI16(unsigned long& val) const;

      ///
      bool getBUI24(unsigned long& val) const;

      ///
      bool getBUI32(unsigned long& val) const;

      // bool getB(xyzzy) const;      // Not Implemented.

      ///
      bool getBFP32(float& val) const;

      ///
      bool getBFP64(double& val) const;

      /// convert the value to an integer, returning false if the underlying type doesn't support it
      /**
         The is a convenience member function to spare the programmer
         from having to go through a variety of machinations to first
         check the type, then use a variable of the correct type, use
         it, then convert it back to the target type.  That's all just
         silly.  So I do it all here for you.

      */
      bool getInt( int & val ) const;


      /// convert the value to a real, returning false if the underlying type doesn't support it
      /**
         The is a convenience member function to spare the programmer
         from having to go through a variety of machinations to first
         check the type, then use a variable of the correct type, use
         it, then convert it back to the target type.  That's all just
         silly.  So I do it all here for you.

      */
      bool getFloat( float & val ) const;



      /// convert the value to a real, returning false if the underlying type doesn't support it
      /**
         The is a convenience member function to spare the programmer
         from having to go through a variety of machinations to first
         check the type, then use a variable of the correct type, use
         it, then convert it back to the target type.  That's all just
         silly.  So I do it all here for you.

      */
      bool getDouble( double & val ) const;

      /**
       Returns the 'value' of the subfield in an object that can be 
       one of several different types.
       SlUtils defines functions for extracting specific
       C++ types from a sc_Subfield using this member function.
       (For example, see: SlUtils::getDoubleFromSubfield()) */
      sc_MultiTypeValue const& getValue() const;

      ///
      std::string const& setName(std::string const& name);
      std::string const& name(std::string const& name) { return setName(name); }

      ///
      std::string const& setMnemonic(std::string const& mnem);
      std::string const& mnemonic(std::string const& mnem) { return setMnemonic(mnem); }

      //@{
      /// set the subfield to the given value and lock it into its type
      void setA(std::string const& val);
      void setI(long val);
      void setR(double val);
      void setS(double val);
      void setC(std::string const& val);
      // void setB(xyzzy);      // NOT IMPLEMENTED.
      void setBI8(long val);
      void setBI16(long val);
      void setBI24(long val);
      void setBI32(long val);
      void setBUI8(unsigned long val);
      void setBUI16(unsigned long val);
      void setBUI24(unsigned long val);
      void setBUI32(unsigned long val);
      void setBFP32(float val);
      void setBFP64(double val);
      //@}

      /**
       This resets the subfield back to a 'uninitialized' state such that
       it still has a type, but no value.
      */
      void setUnvalued();

      /// returns true if the subfield has no value and associated type
      bool isUnvalued() const { return value_.null(); }

   private:

      /// SDTS Subfield Name
      std::string name_;
      
      /// SDTS Subfield Mnemonic
      std::string mnemonic_;

      ///
      sc_MultiTypeValue value_;

      ///
      SubfieldType type_;

      friend std::ostream& operator<<(std::ostream&, sc_Subfield const&);

}; // class sc_Subfield



#endif  // INCLUDED_SCSUBFIELD_HXX

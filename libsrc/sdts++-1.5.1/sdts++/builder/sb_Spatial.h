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
// $Id: sb_Spatial.h,v 1.10 2002/11/24 22:07:43 mcoletti Exp $

#ifndef INCLUDED_SB_SPATIAL_H
#define INCLUDED_SB_SPATIAL_H

#include <iostream>
#include <deque>


#ifndef INCLUDED_SCSUBFIELD_HXX
#include <sdts++/container/sc_Subfield.h>
#endif



/// This class represents an SDTS spatial address field
/**

   For now, we store these as three sc_Subfields.  Presumably the
   proper internal formatting has been taken care of by whatever
   entity makes use of these.

*/
class sb_Spatial
{
   public:

      sb_Spatial( sc_Subfield::SubfieldType st = sc_Subfield::is_I ) 
         : x_( "X", "" ), 
           y_( "Y", "" ), 
           default_subfield_type_( st )
      {
         x_.setUnvalued();
         y_.setUnvalued();
         z_.setUnvalued();
      }

      sb_Spatial( sc_Subfield const & x, 
                  sc_Subfield const & y ) 
         : x_( x ), y_( y ), default_subfield_type_( x.getSubfieldType() )
      {}


      sb_Spatial( sc_Subfield const & x, 
                  sc_Subfield const & y, 
                  sc_Subfield const & z ) 
         : x_( x ), y_( y ), z_( z ), 
           default_subfield_type_( x.getSubfieldType() )
      {}

      /// assigns (x,y)
      /** returns false if unable to set to new spatial value */
      bool assign( sb_Spatial const & sadr )
      {
         x() = sadr.x();
         y() = sadr.y();
         z() = sadr.z();

         default_subfield_type_ = sadr.default_subfield_type_;

         return true;
      }



      /// assigns (x,y)
      /** returns false if unable to set to new spatial value
       */
      bool assign( float x, float y, float z )
      {
         switch( default_subfield_type_ )
         {
            case sc_Subfield::is_A :
               return false;
               break;

            case sc_Subfield::is_I :
               z_.setI( z );
               break;

            case sc_Subfield::is_R :
               z_.setR( z );
               break;

            case sc_Subfield::is_S :
               z_.setS( z );
               break;

            case sc_Subfield::is_C :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_B :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BI8 :
               z_.setBI8( z );
               break;

            case sc_Subfield::is_BI16 :
               z_.setBI16( z );
               break;

            case sc_Subfield::is_BI24 :
               z_.setBI24( z );
               break;

            case sc_Subfield::is_BI32 :
               z_.setBI32( z );
               break;

            case sc_Subfield::is_BUI :
               return false;    // unsupported
               break;

            case sc_Subfield::is_BUI8 :
               z_.setBUI8( z );
               break;

            case sc_Subfield::is_BUI16 :
               z_.setBUI16( z );
               break;

            case sc_Subfield::is_BUI24 :
               z_.setBUI24( z );
               break;

            case sc_Subfield::is_BUI32 :
               z_.setBUI32( z );
               break;

            case sc_Subfield::is_BFP32 :
               z_.setBFP32( z );
               break;

            case sc_Subfield::is_BFP64 :
               z_.setBFP64( z );
               break;

         }

         return assign( x, y );

      } // assign


      /// assigns (x,y)
      /** returns false if unable to set to new spatial value
       */
      bool assign( float x, float y )
      {
         switch( default_subfield_type_ )
         {
            case sc_Subfield::is_A :
               return false;
               break;

            case sc_Subfield::is_I :
               x_.setI( x );
               y_.setI( y );
               break;

            case sc_Subfield::is_R :
               x_.setR( x );
               y_.setR( y );
               break;

            case sc_Subfield::is_S :
               x_.setS( x );
               y_.setS( y );
               break;

            case sc_Subfield::is_C :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_B :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BI8 :
               x_.setBI8( x );
               y_.setBI8( y );
               break;

            case sc_Subfield::is_BI16 :
               x_.setBI16( x );
               y_.setBI16( y );
               break;

            case sc_Subfield::is_BI24 :
               x_.setBI24( x );
               y_.setBI24( y );
               break;

            case sc_Subfield::is_BI32 :
               x_.setBI32( x );
               y_.setBI32( y );
               break;

            case sc_Subfield::is_BUI :
               return false;    // unsupported
               break;

            case sc_Subfield::is_BUI8 :
               x_.setBUI8( x );
               y_.setBUI8( y );
               break;

            case sc_Subfield::is_BUI16 :
               x_.setBUI16( x );
               y_.setBUI16( y );
               break;

            case sc_Subfield::is_BUI24 :
               x_.setBUI24( x );
               y_.setBUI24( y );
               break;

            case sc_Subfield::is_BUI32 :
               x_.setBUI32( x );
               y_.setBUI32( y );
               break;

            case sc_Subfield::is_BFP32 :
               x_.setBFP32( x );
               y_.setBFP32( y );
               break;

            case sc_Subfield::is_BFP64 :
               x_.setBFP64( x );
               y_.setBFP64( y );
               break;

         }
         return true;
      } // assign



      /// assigns (x,y,z)
      /** returns false if unable to set to new spatial value
       */
      bool assign( int x, int y, int z )
      {
         switch( default_subfield_type_ )
         {
            case sc_Subfield::is_A :
               return false;
               break;

            case sc_Subfield::is_I :
               z_.setI( z );
               break;

            case sc_Subfield::is_R :
               z_.setR( z );
               break;

            case sc_Subfield::is_S :
               z_.setS( z );
               break;

            case sc_Subfield::is_C :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_B :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BI8 :
               z_.setBI8( z );
               break;

            case sc_Subfield::is_BI16 :
               z_.setBI16( z );
               break;

            case sc_Subfield::is_BI24 :
               z_.setBI24( z );
               break;

            case sc_Subfield::is_BI32 :
               z_.setBI32( z );
               break;

            case sc_Subfield::is_BUI :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BUI8 :
               z_.setBUI8( z );
               break;

            case sc_Subfield::is_BUI16 :
               z_.setBUI16( z );
               break;

            case sc_Subfield::is_BUI24 :
               z_.setBUI24( z );
               break;

            case sc_Subfield::is_BUI32 :
               z_.setBUI32( z );
               break;

            case sc_Subfield::is_BFP32 :
               z_.setBFP32( z );
               break;

            case sc_Subfield::is_BFP64 :
               z_.setBFP64( z );
               break;

         }

         return assign( x, y );

      } // assign


      /// assigns (x,y)
      /** returns false if unable to set to new spatial value
       */
      bool assign( int x, int y )
      {
         switch( default_subfield_type_ )
         {
            case sc_Subfield::is_A :
               return false;
               break;

            case sc_Subfield::is_I :
               x_.setI( x );
               y_.setI( y );
               break;

            case sc_Subfield::is_R :
               x_.setR( x );
               y_.setR( y );
               break;

            case sc_Subfield::is_S :
               x_.setS( x );
               y_.setS( y );
               break;

            case sc_Subfield::is_C :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_B :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BI8 :
               x_.setBI8( x );
               y_.setBI8( y );
               break;

            case sc_Subfield::is_BI16 :
               x_.setBI16( x );
               y_.setBI16( y );
               break;

            case sc_Subfield::is_BI24 :
               x_.setBI24( x );
               y_.setBI24( y );
               break;

            case sc_Subfield::is_BI32 :
               x_.setBI32( x );
               y_.setBI32( y );
               break;

            case sc_Subfield::is_BUI :
               return false;    // unsupported type
               break;

            case sc_Subfield::is_BUI8 :
               x_.setBUI8( x );
               y_.setBUI8( y );
               break;

            case sc_Subfield::is_BUI16 :
               x_.setBUI16( x );
               y_.setBUI16( y );
               break;

            case sc_Subfield::is_BUI24 :
               x_.setBUI24( x );
               y_.setBUI24( y );
               break;

            case sc_Subfield::is_BUI32 :
               x_.setBUI32( x );
               y_.setBUI32( y );
               break;

            case sc_Subfield::is_BFP32 :
               x_.setBFP32( x );
               y_.setBFP32( y );
               break;

            case sc_Subfield::is_BFP64 :
               x_.setBFP64( x );
               y_.setBFP64( y );
               break;

         }
         return true;
      } // assign



      ///
      sc_Subfield const & x() const { return x_; }

      ///
      sc_Subfield const & y() const { return y_; }

      ///
      sc_Subfield const & z() const { return z_; }
      
      ///
      sc_Subfield& x()       { return x_; }

      ///
      sc_Subfield& y()       { return y_; }

      ///
      sc_Subfield& z()       { return z_; }

      /// Get the default SDTS subfield type to be used when using assign()
      sc_Subfield::SubfieldType const & defaultSubfieldType() const { return default_subfield_type_; }

      /// Set the default SDTS subfield type to be used when using assign()
      sc_Subfield::SubfieldType & defaultSubfieldType()  { return default_subfield_type_; }

   private:

      ///
      sc_Subfield x_;

      ///
      sc_Subfield y_;

      ///
      sc_Subfield z_;

      /// default spatial address representation
      /**
         This is used to determine the subfield type to use when
         setting via an nteger value.

         \todo XXX The problem with this, of course, is that it presumes that both the X, Y, and Z coordinates will use the same representation; which, to be fair, is probably the case for (X,Y), and "Z" is never used.
      */
      sc_Subfield::SubfieldType default_subfield_type_;
      
}; // class sb_Spatial


/// convenient container for spatial addresses (SADRs)
typedef std::deque<sb_Spatial> sb_Spatials;


inline
std::ostream & 
operator<<( std::ostream & os, sb_Spatial const & spatial )
{
   os << "("
      << spatial.x() << ","
      << spatial.y();

   if ( ! spatial.x().isUnvalued() )
   {
      os << "," << spatial.z() << ")\n";
   }
   else
   {
      os << ")\n";
   }

   return os;
} // operator<<

#endif

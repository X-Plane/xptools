//
// $Id: sb_ForeignID_t.cpp,v 1.4 2000/08/08 21:17:35 mcoletti Exp $
//

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include <sdts++/builder/sb_ForeignID.h>



void
testUsageModifier( sb_ForeignID & frid, sb_ForeignID::usage_t ut )
{
   sb_ForeignID tmp_frid( frid );

   // save the original packed identifier string so that we can
   // compare it with ones that have the usage modifier character
   // added to it
   std::string packed_id;

   frid.packedIdentifierString( packed_id );

   // now set the usage modifier
   tmp_frid.usageModifier( ut );


   std::string tmp_packed_id;

   assert( tmp_frid.packedIdentifierString( tmp_packed_id ) );

   switch ( ut )
   {
      case sb_ForeignID::none :
         cerr << "invalid usage modifier\n";
         exit( 1 );
         break;

      case sb_ForeignID::start_node :
         assert( sb_ForeignID::start_node == tmp_frid.usageModifier() );
         assert( packed_id + 'S' == tmp_packed_id );
         break;

      case sb_ForeignID::end_node :
         assert( sb_ForeignID::end_node == tmp_frid.usageModifier() );
         assert( packed_id + 'E' == tmp_packed_id );
         break;

      case sb_ForeignID::left_polygon :
         assert( sb_ForeignID::left_polygon == tmp_frid.usageModifier() );
         assert( packed_id + 'L' == tmp_packed_id );
         break;

      case sb_ForeignID::right_polygon :
         assert( sb_ForeignID::right_polygon == tmp_frid.usageModifier() );
         assert( packed_id + 'R' == tmp_packed_id );
         break;

      case sb_ForeignID::forward_orientation :
         assert( sb_ForeignID::forward_orientation == tmp_frid.usageModifier() );
         assert( packed_id + 'F' == tmp_packed_id );
         break;

      case sb_ForeignID::backward_orientation :
         assert( sb_ForeignID::backward_orientation == tmp_frid.usageModifier() );
         assert( packed_id + 'B' == tmp_packed_id );
         break;

      case sb_ForeignID::interior_polygon :
         assert( sb_ForeignID::interior_polygon == tmp_frid.usageModifier() );
         assert( packed_id + 'I' == tmp_packed_id );
         break;

      case sb_ForeignID::exterior_polygon :
         assert( sb_ForeignID::exterior_polygon == tmp_frid.usageModifier() );
         assert( packed_id + 'X' == tmp_packed_id );
         break;

      default :
         cerr << "invalid usage modifier\n";
         exit( 1 );
         break;
   }

} // testUsageModifier




int
main( int argc, char** argv )
{
   string module_name( "LE01" );
   int    record_number( 99 );
   sb_ForeignID::usage_t usage( sb_ForeignID::none );


   // check ctor; set some values and insure accessors return them again
   sb_ForeignID frid( module_name, record_number, usage );

   assert( module_name == frid.moduleName() );
   assert( record_number == frid.recordID() );
   assert( sb_ForeignID::none == frid.usageModifier() );


   // now insure that the generated packed foreign id string is correct

   std::string packed_id( "LE01#99" );
   std::string tmp_packed_id;

   assert( frid.packedIdentifierString( tmp_packed_id ) );

   assert( packed_id == tmp_packed_id );

   // now insure that the copy ctor works; similar testing for regular ctor
   {
      sb_ForeignID tmp_frid( frid );

      assert( module_name == tmp_frid.moduleName() );
      assert( record_number == tmp_frid.recordID() );
      assert( usage == tmp_frid.usageModifier() );


      // this includes checking the generated packed ID again
      std::string tmp_packed_id;

      assert( tmp_frid.packedIdentifierString( tmp_packed_id ) );

      assert( packed_id == tmp_packed_id );

   }

   // now test the hey out of the usage modifier stuff
   testUsageModifier( frid, sb_ForeignID::start_node );
   testUsageModifier( frid, sb_ForeignID::end_node );
   testUsageModifier( frid, sb_ForeignID::left_polygon );
   testUsageModifier( frid, sb_ForeignID::right_polygon );
   testUsageModifier( frid, sb_ForeignID::forward_orientation );
   testUsageModifier( frid, sb_ForeignID::backward_orientation );
   testUsageModifier( frid, sb_ForeignID::interior_polygon );
   testUsageModifier( frid, sb_ForeignID::exterior_polygon );

   exit( 0 );
}


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
// sio_8211DRLeader.cpp
//

#include <sdts++/io/sio_8211DRLeader.h>

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
using namespace std;
#endif

#ifndef INCLUDED_SIO_UTILS_H
#include <sdts++/io/sio_Utils.h>
#endif


using namespace std;


sio_8211DRLeader::sio_8211DRLeader()
{
   setLeaderIdentifier('D');
}



istream&
sio_8211DRLeader::streamExtract(istream& istr)
{

   // Assume istr is positioned on byte 0 of a DR Leader. Read the leader,
   // leaving istr positioned on the byte after the last byte in the leader.

   char leader[24];

   istr.read(leader,24);
   if ((istr.gcount() < 24) || (!istr))
      {
         istr.clear(ios::failbit);
         return istr;
      }
      
   recLength_        = sio_Utils::getLong(leader,0,5);
   leaderIden_       = leader[6];
   fieldAreaStart_   = sio_Utils::getLong(leader,12,5);
   sizeFieldLength_  = sio_Utils::getLong(leader,20,1);
   sizeFieldPos_     = sio_Utils::getLong(leader,21,1);
   sizeFieldTag_     = sio_Utils::getLong(leader,23,1);

                                // if we didn't read a valid leader,
                                // then wedge the stream to indicate
                                // this error
   if ( ! isValid() )
     {
       istr.setstate( ios::badbit );
     }

   return istr;

} // sio_8211DRLeader:;streamExtract()



ostream&
sio_8211DRLeader::streamInsert(ostream& ostr) const
{
   // This code assumes a NEW stream pointing to an existing, empty file.
   // Write a DDR header to the file

   ostr << setw(5) << recLength_;       // Record Length
   ostr << setw(1) << ' ';              // Interchange Level
   ostr << setw(1) << leaderIden_;      // Leader Identifier
   ostr << setw(1) << ' ';              // Inline Code Extension Indicator
   ostr << setw(1) << '1';              // Version Number
   ostr << setw(1) << ' ';              // Application Indicator
   ostr << setw(2) << "  ";             // Field Control Length
   ostr << setw(5) << fieldAreaStart_;  // Base Address of Field Area
   ostr << setw(3) << "   ";            // Extended Character Set Indicator
   ostr << setw(1) << sizeFieldLength_; // Size of Field Length Field
   ostr << setw(1) << sizeFieldPos_;    // Size of Field Position Field
   ostr << setw(1) << '0';              // Reserved
   ostr << setw(1) << sizeFieldTag_;    // Size if Field Tag Field

   return ostr; 

}

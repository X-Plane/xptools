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
// sio_8211DDRLeader.cpp
//

#ifdef NEED_TYPEINFO
#include <typeinfo>
#endif


#include <sdts++/io/sio_8211DDRLeader.h>


#ifndef INCLUDED_SIO_UTILS_H
#include <sdts++/io/sio_Utils.h>
#endif


using namespace std;



// returns true if the given leader has valid values
bool
sio_8211DDRLeader::isValid( ) const
{

  // first do the plain vanilla validity common to all 8211 leaders

  if ( ! sio_8211Leader::isValid() ) return false;


  // now let's do the DDR specific validity checks

                                // interchange level

  if ( ! ( '1' == interchangeLevel_ ||
           '2' == interchangeLevel_ ||
           '3' == interchangeLevel_ ) ) 
    return false;

                                // inline code extension indicator

  if ( ! ( ' ' == inlineCodeExtensionIndicator_ || 
           'E' == inlineCodeExtensionIndicator_ || 
           'h' == inlineCodeExtensionIndicator_ || 
           'H' == inlineCodeExtensionIndicator_ ) )
    return false;

                                // version number

  if ( ! ( ' ' == versionNumber_ || 
           '1' == versionNumber_ ) )
    return false;
                                // app indicator

  // since any character is effectively valid, we can make no validity
  // checking

                                // field control length (c.f., 6.1.4
                                // of ISO 8211 spec)

  switch ( fieldControlLength_ )
    {
    case 0 :
    case 3 :
      if ( 1 == interchangeLevel_ ) 
        return true;
      else
        return false;

      break;


    case 6 :
    case 9 :
      if ( '2' == interchangeLevel_ || '3' == interchangeLevel_ ) 
        return true;
      else
        return false;

      break;

    default :
      return false;             // *bzzz*  You lose!
    }

  return true;

} // isValid






sio_8211DDRLeader::sio_8211DDRLeader()
   :  interchangeLevel_('2'),                // always level 2 8211 files
      inlineCodeExtensionIndicator_(' '),    // no special characters used in modules
      versionNumber_('1'),                   // we're playing with ISO 8211, second edition
      appIndicator_(' '),                    // not going to refer to other standards in DDR
      fieldControlLength_(6)
{
   setLeaderIdentifier('L');
}

sio_8211DDRLeader::~sio_8211DDRLeader()
{
}


long
sio_8211DDRLeader::getFieldControlLength() const
{
   return fieldControlLength_;
} // sio_8211DDRLeader::getFieldControlLength

istream&
sio_8211DDRLeader::streamExtract(istream& istr)
{

   // Assume istr is positioned on byte 0 of a DDR Leader. Read the leader,
   // leaving istr positioned on the byte after the last byte in the leader.

   char leader[24];

   istr.read(leader,24);
   if ((istr.gcount() < 24) || (!istr))
      {
         istr.clear(ios::failbit);
         return istr;
      }
      
   recLength_                    = sio_Utils::getLong(leader,0,5);
   interchangeLevel_             = leader[5];
   leaderIden_                   = leader[6];
   inlineCodeExtensionIndicator_ = leader[7];
   versionNumber_                = leader[8];
   appIndicator_                 = leader[9];
   fieldControlLength_           = sio_Utils::getLong(leader,10,2);
   fieldAreaStart_               = sio_Utils::getLong(leader,12,5);
   sizeFieldLength_              = sio_Utils::getLong(leader,20,1);
   sizeFieldPos_                 = sio_Utils::getLong(leader,21,1);
   sizeFieldTag_                 = sio_Utils::getLong(leader,23,1);


                                // if we didn't read a valid leader,
                                // then wedge the stream to indicate
                                // this error
   if ( ! isValid( ) )
     {
       istr.setstate( ios::badbit );
     }

   return istr;
} // sio_8211DDRLeader::streamExtract()



ostream&
sio_8211DDRLeader::streamInsert(ostream& ostr) const
{
   // This code assumes a NEW stream pointing to an existing, empty file.
   // Write a DDR header to the file

   ostr << setw(5) << recLength_;          // Record Length
   ostr << setw(1) << interchangeLevel_;   // Interchange Level
   ostr << setw(1) << leaderIden_;         // Leader Identifier
   ostr << setw(1) << inlineCodeExtensionIndicator_; // Inline Code Extension Indicator
   ostr << setw(1) << versionNumber_;      // Version Number
   ostr << setw(1) << appIndicator_;       // Application Indicator
   ostr << setw(2) << fieldControlLength_; // Field Control Length
   ostr << setw(5) << fieldAreaStart_; // Base Address of Field Area
   ostr << setw(3) << "   ";               // Extended Character Set Indicator
   ostr << setw(1) << sizeFieldLength_;    // Size of Field Length Field
   ostr << setw(1) << sizeFieldPos_;       // Size of Field Position Field
   ostr << setw(1) << '0';                 // Reserved
   ostr << setw(1) << sizeFieldTag_;       // Size if Field Tag Field

   return ostr; 

} // sio_8211DDRLeader::streamInsert()
                

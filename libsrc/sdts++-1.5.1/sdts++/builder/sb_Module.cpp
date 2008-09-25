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

#include "sb_Module.h"

#include <iostream>

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif


#ifndef INCLUDED_SB_FOREIGNID_H
#include "sb_ForeignID.h"
#endif


using namespace std;


static const char* ident_ = "$Id: sb_Module.cpp,v 1.8 2002/11/24 22:07:42 mcoletti Exp $";



bool
sb_Module::getSchema( sio_8211Schema& schema )
{
                                // if the schema hasn't been built
                                // yet, then go ahead and create it
   if ( schema_().empty() )
   {
      buildSchema_();
   }
                                // if it's still__ empty, then
                                // something went very wrong
   if ( schema_().empty() )
   {
      return false;
   }

   schema = schema_();

   return true;

} // sb_Module::getSchema



void
sb_Module::buildSchema_()
{

   schema_().clear();           // make sure we are starting with clean schema

   // Add the field format for the 8211 record identifier reserved
   // field if the user has specified that modules should use those
   // fields.  sio_Writer objects will automatically write out 8211
   // data records with these fields if it detects the presence of
   // this field format in the current schema it uses.

   if ( willEmitRecIdenField() )
   {
      schema_().push_front( sio_8211FieldFormat() );
      schema_().front().setDataStructCode(
         sio_8211FieldFormat::elementary );
      schema_().front().setDataTypeCode(
         sio_8211FieldFormat::implicit_point );
      schema_().front().setName( "DDF RECORD IDENTIFER" );
      schema_().front().setTag( "0001" );
   }


   // now call the child's module so that it can set up its specific
   // field formats

   buildSpecificSchema_();

} // sb_Module::buildSchema_




ostream&
operator<<( ostream& os, sb_Module const& module )
{
  sc_Record record;

  if ( module.getRecord( record ) )
    {
      os << record << "\n";
    }
  else
    {
      os << "unable to dump builder object\n";
    }

  return os;

}



sb_ForeignID
sb_Module::foreignID() const
{
   return sb_ForeignID( getMnemonic(), getID() );
} // sb_Module::foreignID() const



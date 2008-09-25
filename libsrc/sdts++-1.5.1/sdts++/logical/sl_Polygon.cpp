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
// $Id: sl_Polygon.cpp,v 1.1 2000/05/02 00:55:34 mcoletti Exp $

#include "sl_Polygon.h"


static const char* ident_ =
  "$Id: sl_Polygon.cpp,v 1.1 2000/05/02 00:55:34 mcoletti Exp $";



//
// sl_GPolygon
//

static std::string const g_polygon_object_code = "PG";


std::string const &
sl_GPolygon::objectCode() const
{
   return g_polygon_object_code;
} // sl_GPolygon::objectCode()




//
// sl_GTRingPolygon
//

static std::string const gtring_polygon_object_code = "PR";


std::string const &
sl_GTRingPolygon::objectCode() const
{
   return gtring_polygon_object_code;
} // sl_GTRingPolygon::objectCode()





//
// sl_GTChainPolygon
//

static std::string const gtchain_polygon_object_code = "PC";


std::string const &
sl_GTChainPolygon::objectCode() const
{
   return gtchain_polygon_object_code;
} // sl_GTChainPolygon::objectCode()






//
// sl_UniverseChainPolygon
//

static std::string const unichain_polygon_object_code = "PW";


std::string const &
sl_UniverseChainPolygon::objectCode() const
{
   return unichain_polygon_object_code;
} // sl_UniverseChainPolygon::objectCode()




//
// sl_UniverseRingPolygon
//

static std::string const uniring_polygon_object_code = "PU";


std::string const &
sl_UniverseRingPolygon::objectCode() const
{
   return uniring_polygon_object_code;
} // sl_UniverseRingPolygon::objectCode()






//
// sl_VoidChainPolygon
//

static std::string const voidchain_polygon_object_code = "PX";


std::string const &
sl_VoidChainPolygon::objectCode() const
{
   return voidchain_polygon_object_code;
} // sl_VoidChainPolygon::objectCode()




//
// sl_VoidRingPolygon
//

static std::string const voidring_polygon_object_code = "PV";


std::string const &
sl_VoidRingPolygon::objectCode() const
{
   return voidring_polygon_object_code;
} // sl_VoidRingPolygon::objectCode()

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
// $Id: sl_Point.cpp,v 1.2 2000/04/28 18:11:07 mcoletti Exp $

#include "sl_Point.h"


static const char* ident_ = 
  "$Id: sl_Point.cpp,v 1.2 2000/04/28 18:11:07 mcoletti Exp $";


//
// sl_Point
//

static std::string const point_object_code = "NP";



std::string const &
sl_Point::objectCode() const
{
   return point_object_code;
} // sl_Point::objectCode()



//
// sl_AreaPoint
//

static std::string const area_point_object_code = "NA";


std::string const &
sl_AreaPoint::objectCode() const
{
   return area_point_object_code;
} // sl_AreaPoint::objectCode()



//
// sl_LabelPoint
//

static std::string const label_point_object_code = "NL";


std::string const &
sl_LabelPoint::objectCode() const
{
   return label_point_object_code;
} // sl_LabelPoint::objectCode()



//
// sl_EntityPoint
//

static std::string const entity_point_object_code = "NE";


std::string const &
sl_EntityPoint::objectCode() const
{
   return entity_point_object_code;
} // sl_EntityPoint::objectCode()

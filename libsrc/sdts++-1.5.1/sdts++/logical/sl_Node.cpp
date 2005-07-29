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
// $Id: sl_Node.cpp,v 1.1 2000/04/28 18:25:50 mcoletti Exp $

#include "sl_Node.h"


static const char* ident_ = 
  "$Id: sl_Node.cpp,v 1.1 2000/04/28 18:25:50 mcoletti Exp $";



//
// sl_PlanarNode
//

static std::string const planar_node_object_code = "NO";


std::string const &
sl_PlanarNode::objectCode() const
{
   return planar_node_object_code;
} // sl_PlanarNode::objectCode()



//
// sl_NetworkNode
//

static std::string const network_node_object_code = "NN";


std::string const &
sl_NetworkNode::objectCode() const
{
   return network_node_object_code;
} // sl_NetworkNode::objectCode()



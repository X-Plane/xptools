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
// $Id: sl_Chain.cpp,v 1.1 2000/04/28 21:48:54 mcoletti Exp $

#include "sl_Chain.h"


static const char* ident_ = 
  "$Id: sl_Chain.cpp,v 1.1 2000/04/28 21:48:54 mcoletti Exp $";



//
// sl_CompleteChain
//

static std::string const complete_chain_object_code = "LE";


std::string const &
sl_CompleteChain::objectCode() const
{
   return complete_chain_object_code;
} // sl_CompleteChain::objectCode()




//
// sl_AreaChain
//

static std::string const area_chain_object_code = "LL";


std::string const &
sl_AreaChain::objectCode() const
{
   return area_chain_object_code;
} // sl_AreaChain::objectCode()



//
// sl_NonPlanarNetworkChain
//

static std::string const nonplanarnetwork_chain_object_code = "LY";


std::string const &
sl_NonPlanarNetworkChain::objectCode() const
{
   return nonplanarnetwork_chain_object_code;
} // sl_NonPlanarNetworkChain::objectCode()



//
// sl_PlanarNetworkChain
//

static std::string const planarnetwork_chain_object_code = "LW";


std::string const &
sl_PlanarNetworkChain::objectCode() const
{
   return planarnetwork_chain_object_code;
} // sl_PlanarNetworkChain::objectCode()

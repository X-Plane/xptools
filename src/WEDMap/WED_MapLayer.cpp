#include "WED_MapLayer.h"

#include "WED_MapZoomerNew.h"

WED_MapLayer::WED_MapLayer(WED_MapZoomerNew * z, IResolver * i) : mZoomer(z), mResolver(i)
{
}

WED_MapLayer::~WED_MapLayer()
{
}

#include "WED_MapLayer.h"

#include "WED_MapZoomerNew.h"

WED_MapLayer::WED_MapLayer(GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) : mZoomer(z), mResolver(i), mHost(h)
{
}

WED_MapLayer::~WED_MapLayer()
{
}

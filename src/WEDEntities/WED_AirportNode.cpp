#include "WED_AirportNode.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportNode)

WED_AirportNode::WED_AirportNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	attrs(this,"Attributes","WED_airportnode", "attributes", LinearFeature)
{
}

WED_AirportNode::~WED_AirportNode()
{
}

void	WED_AirportNode::SetAttributes(const set<int>& in_attrs)
{
	attrs = in_attrs;
}

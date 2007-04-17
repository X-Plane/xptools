#include "WED_AirportNode.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportNode)
START_CASTING(WED_AirportNode)
INHERITS_FROM(WED_GISPoint_Bezier)
END_CASTING

WED_AirportNode::WED_AirportNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	attrs(this,"Attributes","WED_airportnode", "attributes", LinearFeature)
{
}

WED_AirportNode::~WED_AirportNode()
{
}

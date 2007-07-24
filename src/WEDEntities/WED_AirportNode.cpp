#include "WED_AirportNode.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportNode)
TRIVIAL_COPY(WED_AirportNode, WED_GISPoint_Bezier)

WED_AirportNode::WED_AirportNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	attrs(this,".Attributes","WED_airportnode", "attributes", LinearFeature),
	lines(this,"Line Attributes","","",".Attributes",line_SolidYellow,line_BWideBrokenDouble),
	lights(this,"Light Attributes","","",".Attributes",line_TaxiCenter,line_BoundaryEdge)
{
}

WED_AirportNode::~WED_AirportNode()
{
}

void	WED_AirportNode::SetAttributes(const set<int>& in_attrs)
{
	attrs = in_attrs;
}

void		WED_AirportNode::GetAttributes(set<int>& out_attrs) const
{
	out_attrs = attrs.value;
}


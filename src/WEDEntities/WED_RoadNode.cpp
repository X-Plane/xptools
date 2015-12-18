//
//  WED_RoadNode.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/15/15.
//
//

#include "WED_RoadNode.h"


DEFINE_PERSISTENT(WED_RoadNode)
TRIVIAL_COPY(WED_RoadNode,WED_GISPoint)

WED_RoadNode::WED_RoadNode(WED_Archive * a, int i) : WED_GISPoint(a,i)
{
}

WED_RoadNode::~WED_RoadNode()
{
}

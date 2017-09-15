/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef WED_GroupCommands_H
#define WED_GroupCommands_H

#include <map>

class	IGISEdge;
class	IResolver;

class	Point2;

class	WED_GISEdge;
class	WED_MapZoomerNew;
class	WED_Thing;

int		WED_CanGroup(IResolver * inResolver);
int		WED_CanUngroup(IResolver * inResolver);
void	WED_DoGroup(IResolver * inResolver);
void	WED_DoUngroup(IResolver * inResolver);

void	WED_DoMakeNewOverlay(IResolver * inResolver, WED_MapZoomerNew * zoomer);

int		WED_CanMakeNewAirport(IResolver * inResolver);
void	WED_DoMakeNewAirport(IResolver * inResolver);

int		WED_CanMakeNewATCFreq(IResolver * inResolver);
void	WED_DoMakeNewATCFreq(IResolver * inResolver);
#if AIRPORT_ROUTING
int		WED_CanMakeNewATCFlow(IResolver * inResolver);
void	WED_DoMakeNewATCFlow(IResolver * inResolver);
int		WED_CanMakeNewATCRunwayUse(IResolver * inResolver);
void	WED_DoMakeNewATCRunwayUse(IResolver * inResolver);
int		WED_CanMakeNewATCWindRule(IResolver * inResolver);
void	WED_DoMakeNewATCWindRule(IResolver * inResolver);
int		WED_CanMakeNewATCTimeRule(IResolver * inResolver);
void	WED_DoMakeNewATCTimeRule(IResolver * inResolver);
#endif

int		WED_CanSetCurrentAirport(IResolver * inResolver, string& io_cmd_name);
void	WED_DoSetCurrentAirport(IResolver * inResolver);

bool	WED_CanAddMetaData(IResolver * inResolver, int command);
void	WED_DoAddMetaData(IResolver * inResolver, int command);

int		WED_CanReorder(IResolver * resolver, int direction, int to_end);
void	WED_DoReorder (IResolver * resolver, int direction, int to_end);

int		WED_CanClear(IResolver * resolver);
void	WED_DoClear(IResolver * resolver);
int		WED_CanCrop(IResolver * resolver);
void	WED_DoCrop(IResolver * resolver);

int		WED_CanMerge(IResolver * resolver);
void	WED_DoMerge(IResolver * resolver);

int		WED_CanSplit(IResolver * resolver);

struct split_edge_info_t {
	WED_GISEdge *			edge;
	bool					active;						// Of all the edges we can split, only some are ACTIVE.  To make a split, at least ONE must be active.
	vector<Point2>			splits;						// This lets us say "the new edges are cutting everything, but legacy edges do not cut each other."
	split_edge_info_t(WED_GISEdge* edge, bool active);
	void sort_along_edge();
};

//Where the key is the edge and the value is the edges that spawned off because of it
typedef map<WED_Thing*, vector<WED_Thing*> > edge_to_child_edges_map_t;

//Given a vector of splittable objects and splittable edges, preform the actual math
//It returns a vector of the new pieces
edge_to_child_edges_map_t run_split_on_edges(vector<split_edge_info_t>& edges);
void	WED_DoSplit(IResolver * resolver);
int		WED_CanAlign(IResolver * resolver);
void	WED_DoAlign(IResolver * resolver);
int		WED_CanOrthogonalize(IResolver * resolver);
void	WED_DoOrthogonalize(IResolver * resolver);
int		WED_CanMakeRegularPoly(IResolver * resolver);
void	WED_DoMakeRegularPoly(IResolver * resolver);
int		WED_CanReverse(IResolver * resolver);
void	WED_DoReverse(IResolver * resolver);
int		WED_CanRotate(IResolver * resolver);
void	WED_DoRotate(IResolver * resolver);
int		WED_CanDuplicate(IResolver * resolver);
void	WED_DoDuplicate(IResolver * resolver, bool wrap_in_cmd);


int		WED_CanSelectAll(IResolver * resolver);
void	WED_DoSelectAll(IResolver * resolver);
int		WED_CanSelectNone(IResolver * resolver);
void	WED_DoSelectNone(IResolver * resolver);
int		WED_CanSelectParent(IResolver * resolver);
void	WED_DoSelectParent(IResolver * resolver);
int		WED_CanSelectChildren(IResolver * resolver);
void	WED_DoSelectChildren(IResolver * resolver);
int		WED_CanSelectVertices(IResolver * resolver);
void	WED_DoSelectVertices(IResolver * resolver);
int		WED_CanSelectPolygon(IResolver * resolver);
void	WED_DoSelectPolygon(IResolver * resolver);
int		WED_CanSelectConnected(IResolver * resolver);
void	WED_DoSelectConnected(IResolver * resolver);

bool	WED_DoSelectZeroLength(IResolver * resolver, WED_Thing * sub_tree=NULL);			// These return true if they did an operation to change selection due to there being work to do.
bool	WED_DoSelectDoubles(IResolver * resolver, WED_Thing * sub_tree=NULL);				// They do not show any UI but they do select the failures.

set<WED_GISEdge*> do_select_crossing(vector<WED_GISEdge* > edges);
bool	WED_DoSelectCrossing(IResolver * resolver, WED_Thing * sub_tree=NULL);

void	WED_DoSelectMissingObjects(IResolver * resolver);
void	WED_DoSelectLocalObjects(IResolver * resolver);
void	WED_DoSelectLibraryObjects(IResolver * resolver);
void	WED_DoSelectDefaultObjects(IResolver * resolver);
void	WED_DoSelectThirdPartyObjects(IResolver * resolver);


// This isn't really a command...rather, it's used by drag & drop code.  But...trying to keep all of the grouping logic in one place.
int		WED_CanMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot);
void	WED_DoMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot);

int		WED_Repair(IResolver * resolver);

int		WED_CanBreakApartSpecialAgps(IResolver* resolver);
void	WED_DoBreakApartSpecialAgps(IResolver* resolver);

int		WED_CanReplaceVehicleObj(IResolver* resolver);
void	WED_DoReplaceVehicleObj(IResolver* resolver);

void WED_UpgradeRampStarts(IResolver * resolver);
void WED_RenameRunwayNames(IResolver * resolver);

#endif /* WED_GroupCommands_H */

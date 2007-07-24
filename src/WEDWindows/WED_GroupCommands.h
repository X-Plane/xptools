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

class	IResolver;
class	WED_Thing;
class	WED_MapZoomerNew;

int		WED_CanGroup(IResolver * inResolver);
int		WED_CanUngroup(IResolver * inResolver);
void	WED_DoGroup(IResolver * inResolver);
void	WED_DoUngroup(IResolver * inResolver);

void	WED_DoMakeNewOverlay(IResolver * inResolver, WED_MapZoomerNew * zoomer);

int		WED_CanMakeNewAirport(IResolver * inResolver);
void	WED_DoMakeNewAirport(IResolver * inResolver);

int		WED_CanMakeNewATC(IResolver * inResolver);
void	WED_DoMakeNewATC(IResolver * inResolver);

int		WED_CanSetCurrentAirport(IResolver * inResolver, string& io_cmd_name);
void	WED_DoSetCurrentAirport(IResolver * inResolver);

int		WED_CanReorder(IResolver * resolver, int direction, int to_end);
void	WED_DoReorder (IResolver * resolver, int direction, int to_end);

int		WED_CanClear(IResolver * resolver);
void	WED_DoClear(IResolver * resolver);
int		WED_CanCrop(IResolver * resolver);
void	WED_DoCrop(IResolver * resolver);

int		WED_CanSplit(IResolver * resolver);
void	WED_DoSplit(IResolver * resolver);
int		WED_CanReverse(IResolver * resolver);
void	WED_DoReverse(IResolver * resolver);
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

// This isn't really a command...rather, it's used by drag & drop code.  But...trying to keep all of the grouping logic in one place.
int		WED_CanMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot);
void	WED_DoMoveSelectionTo(IResolver * resolver, WED_Thing * dest, int dest_slot);

#endif /* WED_GroupCommands_H */

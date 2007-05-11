#ifndef WED_TOOLUTILS_H
#define WED_TOOLUTILS_H

class	WED_Airport;
class	WED_Thing;
class	ISelection;
class	IResolver;

WED_Thing *		WED_FindParent(ISelection * isel,	// Selected objects
					WED_Thing * require_this,		// Our common container will be at or below this in the hiearchy
					WED_Thing * backup_choice);		// If selection is empty, we'll use this.

void			WED_GetSelectionInOrder(IResolver * resolver, vector<WED_Thing *>& out_sel);

WED_Airport *	WED_GetCurrentAirport(IResolver * resolver);
void			WED_SetCurrentAirport(IResolver * resolver, WED_Airport * airport);		// Does NOT create a command!!!!!!

ISelection *	WED_GetSelect(IResolver * resolver);
WED_Thing	*	WED_GetWorld(IResolver * resolver);

#endif /* WED_TOOLUTILS_H */

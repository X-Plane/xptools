#ifndef WED_TOOLUTILS_H
#define WED_TOOLUTILS_H

class	WED_Thing;
class	ISelection;
class	IResolver;

WED_Thing *	WED_FindParent(ISelection * isel,	// Selected objects
				WED_Thing * require_this,		// Our common container will be at or below this in the hiearchy
				WED_Thing * backup_choice);		// If selection is empty, we'll use this.


WED_Thing *		WED_GetCurrentAirport(IResolver * resolver);
ISelection *	WED_GetSelect(IResolver * resolver);

#endif /* WED_TOOLUTILS_H */
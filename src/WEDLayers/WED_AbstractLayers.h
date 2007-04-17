IS THIS USED?  NO!

#ifndef WED_ABSTRACTLAYERS_H
#define WED_ABSTRACTLAYERS_H

#include "GUI_Broadcaster.h"

class	GUI_GraphState;
class	WED_MapZoomer;
/*
	WED_AbstractLayers - THEORY OF OPERATOION
	
	The abstract layer class acts as a data source to the UI.  It is a behavioral
	class in that it knows how to interpret data-model structures and express them
	as UI elements.
	
	This class provides a single-point of entrance to the hierarchy..in other words
	when we want to get a nested layer in a heirarchy, we simply ask for the 54th 
	layer and the particular layers recursively call each other to get the 
	results.
	
	Why do it like this?  Why not have a tree the client follows?  The answer is:
	because the layers model the data model, we do NOT want to have to create and
	destroy objects every time the data model changes.  Since the data model has
	persistent multiple undo, tracking that would be complex and error-prone.  

	Instead each behavior can simply read the data model on the fly and translate.
	The idea is that the data model is fast and can translate things fast.
	
	Typically the containers will have permanent internal ptrs to other containers
	and the groups will simply query the data model.
*/

// Layer Capability Flags
enum {
	wed_Layer_Hide = 1,
	wed_Layer_Edit = 2,
	wed_Layer_Export = 4,
	wed_Layer_Opacity = 8,
	wed_Layer_Rename = 16,
	wed_Layer_Reorder = 32,
	wed_Layer_Children = 64,

// Tools
	wed_Tool_Selection = 0,
	
// Flags
	wed_Flag_Visible = 1,
	wed_Flag_Export = 2,
	wed_Flag_Children = 4
	
};


class	WED_AbstractLayers : public GUI_Broadcaster {
public:

	// HIERARCHY
	
	virtual	int					CountLayers(void)=0;
	virtual	int					GetIndent(int n)=0;

	// BASIC LAYER PROPERTIES:
	
	virtual	int					GetLayerCapabilities(int n)=0;
	virtual	int					GetLayerAllowedTools(int n)=0;
	
	// LAYER ACCESSORS
	
	virtual int					GetFlags(int n) const=0;
	virtual	string				GetName(int n) const=0;
	virtual float				GetOpacity(int n) const=0;
	
	virtual	void				ToggleFlag(int n, int flag)=0;
	virtual void				Rename(int n, const string& name)=0;
	virtual	void				SetOpacity(int n, float opacity)=0;
	
	// VISUALIZATION
	virtual	void				DrawLayer(
									int						n,
									GUI_GraphState *		state,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						overlay)=0;
	virtual	int					HandleMouseDown(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button)=0;
	virtual	void				HandleMouseDrag(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button)=0;
	virtual	void				HandleMouseUp(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button)=0;

};

#endif /* WED_ABSTRACTLAYER_H */

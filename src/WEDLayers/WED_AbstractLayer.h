#ifndef WED_ABSTRACTLAYER_H
#define WED_ABSTRACTLAYER_H

#include "XPLMDisplay.h"

class	WED_MapZoomer;

// Layer Capability Flags
enum {
	layer_Hide = 1,
	layer_Edit = 2,
	layer_Export = 4,
	layer_Opacity = 8,
	layer_Rename = 16,
	layer_Reorder = 32
};

// Tools
enum {
	tool_Selection = 0
};


class	WED_AbstractLayer {
public:

								WED_AbstractLayer(WED_MapZoomer * inZoomer);
	virtual						~WED_AbstractLayer();

	// BASIC LAYER PROPERTIES:
	
	virtual	int					GetLayerCapabilities(void)=0;
	virtual	int					GetLayerAllowedTools(void)=0;
	virtual string				GetLayerClass(void)=0;
	
	// LAYER ACCESSORS
	
	virtual int					GetFlags(int) const=0;
	virtual	string				GetName(void) const=0;
	virtual float				GetOpacity(void) const=0;
	
	virtual	void				ToggleFlag(int)=0;
	virtual void				Rename(const string&)=0;
	virtual	void				SetOpacity(float)=0;
	
	// NESTING
	
	virtual int					CountChildren(void) const=0;
	virtual WED_AbstractLayer *	GetNthChild(int) const=0;
	
	virtual bool				CanAcceptChild(WED_AbstractLayer * who, int pos) const=0;
	virtual void				AcceptLayer(WED_AbstractLayer * who, int pos)=0;
	
	// MOUSE API

	virtual	void				DrawFeedbackUnderlay(
									int					inTool)=0;
	virtual	void				DrawFeedbackOverlay(
									int					inTool)=0;
	virtual	bool				HandleClick(
									int					inTool,
									XPLMMouseStatus		inStatus,
									int 				inX, 
									int 				inY, 
									int 				inButton)=0;
							
	// Support for some properties that can be edited.	
	virtual int		GetNumProperties(void)=0;
	virtual	void	GetNthPropertyName(int, string&)=0;
	virtual	double	GetNthPropertyValue(int)=0;
	virtual	void	SetNthPropertyValue(int, double)=0;
	
	virtual	int		GetNumButtons(void)=0;
	virtual	void	GetNthButtonName(int, string&)=0;
	virtual	void	NthButtonPressed(int)=0;
	
	virtual	char *	GetStatusText(void)=0;

protected:

	inline WED_MapZoomer *	GetZoomer(void) const { return mZoomer; }
		
private:

					WED_AbstractLayer();

		WED_MapZoomer *		mZoomer;
	
};

#endif /* WED_ABSTRACTLAYER_H */

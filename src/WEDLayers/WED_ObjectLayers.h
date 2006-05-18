#ifndef WED_OBJECTLAYERS_H
#define WED_OBJECTLAYERS_H

#include "WED_AbstractLayers.h"

class	WED_ObjectRoot;

class	WED_ObjectLayers : public WED_AbstractLayers {
public:

								 WED_ObjectLayers(WED_ObjectRoot * root);
	virtual						~WED_ObjectLayers();

	virtual	int					CountLayers(void);
	virtual	int					GetIndent(int n);
	virtual	int					GetLayerCapabilities(int n);
	virtual	int					GetLayerAllowedTools(int n);
	virtual int					GetFlags(int n) const;
	virtual	string				GetName(int n) const;
	virtual float				GetOpacity(int n) const;
	
	virtual	void				ToggleFlag(int n, int flag);
	virtual void				Rename(int n, const string& name);
	virtual	void				SetOpacity(int n, float opacity);

public:

		WED_ObjectRoot	*		mRoot;	

};

#endif

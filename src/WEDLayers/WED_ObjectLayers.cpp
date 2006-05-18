#include "WED_ObjectLayers.h"
#include "WED_ObjectPlacements.h"

WED_ObjectLayers::WED_ObjectLayers(WED_ObjectRoot * root) : mRoot(root)
{
}

WED_ObjectLayers::~WED_ObjectLayers()
{
}

int					WED_ObjectLayers::CountLayers(void)
{
	return mRoot->CountLayers();
}

int					WED_ObjectLayers::GetIndent(int n)
{
	return 0;
}

int					WED_ObjectLayers::GetLayerCapabilities(int n)
{
	return wed_Layer_Rename;
}

int					WED_ObjectLayers::GetLayerAllowedTools(int n)
{
	return 0;
}

int					WED_ObjectLayers::GetFlags(int n) const
{
	return wed_Flag_Visible;
}

string				WED_ObjectLayers::GetName(int n) const
{
	return mRoot->GetNthLayer(n)->GetName();
}

float				WED_ObjectLayers::GetOpacity(int n) const
{
	return 1.0;
}

void				WED_ObjectLayers::ToggleFlag(int n, int flag)
{
}

void				WED_ObjectLayers::Rename(int n, const string& name)
{
	mRoot->GetNthLayer(n)->SetName(name);
}

void				WED_ObjectLayers::SetOpacity(int n, float opacity)
{
}


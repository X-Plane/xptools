#include "WED_LayerGroup.h"
#include "WED_Messages.h"

 WED_LayerGroup::WED_LayerGroup(
 		int						flags,
 		int						caps,
 		const string&			name,
 		WED_AbstractLayers *	children) :
 	mChildren(children),
 	mFlags(flags),
 	mCaps(caps),
 	mName(name)
{
}
 
WED_LayerGroup::~WED_LayerGroup()
{
}

int					WED_LayerGroup::CountLayers(void)
{
	if (mFlags & wed_Flag_Children)
		return 1 + mChildren->CountLayers();
	else
		return 1;
}

int					WED_LayerGroup::GetIndent(int n)
{
	if (n == 0) return 0;
	return 1 + mChildren->GetIndent(n-1);
}

int					WED_LayerGroup::GetLayerCapabilities(int n)
{
	if (n == 0) return mCaps;
	return mChildren->GetLayerCapabilities(n-1);
}

int					WED_LayerGroup::GetLayerAllowedTools(int n)
{
	if (n == 0) return 0;
	return mChildren->GetLayerAllowedTools(n-1);
}

int					WED_LayerGroup::GetFlags(int n) const
{
	if (n == 0) return mFlags;
	return mChildren->GetFlags(n-1);
}

string				WED_LayerGroup::GetName(int n) const
{
	if (n == 0) return mName;
	return mChildren->GetName(n-1);
}

float				WED_LayerGroup::GetOpacity(int n) const
{
	if (n == 0) return 1.0;
	return mChildren->GetOpacity(n-1);
}

void				WED_LayerGroup::ToggleFlag(int n, int flag)
{
	if (n == 0)
	{
		if (flag & wed_Flag_Visible)
		if (mCaps & wed_Layer_Hide)
		{
			mFlags ^= wed_Flag_Visible;
			BroadcastMessage(msg_LayerCountChanged, 0);
		}
			
		if (flag & wed_Flag_Export)
		if (mCaps & wed_Layer_Export)
		{
			mFlags ^= wed_Flag_Export;
			BroadcastMessage(msg_LayerStatusChanged, 0);
		}

		if (flag & wed_Flag_Children)
		if (mCaps & wed_Layer_Children)
		{
			mFlags ^= wed_Flag_Children;
			BroadcastMessage(msg_LayerStatusChanged, 0);
		}
	} else
		mChildren->ToggleFlag(n-1,flag);
}

void				WED_LayerGroup::Rename(int n, const string& name)
{
	if (n == 0)
	{
		if (mCaps & wed_Layer_Rename)
		{
			mName = name;
			BroadcastMessage(msg_LayerStatusChanged, 0);
		}
	} else
		mChildren->Rename(n-1,name);
}

void				WED_LayerGroup::SetOpacity(int n, float opacity)
{
	if (n == 0)
	{
	} else
		mChildren->SetOpacity(n-1, opacity);
}

void		WED_LayerGroup::DrawLayer(
						int						n,
						GUI_GraphState *		state,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						overlay)
{
	if (mFlags & wed_Flag_Visible)
	if (n > 0)
		mChildren->DrawLayer(n-1,state,zoomer,tool,selected,overlay);
}

int					WED_LayerGroup::HandleMouseDown(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
	if (mFlags & wed_Flag_Visible)
	if (n > 0)
		return mChildren->HandleMouseDown(n-1,zoomer,tool,selected,x,y,button);
	return 0;
}

void				WED_LayerGroup::HandleMouseDrag(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
	if (mFlags & wed_Flag_Visible)
	if (n > 0)
		mChildren->HandleMouseDrag(n-1,zoomer,tool,selected,x,y,button);
}
						
void				WED_LayerGroup::HandleMouseUp(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
	if (mFlags & wed_Flag_Visible)
	if (n > 0)
		mChildren->HandleMouseUp(n-1,zoomer,tool,selected,x,y,button);
}

void				WED_LayerGroup::ReceiveMessage(
										GUI_Broadcaster *		inSrc,
										int						inMsg,
										int						inParam)
{
	switch(inMsg) {
	case msg_LayerStatusChanged:
	case msg_LayerCountChanged:
		if (mFlags & wed_Flag_Visible)
			BroadcastMessage(inMsg, inParam);
		break;
	}
}


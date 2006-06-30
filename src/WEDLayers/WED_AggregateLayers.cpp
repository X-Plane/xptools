#include "WED_AggregateLayers.h"
#include "WED_Messages.h"

WED_AggregateLayers::WED_AggregateLayers()
{
}

WED_AggregateLayers::~WED_AggregateLayers()
{
}
	
void				WED_AggregateLayers::AppendLayers(WED_AbstractLayers * layers)
{
	mLayers.push_back(layers);
	layers->AddListener(this);
}

int					WED_AggregateLayers::CountLayers(void)
{
	int total = 0;
	for (int n = 0; n < mLayers.size(); ++n)
		total += mLayers[n]->CountLayers();
	return total;
}

int					WED_AggregateLayers::GetIndent(int n)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetIndent(n);
	return 0;
}

int					WED_AggregateLayers::GetLayerCapabilities(int n)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetLayerCapabilities(n);
	return 0;
}

int					WED_AggregateLayers::GetLayerAllowedTools(int n)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetLayerAllowedTools(n);
	return 0;
}

int					WED_AggregateLayers::GetFlags(int n) const
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetFlags(n);
	return 0;
}

string				WED_AggregateLayers::GetName(int n) const
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetName(n);
	return string();
}

float				WED_AggregateLayers::GetOpacity(int n) const
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->GetOpacity(n);
	return 0.0f;
}

void				WED_AggregateLayers::ToggleFlag(int n, int flag)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) l->ToggleFlag(n, flag);
}

void				WED_AggregateLayers::Rename(int n, const string& name)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->Rename(n, name);
}

void				WED_AggregateLayers::SetOpacity(int n, float opacity)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->SetOpacity(n, opacity);
}

WED_AbstractLayers *		WED_AggregateLayers::Translate(int& n) const
{
	for (int i = 0; i < mLayers.size(); ++i)
	{
		int sz = mLayers[i]->CountLayers();
		if (n < sz)
			return mLayers[i];
		else
			n -= sz;
	}
	return NULL;
}

void	WED_AggregateLayers::DrawLayer(
			int						n,
			GUI_GraphState *		state,
			WED_MapZoomer *			zoomer,
			int						tool,
			int						selected,
			int						overlay)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) l->DrawLayer(n,state,zoomer,tool,selected,overlay);
}

int		WED_AggregateLayers::HandleMouseDown(
			int						n,
			WED_MapZoomer *			zoomer,
			int						tool,
			int						selected,
			int						x,
			int						y,
			int						button)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) return l->HandleMouseDown(n,zoomer,tool,selected,x,y,button);
	return 0;
}

void	WED_AggregateLayers::HandleMouseDrag(
			int						n,
			WED_MapZoomer *			zoomer,
			int						tool,
			int						selected,
			int						x,
			int						y,
			int						button)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) l->HandleMouseDrag(n,zoomer,tool,selected,x,y,button);
}

void	WED_AggregateLayers::HandleMouseUp(
			int						n,
			WED_MapZoomer *			zoomer,
			int						tool,
			int						selected,
			int						x,
			int						y,
			int						button)
{
	WED_AbstractLayers * l = Translate(n);
	if (l) l->HandleMouseUp(n,zoomer,tool,selected,x,y,button);
}

void	WED_AggregateLayers::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	switch(inMsg) {
	case msg_LayerStatusChanged:
	case msg_LayerCountChanged:
		BroadcastMessage(inMsg, inParam);
		break;
	}
}
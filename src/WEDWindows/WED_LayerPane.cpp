#include "WED_LayerPane.h"
#include "WED_AbstractLayers.h"
WED_LayerPane::WED_LayerPane(WED_AbstractLayers * layers) : mLayers(layers)
{
	layers->AddListener(this);
	RecalcSize();
}

WED_LayerPane::~WED_LayerPane()
{
}

void		WED_LayerPane::Draw(GUI_GraphState * state)
{
}

int			WED_LayerPane::MouseDown(int x, int y, int button)
{
	return 0;
}

void		WED_LayerPane::MouseDrag(int x, int y, int button)
{
}

void		WED_LayerPane::MouseUp(int x, int y, int button)
{
	
}

void		WED_LayerPane::ReceiveMessage(
GUI_Broadcaster *		inSrc,
int						inMsg,
int						inParam)
{
}

void		WED_LayerPane::RecalcSize(void)
{
	#if !DEV
		do this
	#endif
}

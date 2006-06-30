#include "GUI_Fonts.h"
#include "FontMgr.h"
#include "GUI_GraphState.h"

static const char * kFontNames[font_Max] = {
	"LucidaGrande.ttf"
//	"Arial"
};

static const int	kFontSizes[font_Max] = { 
	12
};

static FontHandle	sFonts[font_Max] = { 0 };
static FontMgr *	sFontMgr = NULL;

static const int	kAlign[3] = { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

static void MyGenerateTextures(int n,	int* textures) { glGenTextures (n, (unsigned long *) textures); }
static void MyBindTexture(int target,	int texture)   
{	 
	glBindTexture(target, texture);
}

static void	EstablishFont(int id)
{
	if (sFontMgr == NULL)
	{
		sFontMgr = new FontMgr;
		FontFuncs funcs = { MyGenerateTextures, MyBindTexture };
		sFontMgr->InstallCallbacks(&funcs);
	}	
	if (sFonts[id] == NULL)
	{
		sFonts[id] = sFontMgr->LoadFont(kFontNames[id], kFontSizes[id], true);	
	}
}

void	GUI_FontDraw(	
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString)
{
	EstablishFont(inFontID);
	inState->SetState(0,1,0,  1,1,  0, 0);
	
	sFontMgr->DrawString(sFonts[inFontID], color, inX, inY, inString);
}

void	GUI_FontDrawScaled(	
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inLeft,
				float							inBottom,
				float							inRight,
				float							inTop,
				const char *					inStart,
				const char *					inEnd,
				int								inAlign)
{
	EstablishFont(inFontID);
	inState->SetState(0,1,0,  1,1,  0, 0);
	
	sFontMgr->DrawRange(sFonts[inFontID], color, inLeft, inBottom, inRight, inTop, inStart, inEnd, kAlign[inAlign]);
}
float	GUI_MeasureRange(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd)
{
	EstablishFont(inFontID);
	return sFontMgr->MeasureRange(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inCharStart,inCharEnd);
}

int		GUI_FitForward(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	EstablishFont(inFontID);
	return sFontMgr->FitForward(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inSpace, inCharStart, inCharEnd);

}

int		GUI_FitReverse(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace)
{
	EstablishFont(inFontID);
	return sFontMgr->FitReverse(
							sFonts[inFontID], 
							sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]),
							inSpace, inCharStart, inCharEnd);
}

float	GUI_GetLineHeight(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineHeight(sFonts[inFontID], kFontSizes[inFontID]);
}

float	GUI_GetLineDescent(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineDescent(sFonts[inFontID], kFontSizes[inFontID]);
}

float	GUI_GetLineAscent(int inFontID)
{
	EstablishFont(inFontID);
	return sFontMgr->GetLineAscent(sFonts[inFontID], kFontSizes[inFontID]);
}


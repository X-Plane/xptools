#include "GUI_Fonts.h"
#include "FontMgr.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

static const char * kFontNames[font_Max] = {
	"Vera.ttf"
//	"Arial"
};

static const int	kFontSizes[font_Max] = { 
	10
};

static FontHandle	sFonts[font_Max] = { 0 };
static FontMgr *	sFontMgr = NULL;

static const int	kAlign[3] = { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

static void MyGenerateTextures(int n,	int* textures) { glGenTextures (n, (GLuint *) textures); }
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
		string full_path;
		if (GUI_GetResourcePath(kFontNames[id],full_path))
			sFonts[id] = sFontMgr->LoadFont(full_path.c_str(), kFontSizes[id], true);	
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


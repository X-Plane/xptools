#ifndef GUI_FONTS_H
#define GUI_FONTS_H

class	GUI_GraphState;
enum {

	font_UI_Basic = 0,
	font_Max,

	align_Left = 0,
	align_Center,
	align_Right

};

void	GUI_FontDraw(	
				GUI_GraphState *				inState,
				int 							inFontID,
				float							color[4],	//	4-part color, featuring alpha.
				float							inX,
				float							inY,
				const char *					inString);

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
				int								inAlign);

float	GUI_MeasureRange(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd);
int		GUI_FitForward(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace);

int		GUI_FitReverse(
				int 							inFontID,
				const char *					inCharStart,
				const char *					inCharEnd,
				float							inSpace);

float	GUI_GetLineHeight(int inFontID);
float	GUI_GetLineDescent(int inFontID);
float	GUI_GetLineAscent(int inFontID);

void	GUI_TruncateText(
				string&							ioText,
				int								inFontID,
				float							inSpace);

#endif

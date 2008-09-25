/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "OE_TexInfo.h"

#include "XPLMGraphics.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"
#include "OE_Globals.h"
#include "OE_TexMgr.h"


/*

	Name:

			Pixels	%
	Left
	Right
	Top
	Bottom

*/

const	long	kNameField 		= 1000;
const	long	kPixLField 		= 1001;
const	long	kPerLField 		= 1002;
const	long	kPixRField 		= 1003;
const	long	kPerRField 		= 1004;
const	long	kPixTField 		= 1005;
const	long	kPerTField 		= 1006;
const	long	kPixBField 		= 1007;
const	long	kPerBField 		= 1008;

static void	PrintToWidget(XPWidgetID host, long sub_id, const char * fmt, ...)
{
	va_list	args;
	va_start(args, fmt);
	static	char buf[512];
	vsprintf(buf, fmt, args);

	XPSetWidgetDescriptor((XPWidgetID) XPGetWidgetProperty(host, sub_id, NULL), buf);
}

static	void	OE_TexInfoNotify(int cat, int msg, void * param, void * ref);
static	int		OE_TexInfoFunc(		   XPWidgetMessage      inMessage,
	                                   XPWidgetID           inWidget,
	                                   long                 inParam1,
	                                   long                 inParam2);
static	void	ResyncTexInfo(XPWidgetID, bool inPush, long inWho);

XPWidgetID	OE_CreateTexInfo(int x1, int y1, int x2, int y2)
{
	XPWidgetID texInfo = XPCreateWidget(x1, y2, x2, y1, 1, "Texture Edit", 1, NULL, xpWidgetClass_MainWindow);
	XPSetWidgetProperty(texInfo, xpProperty_MainWindowHasCloseBoxes, 0);

	// Name
	XPCreateWidget(x1 + 10, y2 - 20, x1 + 90, y2 - 40,
				1, "Name:", 0, texInfo, xpWidgetClass_Caption);
	XPWidgetID name = XPCreateWidget(x1 + 100, y2 - 20, x1 + 290, y2 - 40,
				1, "untitled", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kNameField, (long) name);

	// Captions
	XPCreateWidget(x1 + 100, y2 - 40, x1 + 190, y2 - 60,
				1, "Pixels", 0, texInfo, xpWidgetClass_Caption);
	XPCreateWidget(x1 + 200, y2 - 40, x1 + 290, y2 - 60,
				1, "Percent", 0, texInfo, xpWidgetClass_Caption);

	// Left
	XPCreateWidget(x1 + 10, y2 - 60, x1 + 90, y2 - 80,
				1, "Left:", 0, texInfo, xpWidgetClass_Caption);
	XPWidgetID leftPix = XPCreateWidget(x1 + 100, y2 - 60, x1 + 190, y2 - 80,
				1, "0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPixLField, (long) leftPix);
	XPWidgetID leftPer = XPCreateWidget(x1 + 200, y2 - 60, x1 + 290, y2 - 80,
				1, "0.0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPerLField, (long) leftPer);

	// Right
	XPCreateWidget(x1 + 10, y2 - 80, x1 + 90, y2 - 100,
				1, "Right:", 0, texInfo, xpWidgetClass_Caption);
	XPWidgetID rightPix = XPCreateWidget(x1 + 100, y2 - 80, x1 + 190, y2 - 100,
				1, "1", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPixRField, (long) rightPix);
	XPWidgetID rightPer = XPCreateWidget(x1 + 200, y2 - 80, x1 + 290, y2 - 100,
				1, "1.0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPerRField, (long) rightPer);

	// Bottom
	XPCreateWidget(x1 + 10, y2 - 100, x1 + 90, y2 - 120,
				1, "Bottom:", 0, texInfo, xpWidgetClass_Caption);
	XPWidgetID botPix = XPCreateWidget(x1 + 100, y2 - 100, x1 + 190, y2 - 120,
				1, "0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPixBField, (long) botPix);
	XPWidgetID botPer = XPCreateWidget(x1 + 200, y2 - 100, x1 + 290, y2 - 120,
				1, "0.0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPerBField, (long) botPer);

	// Top
	XPCreateWidget(x1 + 10, y2 - 120, x1 + 90, y2 - 140,
				1, "Top:", 0, texInfo, xpWidgetClass_Caption);
	XPWidgetID topPix = XPCreateWidget(x1 + 100, y2 - 120, x1 + 190, y2 - 140,
				1, "0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPixTField, (long) topPix);
	XPWidgetID topPer = XPCreateWidget(x1 + 200, y2 - 120, x1 + 290, y2 - 140,
				1, "0.0", 0, texInfo, xpWidgetClass_TextField);
	XPSetWidgetProperty(texInfo, kPerTField, (long) topPer);



	XPAddWidgetCallback(texInfo, OE_TexInfoFunc);
	OE_Register(OE_TexInfoNotify, texInfo);

	return texInfo;
}

void	OE_TexInfoNotify(int cat, int msg, void * param, void * ref)
{
	if ((cat == catagory_Texture) ||
		(cat == catagory_Object && msg == msg_ObjectLoaded))
	{
		ResyncTexInfo((XPWidgetID) ref, true, 0);
	}
}

void	ResyncTexInfo(XPWidgetID me, bool inPush, long inWho)
{
	int twidth = 32, theight = 32;
	if (!gObjects.empty())
	{
		string tex = gObjects[gLevelOfDetail].texture;
		if (!tex.empty())
		{
			int twidth, theight;
			FindTexture(tex, false, &twidth, &theight);
		}
	}

	if (inPush)
	{
		if (gCurTexture == -1)
		{
			PrintToWidget(me, kNameField, "(no selection)");
			PrintToWidget(me, kPerLField, "0.0");
			PrintToWidget(me, kPixLField, "0");
			PrintToWidget(me, kPerRField, "1.0");
			PrintToWidget(me, kPixRField, "1");
			PrintToWidget(me, kPerBField, "0.0");
			PrintToWidget(me, kPixBField, "0");
			PrintToWidget(me, kPerTField, "1.0");
			PrintToWidget(me, kPixTField, "1");
		} else {


			PrintToWidget(me, kNameField, "%s", gTextures[gCurTexture].name.c_str());
			PrintToWidget(me, kPerLField, "%f", gTextures[gCurTexture].s1);
			PrintToWidget(me, kPixLField, "%d", (int) (gTextures[gCurTexture].s1 * twidth));
			PrintToWidget(me, kPerRField, "%f", gTextures[gCurTexture].s2);
			PrintToWidget(me, kPixRField, "%d", (int) (gTextures[gCurTexture].s2 * twidth));
			PrintToWidget(me, kPerBField, "%f", gTextures[gCurTexture].t1);
			PrintToWidget(me, kPixBField, "%d", (int) (gTextures[gCurTexture].t1 * theight));
			PrintToWidget(me, kPerTField, "%f", gTextures[gCurTexture].t2);
			PrintToWidget(me, kPixTField, "%d", (int) (gTextures[gCurTexture].t2 * theight));
		}
	} else {
		if (gCurTexture != -1)
		{
			float	f;
			if (inWho == XPGetWidgetProperty(me, kNameField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				gTextures[gCurTexture].name = buf;
			}

			if (inWho == XPGetWidgetProperty(me, kPerLField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].s1 = f;
			}
			if (inWho == XPGetWidgetProperty(me, kPerRField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].s2 = f;
			}
			if (inWho == XPGetWidgetProperty(me, kPerBField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].t1 = f;
			}
			if (inWho == XPGetWidgetProperty(me, kPerTField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].t2 = f;
			}

			if (inWho == XPGetWidgetProperty(me, kPixLField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].s1 = f / (float) twidth;
			}
			if (inWho == XPGetWidgetProperty(me, kPixRField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].s2 = f / (float) twidth;
			}
			if (inWho == XPGetWidgetProperty(me, kPixBField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].t1 = f / (float) theight;
			}
			if (inWho == XPGetWidgetProperty(me, kPixTField, NULL))
			{
				char	buf[256];
				XPGetWidgetDescriptor((XPWidgetID) inWho, buf, sizeof(buf));
				sscanf(buf, "%f", &f);
				gTextures[gCurTexture].t2 = f / (float) theight;
			}

			OE_Notify(catagory_Texture, msg_TexSelectionEdited, NULL);
		}
	}
}

int		OE_TexInfoFunc(		   XPWidgetMessage      inMessage,
	                                   XPWidgetID           inWidget,
	                                   long                 inParam1,
	                                   long                 inParam2)
{
	switch(inMessage) {
	case xpMsg_TextFieldChanged:
		ResyncTexInfo(inWidget, false, inParam1);
		return 1;
	default:
		return 0;
	}
}

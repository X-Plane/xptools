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
#ifndef RF_TERRATOOL_H
#define RF_TERRATOOL_H

#include "RF_MapTool.h"

class AsyncImage;
class AsyncImageLocator;
class AsyncConnectionPool;

class	RF_TerraTool : public RF_MapTool {
public:

					RF_TerraTool(RF_MapZoomer * inZoomer);
	virtual			~RF_TerraTool();

	virtual	void	DrawFeedbackUnderlay(
							GUI_GraphState *	state,
							bool				inCurrent);
	virtual	void	DrawFeedbackOverlay(
							GUI_GraphState *	state,
							bool				inCurrent);
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX,
							int 				inY,
							int 				inButton,
							GUI_KeyFlags		inModifiers);

	virtual int		GetNumProperties(void);
	virtual	void	GetNthPropertyName(int, string&);
	virtual	double	GetNthPropertyValue(int);
	virtual	void	SetNthPropertyValue(int, double);

	virtual	int		GetNumButtons(void);
	virtual	void	GetNthButtonName(int, string&);
	virtual	void	NthButtonPressed(int);

	virtual	char *	GetStatusText(int x, int y);

private:

	const char *	ResString(void);

	map<long long, AsyncImage*>		mImages;

	int mX1, mX2, mY1, mY2, mDomain, mHas;

	AsyncImageLocator *				mLocator;
	AsyncConnectionPool *			mPool;

	string							mData;
	int								mRes;
};

#endif

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
#ifndef RF_CROPTOOL_H
#define RF_CROPTOOL_H

#include "RF_MapTool.h"
#include "UIUtils.h"

class	WED_CropTool : public WED_MapTool, public DragHandleManager {
public:

					WED_CropTool(WED_MapZoomer * inZoomer);
	virtual			~WED_CropTool();

	virtual	void	DrawFeedbackUnderlay(
							bool				inCurrent);
	virtual	void	DrawFeedbackOverlay(
							bool				inCurrent);
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX,
							int 				inY,
							int 				inButton);

	// Support for some properties that can be edited.
	virtual int		GetNumProperties(void);
	virtual	void	GetNthPropertyName(int, string&);
	virtual	double	GetNthPropertyValue(int);
	virtual	void	SetNthPropertyValue(int, double);

	virtual	int		GetNumButtons(void);
	virtual	void	GetNthButtonName(int, string&);
	virtual	void	NthButtonPressed(int);

	virtual	char *	GetStatusText(void);

	virtual	double		UIToLogX(double) const;
	virtual	double		UIToLogY(double) const;
	virtual	double		LogToUIX(double) const;
	virtual	double		LogToUIY(double) const;

	virtual	double		GetHandleX(int inHandle) const;
	virtual	double		GetHandleY(int inHandle) const;

	virtual	void		MoveHandleX(int handle, double deltaX);
	virtual	void		MoveHandleY(int handle, double deltaY);

private:

	double			mBounds[4];
	bool			mCrop;

	DragHandleSet	mHandles;

};

#endif

/*
 * Copyright (c) 2004, Ben Supnik and Sandy Barbour.
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
#ifndef _XPLMPrivate_h__
#define _XPLMPrivate_h__


void	XPLMDisplayDoDrawingHook(void);

void	XPLMDisplayMouseClickHook(
						int 			inX,
						int 			inY,
						int				inIsDown,
						int				inButton);
int		XPLMDisplayKeyPressHook(
						long			inMsg,
						long			inParam1,
						long			inParam2);
void	XPLMDisplayMouseDragHook(void);

void	XPLMDisplayCleanupHook(void);

void	XPLMDoFlightLoopProcessing(float inElapsedTime);
void	XPLMProcessingCleanupHook(void);

#endif

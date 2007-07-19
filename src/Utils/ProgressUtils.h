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
#ifndef PROGRESSUTILS_H
#define PROGRESSUTILS_H

#define PROGRESS_START(__FUNC, __STAGE, __STAGECOUNT, __MSG)							if (__FUNC) __FUNC(__STAGE, __STAGECOUNT, __MSG, 0.0);
#define PROGRESS_SHOW(__FUNC, __STAGE, __STAGECOUNT, __MSG, __NOW, __MAX)				if (__FUNC) __FUNC(__STAGE, __STAGECOUNT, __MSG, (float) (__NOW) / (float) (__MAX));
#define PROGRESS_CHECK(__FUNC, __STAGE, __STAGECOUNT, __MSG, __NOW, __MAX, __INTERVAL)	if (__FUNC && (__INTERVAL) && (((__NOW) % (__INTERVAL)) == 0)) __FUNC(__STAGE, __STAGECOUNT, __MSG, (float) (__NOW) / (float) (__MAX));
#define PROGRESS_DONE(__FUNC, __STAGE, __STAGECOUNT, __MSG)								if (__FUNC) __FUNC(__STAGE, __STAGECOUNT, __MSG, 1.0);

typedef	bool (* ProgressFunc)(
						int				inCurrentStage,
						int				inCurrentStageCount,
						const char *	inCurrentStageName,
						float			inProgress);
						
bool	ConsoleProgressFunc(
						int				inCurrentStage,
						int				inCurrentStageCount,
						const char *	inCurrentStageName,
						float			inProgress);

#endif
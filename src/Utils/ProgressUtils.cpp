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
#include "ProgressUtils.h"

bool	ConsoleProgressFunc(
						int				inCurrentStage,
						int				inCurrentStageCount,
						const char *	inCurrentStageName,
						float			inProgress)
{
	static	bool	is_shown = false;
	static	int		last_char = 0;
	if (inProgress == 1.0)
	{
		if (is_shown)
		{	
			int pos = 50.0 * inProgress;
			if (pos > last_char)
			{
				for (int n = last_char; n < pos; ++n)
					printf(".");
				last_char = pos;
//				fflush(stdout);
	//			last_progress = inProgress;
			}
			printf("Done\n");
			is_shown = false;
		}
	} else {
		if (!is_shown)
		{
			is_shown = true;
			printf("Task %d of %d - %s...\n", inCurrentStage+1, inCurrentStageCount, inCurrentStageName);
			printf("+----+----+----+----+----+----+----+----+----+----+\n");			
//			last_progress = 0.0;
			last_char = 0;
		}
		int pos = 50.0 * inProgress;
		if (pos > last_char)
		{
			for (int n = last_char; n < pos; ++n)
				printf(".");
			last_char = pos;
			fflush(stdout);
//			last_progress = inProgress;
		}
	}
	
	return false;
}						

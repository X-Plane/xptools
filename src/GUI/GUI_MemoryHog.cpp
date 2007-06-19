/* 
 * Copyright (c) 2007, Laminar Research.
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

#include "GUI_MemoryHog.h"
#include <new>
static set<GUI_MemoryHog *>	sHogs;
static new_handler			sOldHandler;


GUI_MemoryHog::GUI_MemoryHog()
{
	sHogs.insert(this);
}

GUI_MemoryHog::~GUI_MemoryHog()
{
	sHogs.erase(this);
}
		
void	GUI_MemoryHog::InstallNewHandler(void)
{
	sOldHandler = set_new_handler(GUI_MemoryHog::our_new_handler);	
}

void	GUI_MemoryHog::RemoveNewHandler(void)
{
	set_new_handler(sOldHandler);	
}
	
void	GUI_MemoryHog::our_new_handler()
{
	for(set<GUI_MemoryHog *>::iterator h = sHogs.begin(); h != sHogs.end(); ++h)
	{
		if ((*h)->ReleaseMemory())	
			return;
	}
	if (sOldHandler)	sOldHandler();
	else				throw bad_alloc();
}


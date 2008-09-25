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

/*
	GUI_MemmoryHog - THEORY OF OPERATION

	Simply: GU_MemoryHog is a mix-in that lets classes participate in emergency memory management.  When we run out of memory,
	ReleaseMemory() is called on all memory hogs.  On each call, a participant should release some (but not necessarily) all memory
	in an attempt to free memory.  Result codes: true if we did free memory, false if we have nothing to free.

	This is called from operator new via the new_handler.  For example, WED's undo manager is a memory hog and it will urge undos
	to free memory in emergencies.

*/

#ifndef GUI_MemoryHog_H
#define GUI_MemoryHog_H

class GUI_MemoryHog {
public:

					 GUI_MemoryHog();
	virtual			~GUI_MemoryHog();

	virtual	bool	ReleaseMemory(void)=0;

	static	void	InstallNewHandler(void);
	static	void	RemoveNewHandler(void);

private:

	  static void	our_new_handler();

};

#endif /* GUI_MemoryHog_H */

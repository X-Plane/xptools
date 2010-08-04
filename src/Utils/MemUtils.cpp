/* 
 * Copyright (c) 2010, Laminar Research.
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

#include "MemUtils.h"

#if WANT_NED_MALLOC && 0

#include <nedmalloc.h>

void* operator new(std::size_t sz) throw (std::bad_alloc)
{
		void *	p;
		
	if (sz == 0) sz = 1;
	p = nedalloc::nedmalloc (sz);
	while (p == 0)
	{
		new_handler handler = set_new_handler(NULL);
							  set_new_handler(handler);
		if (! handler)
			throw bad_alloc();

		handler ();							// Ben says: handler MUST free up memory OR throw an exception!
		p = nedalloc::nedmalloc (sz);
	}

	return p;
}

void* operator new[](std::size_t s) throw (std::bad_alloc)
{
	return operator new(s);
}

void* operator new(std::size_t sz, const std::nothrow_t& nt) throw()
{
		void *	p;
		
	if (sz == 0) sz = 1;
	p = nedalloc::nedmalloc(sz);
	while (p == 0)
	{
		new_handler handler = set_new_handler(NULL);
							  set_new_handler(handler);
		if (! handler) return 0;
		try
		{
			handler ();
		}
		catch (bad_alloc &) { return 0; }

		p = nedalloc::nedmalloc(sz);
	}
	
	return p;
}

void* operator new[](std::size_t s, const std::nothrow_t& nt) throw()
{
	return operator new(s,nt);
}


void operator delete(void* p) throw() { if(p) nedalloc::nedfree(p); }
void operator delete[](void* p) throw() { if(p) nedalloc::nedfree(p); }
void operator delete(void* p, const std::nothrow_t&) throw() { if(p) nedalloc::nedfree(p); }
void operator delete[](void* p, const std::nothrow_t&) throw() { if(p) nedalloc::nedfree(p); }

#define ABORT_ON_ASSERT_FAILURE 0
#include <nedmalloc.c>

#endif

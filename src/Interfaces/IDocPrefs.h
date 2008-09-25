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

#ifndef IDocPrefs_H
#define IDocPrefs_H

#include "IBase.h"

class IDocPrefs : public virtual IBase {
public:

	virtual	int		ReadIntPref(const char * in_key, int in_default)=0;
	virtual	void	WriteIntPref(const char * in_key, int in_value)=0;
	virtual	double	ReadDoublePref(const char * in_key, double in_default)=0;
	virtual	void	WriteDoublePref(const char * in_key, double in_value)=0;
	virtual	string	ReadStringPref(const char * in_key, const string& in_default)=0;
	virtual	void	WriteStringPref(const char * in_key, const string& in_value)=0;

};

#endif /* IDocPrefs_H */

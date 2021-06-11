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
	// Bitfield for where to store preferences.
	// When reading preferences, if both document and global preferences are specified, we will try
	// reading the preference from the doc first, then read the global preference if the document
	// didn't contain it.
	// Integer set preferences are only stored in the doc.
	enum PrefType : unsigned
	{
		pref_type_doc = 1,
		pref_type_global = 2,
	};

	virtual	int		ReadIntPref(const char * in_key, int in_default, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	void	WriteIntPref(const char * in_key, int in_value, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	double	ReadDoublePref(const char * in_key, double in_default, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	void	WriteDoublePref(const char * in_key, double in_value, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	string	ReadStringPref(const char * in_key, const string& in_default, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	void	WriteStringPref(const char * in_key, const string& in_value, unsigned type = pref_type_doc | pref_type_global)=0;
	virtual	void	ReadIntSetPref(const char * in_key, set<int>& out_value)=0;
	virtual	void	WriteIntSetPref(const char * in_key, const set<int>& in_value)=0;
};

#endif /* IDocPrefs_H */

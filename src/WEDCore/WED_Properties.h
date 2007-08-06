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

#ifndef WED_PROPERTIES_H
#define WED_PROPERTIES_H

#include <string>
#include "SQLUtils.h"
using std::string;

class	WED_Properties {
public:
	 WED_Properties(sqlite3 * db);
	~WED_Properties();
	
	int	exists(const char * key);

	double	getd(const char * key);
	double	geti(const char * key);
	string	gets(const char * key);
	
	void	setd(const char * key, double		 v);
	void	seti(const char * key, int			 v);
	void	sets(const char * key, const string& v);
	
	
private:

	sql_init		mMakeTable;
	sql_init		mMakeIndex;

	sql_command		mCountKeys;
	sql_command		mGetKey;
	sql_command		mSetKey;
	
};

#endif /* WED_PROPERTIES_H */

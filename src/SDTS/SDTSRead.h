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
#ifndef SDTSREAD_H
#define SDTSREAD_H

#include <map>

#include "MemFileUtils.h"

#include <sdts++/io/sio_8211FieldFormat.h>

class	sc_Record;

class	SDTSDirectory {
public:
					SDTSDirectory(const char * inContainerPath, const char * inExternalPath);
					~SDTSDirectory();
	
	bool			DidLoad(void) const;	
	MFMemFile *		OpenModule(const string& inModuleName);
	void			GetAllModuleNames(vector<string>& outNames);
	
private:

		hash_map<string, pair<string, bool> >	mModuleMap;		// Name, external?
		MFFileSet *								mFiles;
		MFFileSet *								mExternal;

};	

void	AddConverters(SDTSDirectory& inDirectory, sio_8211_converter_dictionary& inDictionary);

class	SDTSModuleIterator {
public:

	SDTSModuleIterator(MFMemFile * inFile, sio_8211_converter_dictionary * inConverters);
	~SDTSModuleIterator();
	
	bool			Done(void);
	void			Get(sc_Record& outRecord);
	void			Increment(void);
	bool			Error(void);
	
private:

	struct SDTSModuleIteratorImp;
	SDTSModuleIteratorImp * mImp;

};

#endif /* STDREAD_H */

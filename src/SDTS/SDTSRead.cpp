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
#include "SDTSRead.h"
#include <iostream>
#include "memistreambuf.h"
#include <sdts++/io/sio_Reader.h>
#include <sdts++/io/sio_ConverterFactory.h>
#include <sdts++/container/sc_Record.h>

#include <sdts++/builder/sb_Catd.h>
#include <sdts++/builder/sb_Ddsh.h>
#include <sdts++/builder/sb_Iref.h>
#include <sdts++/builder/sb_Utils.h>

SDTSDirectory::SDTSDirectory(const char * inContainerPath, const char * inExternal)
{
	mFiles = FileSet_Open(inContainerPath);
	mExternal = inExternal ? FileSet_Open(inExternal) : NULL;
	if (mFiles)
	{
		sio_8211_converter_dictionary converters; // hints for reader for binary data

		int count = FileSet_Count(mFiles);
		for (int n = 0; n < count; ++n)
		{
			MFMemFile *	mf = FileSet_OpenNth(mFiles, n);
			if (!mf) continue;

			memstreambuf	buf(MemFile_GetBegin(mf), MemFile_GetEnd(mf));
			istream	ifs(&buf);
			sio_8211Reader          reader( ifs, &converters );
			sio_8211ForwardIterator i( reader );

			sc_Record record;
			
			while (i)
			{
				i.get(record);
				sb_Catd	catd;
				
				if (catd.setRecord(record))
				{
					string	nm, fl, ex;
					if (catd.getName(nm) && catd.getFile(fl) && catd.getExternal(ex))
					{
						mModuleMap.insert(hash_map<string,pair<string, bool> >::value_type(nm, pair<string, bool>(fl,ex == "Y" || ex == "y")));
					}
				} else 
					break;
				++i;
			}
			
			MemFile_Close(mf);
		}
	}
}
SDTSDirectory::~SDTSDirectory()
{
	if (mFiles)
		FileSet_Close(mFiles);
}

bool			SDTSDirectory::DidLoad(void) const
{
	return mFiles != NULL;
}

MFMemFile *		SDTSDirectory::OpenModule(const string& inModuleName)
{
	hash_map<string, pair<string, bool> >::iterator i = mModuleMap.find(inModuleName);
	if (i == mModuleMap.end()) 
		return NULL;
	else {
		if (i->second.second && !mExternal) 
			return NULL;
		return FileSet_OpenSpecific(i->second.second ? mExternal : mFiles, i->second.first.c_str());
	}
}

void	SDTSDirectory::GetAllModuleNames(vector<string>& outNames)
{
	outNames.clear();
	for (hash_map<string, pair<string, bool> >::iterator i = mModuleMap.begin();
		i != mModuleMap.end(); ++i)	
	{
		outNames.push_back(i->first);
	}
}

struct	SDTSModuleIterator::SDTSModuleIteratorImp {
	memstreambuf					buf;
	istream							ifs;
	sio_8211Reader					reader;
	sio_8211ForwardIterator			iter;

	SDTSModuleIteratorImp(MFMemFile * mf, sio_8211_converter_dictionary * conv) :
		buf(MemFile_GetBegin(mf), MemFile_GetEnd(mf)),
		ifs(&buf),
		reader(ifs, conv),
		iter(reader)
	{
	}
	
	bool	done(void) { return !(iter); }
	void	get(sc_Record& r) { iter.get(r); }
	void	increment(void) { ++iter; }
	bool	error(void) { return ifs.bad(); }
};
	
SDTSModuleIterator::SDTSModuleIterator(MFMemFile * inFile, sio_8211_converter_dictionary * conv) :
	mImp(new SDTSModuleIteratorImp(inFile, conv))
{
}
SDTSModuleIterator::~SDTSModuleIterator()
{
	delete mImp;
}
	
bool			SDTSModuleIterator::Done(void)
{
	return mImp->done();
}

void			SDTSModuleIterator::Get(sc_Record& outRecord)
{
	mImp->get(outRecord);
}

void			SDTSModuleIterator::Increment(void)
{
	mImp->increment();
}

bool			SDTSModuleIterator::Error(void)
{
	return mImp->error();
}

void	AddConverters(SDTSDirectory& inDirectory, sio_8211_converter_dictionary& inDictionary)
{
	sio_8211_converter_dictionary	dummy;
	sc_Record	rec;
	sb_Iref		iref;
	sb_Ddsh		ddsh;
	
	MFMemFile * irefFile = inDirectory.OpenModule("IREF");
	if (irefFile)
	{
		SDTSModuleIterator	irefIter(irefFile, &dummy);
		while (!irefIter.Done())
		{
			irefIter.Get(rec);
			if (iref.setRecord(rec))
			{
				sb_Utils::addConverter(iref, inDictionary);
			}
			irefIter.Increment();
		}
		
		MemFile_Close(irefFile);
	}
	
	MFMemFile * ddshFile = inDirectory.OpenModule("DDSH");
	if (ddshFile)
	{
		SDTSModuleIterator	ddshIter(ddshFile, &dummy);
		while (!ddshIter.Done())
		{
			ddshIter.Get(rec);
			if (ddsh.setRecord(rec))
			{
				sb_Utils::addConverter(ddsh, inDictionary);
			}
			ddshIter.Increment();
		}
		
		MemFile_Close(ddshFile);
	}
}

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
#include "VPFTable.h"
#include "MemFileUtils.h"

const char kFlipShort[] = { 2, 0 };
const char kFlipInt[] = { 4, 0 };
const char kFlipFloat[] = { 4, 0 };
const char kFlipDouble[] = { 8, 0 };

inline void ZeroFixf(float& v)
{
	if (-1.0e-9 < v && v < 1.0e-9)
		v = 0.0;
}

inline void ZeroFixd(double& v)
{
	if (-1.0e-9 < v && v < 1.0e-9)
		v = 0.0;
}


void	TokenizeString(const string& inString, vector<string>& outTokens, char inSep)
{
	string::size_type startPos = 0;
	string::size_type endPos;

	while (startPos < inString.length())
	{
		endPos = inString.find(inSep, startPos);
		if (endPos == inString.npos)
		{
			outTokens.push_back(inString.substr(startPos));
			return;
		} else {
			outTokens.push_back(inString.substr(startPos, endPos - startPos));
			startPos = endPos + 1;
		}
	}
}

bool	ReadVPFTableHeader(MFMemFile * inFile, VPF_TableDef& outDef)
{
	const char *	bPtr = MemFile_GetBegin(inFile);
	const char *	ePtr = MemFile_GetEnd(inFile);
	int				len = ePtr - bPtr;
	bool			hasEndian = false;

	if (len < 5)
		return false;

	outDef.endian = platform_LittleEndian;

	if (bPtr[5] == ';')
	{
		if (bPtr[4] == 'L' || bPtr[4] == 'l')
			outDef.endian = platform_LittleEndian, hasEndian = true;
		if (bPtr[4] == 'M' || bPtr[4] == 'm')
			outDef.endian = platform_BigEndian, hasEndian = true;
	}

	const char *	hStart = bPtr + 4;	// (hasEndian ? 6 : 4);
	int				headerLen = *((int *) bPtr);

	EndianSwapBuffer(outDef.endian, platform_Native, kFlipInt, &headerLen);

	const char *	hEnd = hStart + headerLen;

	string	header(hStart, hEnd);

	vector<string>	headerFields, columnDefs;

	TokenizeString(header, headerFields, ';');

	if (headerFields.empty()) return false;
	if (hasEndian) headerFields.erase(headerFields.begin());

	if (headerFields.size() < 3) return false;
	outDef.name = headerFields[0];
	outDef.desc = headerFields[1];
	TokenizeString(headerFields[2], columnDefs, ':');

	for (vector<string>::iterator col = columnDefs.begin(); col != columnDefs.end(); ++col)
	{
		vector<string>	defParts;
		string::size_type div = col->find('=');
		if (div == col->npos) return false;

		VPF_ColumnDef 	colDef;
		colDef.name = col->substr(0, div);
		// Must have at least the first 3 columns
		TokenizeString(col->substr(div+1), defParts, ',');
		if (defParts.size() < 3) return false;
		// data type
		if (defParts[0].size() != 1) return false;
		colDef.dataType = defParts[0][0];
		// data count
		if (defParts[1].empty()) return false;
		if (defParts[1].size() > 3) return false;
		colDef.elementCount = atoi(defParts[1].c_str());
		// key type
		if (defParts[2].size() != 1) return false;
		colDef.keyType = defParts[2][0];
		// desc
		if (defParts.size() > 3)	colDef.description = defParts[3];
		if (defParts.size() > 4)	colDef.descTableName = defParts[4];
		if (defParts.size() > 5)	colDef.themeTableName = defParts[5];
		if (defParts.size() > 6)	colDef.narrativeFileName = defParts[6];

		outDef.columns.push_back(colDef);
	}
	return true;
}

void	DumpVPFTableHeader(const VPF_TableDef& inDef)
{
	printf("Table is %s endian.\n", inDef.endian == platform_LittleEndian ? "little" : "big");
	printf("Table name: %s\n", inDef.name.c_str());
	printf("Table description: %s\n", inDef.desc.c_str());
	int ii = 0;
	for (vector<VPF_ColumnDef>::const_iterator i = inDef.columns.begin(); i != inDef.columns.end(); ++i, ++ii)
	{
		printf("% 4d %-17s %c % 4d %c %-50s %-14s %-14s %-14s\n",
			ii,
			i->name.c_str(),
			i->dataType,
			i->elementCount,
			i->keyType,
			i->description.c_str(),
			i->descTableName.c_str(),
			i->themeTableName.c_str(),
			i->narrativeFileName.c_str());
	}
}

bool	VPF_TableDef::IsFieldString(int n) const
{
	switch(columns[n].dataType) {
	case 'D':
	case 'T':
	case 'L':
	case 'M':
	case 'N':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldArray(int n) const
{
	if (IsFieldString(n)) return false;
	return columns[n].elementCount != 1;
}

int		VPF_TableDef::IsFieldNumeric(int n) const
{
	switch(columns[n].dataType) {
	case 'S':
	case 'I':
	case 'F':
	case 'R':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldInt(int n) const
{
	switch(columns[n].dataType) {
	case 'S':
	case 'I':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldFloat(int n) const
{
	switch(columns[n].dataType) {
	case 'F':
	case 'R':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldTwoTuple(int n) const
{
	switch(columns[n].dataType) {
	case 'C':
	case 'B':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldThreeTuple(int n) const
{
	switch(columns[n].dataType) {
	case 'Y':
	case 'Z':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::IsFieldTripletKey(int n) const
{
	switch(columns[n].dataType) {
	case 'K':
		return true;
	default:
		return false;
	}
}

int		VPF_TableDef::GetColumnByName(const string& inName) const
{
	for (int n = 0; n < columns.size(); ++n)
	{
		if (columns[n].name == inName) return n;
	}
	return -1;
}



VPFTableIterator::VPFTableIterator(MFMemFile * inFile, const VPF_TableDef& inDef) :
	mFile(inFile),
	mDef(inDef)
{
	mCurrentRecord = MemFile_GetBegin(inFile);
	int headerLen = *((int *) mCurrentRecord);
	EndianSwapBuffer(mDef.endian, platform_Native, kFlipInt, &headerLen);
	mCurrentRecord += (headerLen + 4);

	ParseCurrent();
}

bool	VPFTableIterator::Done(void)
{
	return (mCurrentRecord == NULL || mCurrentRecord == MemFile_GetEnd(mFile));
}

void	VPFTableIterator::Next(void)
{
	if (!Done())
	{
		mCurrentRecord = mNextRecord;
		ParseCurrent();
	}
}

bool	VPFTableIterator::Error(void)
{
	return mCurrentRecord == NULL;
}

int		VPFTableIterator::GetFieldCount(void)
{
	return mFieldOffsets.size();
}

// RAW FETCHERS

static const char *	GetRawChar(const char * inP, char& outValue, PlatformType endian);
static const char *	GetRawUChar(const char * inP, unsigned char& outValue, PlatformType endian);
static const char *	GetRawShort(const char * inP, short& outValue, PlatformType endian);
static const char *	GetRawUShort(const char * inP, unsigned short& outValue, PlatformType endian);
static const char *	GetRawInt(const char * inP, int& outValue, PlatformType endian);
static const char *	GetRawUInt(const char * inP, unsigned int& outValue, PlatformType endian);
static const char *	GetRawFloat(const char * inP, float& outValue, PlatformType endian);
static const char *	GetRawDouble(const char * inP, double& outDouble, PlatformType endian);

bool	VPFTableIterator::GetNthFieldAsString(int n, string& s)
{
	if (!mCurrentRecord) return false;
	const char * sstart = mCurrentRecord + mFieldOffsets[n];
	int len = mDef.columns[n].elementCount;
	if (len == 0)
	{
		sstart = GetRawInt(sstart, len, mDef.endian);
	}
	switch(mDef.columns[n].dataType) {
	case 'T':
	case 'L':
	case 'M':
	case 'N':
		break;
	case 'D':
		len *= 200;
		break;
	default: return false;
	}
	s = string(sstart, sstart + len);
	return true;
}

bool	VPFTableIterator::GetNthFieldAsInt(int n, int& v)
{
	if (!mCurrentRecord) return false;
	short	sval;
	int		ival;
	float	fval;
	double	dval;
	if (mDef.columns[n].elementCount != 1) return false;
	switch(mDef.columns[n].dataType) {
	case 'S':	GetRawShort(mCurrentRecord + mFieldOffsets[n], sval, mDef.endian);	v = sval; return true;
	case 'I':	GetRawInt(mCurrentRecord + mFieldOffsets[n], ival, mDef.endian);	v = ival; return true;
	case 'F':	GetRawFloat(mCurrentRecord + mFieldOffsets[n], fval, mDef.endian);	v = fval; return true;
	case 'R':	GetRawDouble(mCurrentRecord + mFieldOffsets[n], dval, mDef.endian);	v = dval; return true;
	default:	return false;
	}
}

bool	VPFTableIterator::GetNthFieldAsDouble(int n, double& v)
{
	if (!mCurrentRecord) return false;
	short	sval;
	int		ival;
	float	fval;
	double	dval;
	if (mDef.columns[n].elementCount != 1) return false;
	switch(mDef.columns[n].dataType) {
	case 'S':	GetRawShort(mCurrentRecord + mFieldOffsets[n], sval, mDef.endian);	v = sval; ZeroFixd(v); return true;
	case 'I':	GetRawInt(mCurrentRecord + mFieldOffsets[n], ival, mDef.endian);	v = ival; ZeroFixd(v); return true;
	case 'F':	GetRawFloat(mCurrentRecord + mFieldOffsets[n], fval, mDef.endian);	v = fval; ZeroFixd(v); return true;
	case 'R':	GetRawDouble(mCurrentRecord + mFieldOffsets[n], dval, mDef.endian);	v = dval; ZeroFixd(v); return true;
	default:	return false;
	}
}

bool	VPFTableIterator::GetNthFieldAsCoordPair(int n, Point2& p)
{
	if (mCurrentRecord) return false;
	if (mDef.columns[n].elementCount != 1) return false;

	double	d1, d2;
	float	f1, f2;
	const char * start = mCurrentRecord + mFieldOffsets[n];

	switch(mDef.columns[n].dataType) {
	case 'C':		// Two-pair float
	case 'Z':		// Three-pair float
		start = GetRawFloat(start, f1, mDef.endian);
		start = GetRawFloat(start, f2, mDef.endian);
		ZeroFixf(f1);
		ZeroFixf(f2);
		p = Point2(f1, f2);
		return true;
	case 'B':		// Two-pair double
	case 'Y':		// Three-pair double
		start = GetRawDouble(start, d1, mDef.endian);
		start = GetRawDouble(start, d2, mDef.endian);
		ZeroFixd(d1);
		ZeroFixd(d2);
		p = Point2(d1, d2);
		return true;
	default:
		return false;
	}
}

bool	VPFTableIterator::GetNthFieldAsCoordTriple(int n , Point3& p)
{
	if (mCurrentRecord) return false;
	if (mDef.columns[n].elementCount != 1) return false;

	double	d1, d2, d3 = 0.0;;
	float	f1, f2, f3 = 0.0;;
	const char * start = mCurrentRecord + mFieldOffsets[n];
	bool	hasThree = true;

	switch(mDef.columns[n].dataType) {
	case 'C':		// Two-pair float
		hasThree = false;
	case 'Z':		// Three-pair float
		start = GetRawFloat(start, f1, mDef.endian);
		start = GetRawFloat(start, f2, mDef.endian);
		if (hasThree)
		start = GetRawFloat(start, f3, mDef.endian);
		ZeroFixf(f1);
		ZeroFixf(f2);
		ZeroFixf(f3);
		p = Point3(f1, f2, f3);
		return true;
	case 'B':		// Two-pair double
		hasThree = false;
	case 'Y':		// Three-pair double
		start = GetRawDouble(start, d1, mDef.endian);
		start = GetRawDouble(start, d2, mDef.endian);
		if (hasThree)
		start = GetRawDouble(start, d3, mDef.endian);
		ZeroFixd(d1);
		ZeroFixd(d2);
		ZeroFixd(d3);
		p = Point3(d1, d2, d3);
		return true;
	default:
		return false;
	}
}

bool	VPFTableIterator::GetNthFieldAsIntArray(int n, vector<int>& v)
{
	if (!mCurrentRecord) return false;
	const char * start = mCurrentRecord + mFieldOffsets[n];
	int len = mDef.columns[n].elementCount;
	short	sval;
	int		ival;
	float	fval;
	double	dval;
	if (len == 0)
		start = GetRawInt(start, len, mDef.endian);
	v.clear();
	while (len--)
	{
		switch(mDef.columns[n].dataType) {
		case 'S':	start = GetRawShort(start, sval, mDef.endian);	v.push_back(sval); break;
		case 'I':	start = GetRawInt(start, ival, mDef.endian);	v.push_back(ival); break;
		case 'F':	start = GetRawFloat(start, fval, mDef.endian);	v.push_back(fval); break;
		case 'R':	start = GetRawDouble(start, dval, mDef.endian);	v.push_back(dval); break;
		default:	return false;
		}
	}
	return true;
}

bool	VPFTableIterator::GetNthFieldAsDoubleArray(int n, vector<double>& v)
{
	if (!mCurrentRecord) return false;
	const char * start = mCurrentRecord + mFieldOffsets[n];
	int len = mDef.columns[n].elementCount;
	short	sval;
	int		ival;
	float	fval;
	double	dval;
	if (len == 0)
		start = GetRawInt(start, len, mDef.endian);
	v.clear();
	while (len--)
	{
		switch(mDef.columns[n].dataType) {
		case 'S':	start = GetRawShort(start, sval, mDef.endian);					v.push_back(sval); break;
		case 'I':	start = GetRawInt(start, ival, mDef.endian);					v.push_back(ival); break;
		case 'F':	start = GetRawFloat(start, fval, mDef.endian);	ZeroFixf(fval); v.push_back(fval); break;
		case 'R':	start = GetRawDouble(start, dval, mDef.endian);	ZeroFixd(dval); v.push_back(dval); break;
		default:	return false;
		}
	}
	return true;
}

bool	VPFTableIterator::GetNthFieldAsCoordPairArray(int n, vector<Point2>& v)
{
	if (!mCurrentRecord) return false;
	const char * start = mCurrentRecord + mFieldOffsets[n];
	int len = mDef.columns[n].elementCount;
	double	d1, d2, d3;
	float	f1, f2, f3;
	if (len == 0)
		start = GetRawInt(start, len, mDef.endian);
	v.clear();
	while (len--)
	{
		switch(mDef.columns[n].dataType) {
		case 'C':
			start = GetRawFloat(start, f1, mDef.endian);
			start = GetRawFloat(start, f2, mDef.endian);
			ZeroFixf(f1);
			ZeroFixf(f2);
			v.push_back(Point2(f1, f2));
			break;
		case 'B':
			start = GetRawDouble(start, d1, mDef.endian);
			start = GetRawDouble(start, d2, mDef.endian);
			ZeroFixd(d1);
			ZeroFixd(d2);
			v.push_back(Point2(d1, d2));
			break;
		case 'Z':
			start = GetRawFloat(start, f1, mDef.endian);
			start = GetRawFloat(start, f2, mDef.endian);
			start = GetRawFloat(start, f3, mDef.endian);
			ZeroFixf(f1);
			ZeroFixf(f2);
			v.push_back(Point2(f1, f2));
			break;
		case 'Y':
			start = GetRawDouble(start, d1, mDef.endian);
			start = GetRawDouble(start, d2, mDef.endian);
			start = GetRawDouble(start, d3, mDef.endian);
			ZeroFixd(d1);
			ZeroFixd(d2);
			v.push_back(Point2(d1, d2));
			break;
		default:	return false;
		}
	}
	return true;
}

bool	VPFTableIterator::GetNthFieldAsCoordTripleArray(int n, vector<Point3>& v)
{
	if (!mCurrentRecord) return false;
	const char * start = mCurrentRecord + mFieldOffsets[n];
	int len = mDef.columns[n].elementCount;
	double	d1, d2, d3;
	float	f1, f2, f3;
	if (len == 0)
		start = GetRawInt(start, len, mDef.endian);
	v.clear();
	while (len--)
	{
		switch(mDef.columns[n].dataType) {
		case 'C':
			start = GetRawFloat(start, f1, mDef.endian);
			start = GetRawFloat(start, f2, mDef.endian);
			f3 = 0.0;
			ZeroFixf(f1);
			ZeroFixf(f2);
			v.push_back(Point3(f1, f2, f3));
			break;
		case 'B':
			start = GetRawDouble(start, d1, mDef.endian);
			start = GetRawDouble(start, d2, mDef.endian);
			d3 = 0.0;
			ZeroFixd(d1);
			ZeroFixd(d2);
			v.push_back(Point3(d1, d2, d3));
			break;
		case 'Z':
			start = GetRawFloat(start, f1, mDef.endian);
			start = GetRawFloat(start, f2, mDef.endian);
			start = GetRawFloat(start, f3, mDef.endian);
			ZeroFixf(f1);
			ZeroFixf(f2);
			ZeroFixf(f3);
			v.push_back(Point3(f1, f2, f3));
			break;
		case 'Y':
			start = GetRawDouble(start, d1, mDef.endian);
			start = GetRawDouble(start, d2, mDef.endian);
			start = GetRawDouble(start, d3, mDef.endian);
			ZeroFixd(d1);
			ZeroFixd(d2);
			ZeroFixd(d3);
			v.push_back(Point3(d1, d2, d3));
			break;
		default:	return false;
		}
	}
	return true;
}

bool	VPFTableIterator::GetNthFieldAsTripletKey(int n, VPF_TripletKey& k)
{
	if (!mCurrentRecord) return false;
	if (mDef.columns[n].dataType != 'K') return false;
	if (mDef.columns[n].elementCount != 1) return false;

	const char *	start = mCurrentRecord + mFieldOffsets[n];
	unsigned char ctrlCode = *start;
	++start;
	unsigned char	cval;
	unsigned short	sval;
	unsigned int	ival;

	if ((ctrlCode & 0xC0) == 0xC0) { start = GetRawUInt(start, ival, mDef.endian); k.row_id = ival; }
	if ((ctrlCode & 0xC0) == 0x80) { start = GetRawUShort(start, sval, mDef.endian); k.row_id = sval; }
	if ((ctrlCode & 0xC0) == 0x40) { start = GetRawUChar(start, cval, mDef.endian); k.row_id = cval; }
	if ((ctrlCode & 0xC0) == 0x00) { k.row_id = 0; }

	if ((ctrlCode & 0x30) == 0x30) { start = GetRawUInt(start, ival, mDef.endian); k.tile_id = ival; }
	if ((ctrlCode & 0x30) == 0x20) { start = GetRawUShort(start, sval, mDef.endian); k.tile_id = sval; }
	if ((ctrlCode & 0x30) == 0x10) { start = GetRawUChar(start, cval, mDef.endian); k.tile_id = cval; }
	if ((ctrlCode & 0x30) == 0x00) { k.tile_id = 0; }

	if ((ctrlCode & 0x0C) == 0x0C) { start = GetRawUInt(start, ival, mDef.endian); k.next_row_id = ival; }
	if ((ctrlCode & 0x0C) == 0x08) { start = GetRawUShort(start, sval, mDef.endian); k.next_row_id = sval; }
	if ((ctrlCode & 0x0C) == 0x04) { start = GetRawUChar(start, cval, mDef.endian); k.next_row_id = cval; }
	if ((ctrlCode & 0x0C) == 0x00) { k.next_row_id = 0; }

	return true;
}


void	VPFTableIterator::ParseCurrent(void)
{
	if (mCurrentRecord == MemFile_GetEnd(mFile))	return;
	mFieldOffsets.clear();
	int off = 0;
	int	itemSize = 0;
	int	varCount;
	unsigned char ctrlCode;
	for (int i = 0; i < mDef.columns.size(); ++i)
	{
		switch(mDef.columns[i].dataType) {
		case 'X':				// Null field
			itemSize = 0;
		case 'T':					// ASCII
		case 'L':					// Latin-1
		case 'M':					// Multilingual
		case 'N':					// Multilingual
			itemSize = 1;
			break;
		case 'S':					// short
			itemSize = 2;
			break;
		case 'F':					// float
		case 'I':					// int
			itemSize = 4;
			break;
		case 'R':					// double
		case 'C':					// float pair
			itemSize = 8;
			break;
		case 'Z':					// float triple
			itemSize = 12;
			break;
		case 'B':					// double pair
			itemSize = 20;
			break;
		case 'D':					// Date
			itemSize = 20;
			break;
		case 'Y':					// double triple
			itemSize = 24;
			break;
		case 'K':
			ctrlCode = mCurrentRecord[off];
			itemSize = 1;
			if ((ctrlCode & 0xC0) == 0xC0)	itemSize += 4;
			if ((ctrlCode & 0xC0) == 0x80)	itemSize += 2;
			if ((ctrlCode & 0xC0) == 0x40)	itemSize += 1;
			if ((ctrlCode & 0x30) == 0x30)	itemSize += 4;
			if ((ctrlCode & 0x30) == 0x20)	itemSize += 2;
			if ((ctrlCode & 0x30) == 0x10)	itemSize += 1;
			if ((ctrlCode & 0x0C) == 0x0C)	itemSize += 4;
			if ((ctrlCode & 0x0C) == 0x08)	itemSize += 2;
			if ((ctrlCode & 0x0C) == 0x04)	itemSize += 1;
			if ((ctrlCode & 0x03) == 0x03)	itemSize += 4;
			if ((ctrlCode & 0x03) == 0x02)	itemSize += 2;
			if ((ctrlCode & 0x03) == 0x01)	itemSize += 1;
			break;
		}
		if (mDef.columns[i].elementCount == 0)
		{
			varCount = *((int *) (mCurrentRecord + off));
			EndianSwapBuffer(mDef.endian, platform_Native, kFlipInt, &varCount);
			itemSize = 4 + itemSize * varCount;
		} else
			itemSize *= mDef.columns[i].elementCount;

		mFieldOffsets.push_back(off);
		off += itemSize;
	}

	mNextRecord = mCurrentRecord + off;
	if (mNextRecord <= mCurrentRecord || mNextRecord > MemFile_GetEnd(mFile))
		mCurrentRecord = NULL;
}


const char *	GetRawChar(const char * inP, char& outValue, PlatformType endian)
{
	outValue = *((char *) inP);
	return inP + 1;
}

const char *	GetRawUChar(const char * inP, unsigned char& outValue, PlatformType endian)
{
	outValue = *((unsigned char *) inP);
	return inP + 1;
}

const char *	GetRawShort(const char * inP, short& outValue, PlatformType endian)
{
	outValue = *((short *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipShort, &outValue);
	return inP + 2;
}

const char *	GetRawUShort(const char * inP, unsigned short& outValue, PlatformType endian)
{
	outValue = *((unsigned short *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipShort, &outValue);
	return inP + 2;
}

const char *	GetRawInt(const char * inP, int& outValue, PlatformType endian)
{
	outValue = *((int *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipInt, &outValue);
	return inP + 4;
}

const char *	GetRawUInt(const char * inP, unsigned int& outValue, PlatformType endian)
{
	outValue = *((unsigned int *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipInt, &outValue);
	return inP + 4;
}

const char *	GetRawFloat(const char * inP, float& outValue, PlatformType endian)
{
	outValue = *((float *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipFloat, &outValue);
	return inP + 4;
}

const char *	GetRawDouble(const char * inP, double& outValue, PlatformType endian)
{
	outValue = *((double *) inP);
	EndianSwapBuffer(endian, platform_Native, kFlipDouble, &outValue);
	return inP + 8;
}

void	DumpVPFTable(MFMemFile * inFile, const VPF_TableDef& inDef)
{
	VPFTableIterator	iter(inFile, inDef);
	int i = 0;
	while (!iter.Done())
	{
		printf("(% 5d) ", i++);
		for (int n = 0; n < iter.GetFieldCount(); ++n)
		{
			if (inDef.IsFieldString(n))
			{
				string	f;
				iter.GetNthFieldAsString(n, f);
				printf("%s ", f.c_str());
			} else if (inDef.IsFieldTripletKey(n)) {
				VPF_TripletKey	k;
				iter.GetNthFieldAsTripletKey(n, k);
				printf("%d/%d/%d ", k.row_id, k.tile_id, k.next_row_id);
			} else {
				if (inDef.IsFieldArray(n))
				{
					if (inDef.IsFieldInt(n)) {
						vector<int> i;
						iter.GetNthFieldAsIntArray(n, i);
						printf("(");
						for (int n = 0; n < i.size(); ++n)
						{
							printf("%d ", i[n]);
						}
						printf(")");
					} else if (inDef.IsFieldFloat(n)) {
						vector<double> d;
						iter.GetNthFieldAsDoubleArray(n, d);
						printf("(");
						for (int n = 0; n < d.size(); ++n)
						{
							printf("%lf ", d[n]);
						}
						printf(")");
					}  else if (inDef.IsFieldTwoTuple(n)) {
						vector<Point2> 	p2;
						iter.GetNthFieldAsCoordPairArray(n, p2);
						printf("(");
						for (int n = 0; n < p2.size(); ++n)
						{
							printf("%f,%f ", p2[n].x(), p2[n].y());
						}
						printf(")");
					} else if (inDef.IsFieldThreeTuple(n)) {
						vector<Point3> 	p3;
						iter.GetNthFieldAsCoordTripleArray(n, p3);
						printf("(");
						for (int n = 0; n < p3.size(); ++n)
						{
							printf("%f,%f,%f ", p3[n].x, p3[n].y, p3[n].z);
						}
						printf(")");
					}
				} else {
					if (inDef.IsFieldInt(n)) {
						int i;
						iter.GetNthFieldAsInt(n, i);
						printf("%d ", i);
					} else if (inDef.IsFieldFloat(n)) {
						double d;
						iter.GetNthFieldAsDouble(n, d);
						printf("%f ", d);
					} else if (inDef.IsFieldTwoTuple(n)) {
						Point2 	p2;
						iter.GetNthFieldAsCoordPair(n, p2);
						printf("%f,%f ", p2.x(), p2.y());
					} else if (inDef.IsFieldThreeTuple(n)) {
						Point3 	p3;
						iter.GetNthFieldAsCoordTriple(n, p3);
						printf("%f,%f,%f ", p3.x, p3.y, p3.z);
					}
				}
			}
		}

		printf("\n");

		iter.Next();
	}
	if (iter.Error())
		printf("Got error with table.\n");
	else
		printf("End of table.\n");
}

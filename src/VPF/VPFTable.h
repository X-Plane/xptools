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
#ifndef VPFTABLE_H
#define VPFTABLE_H

#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

#include "EndianUtils.h"
#include "MemFileUtils.h"

struct	VPF_TripletKey {
	unsigned int row_id;
	unsigned int tile_id;
	unsigned int next_row_id;
};

struct	VPF_ColumnDef {
	string		name;
	
	char		dataType;
	int			elementCount;
	char		keyType;

	string		description;

	string		descTableName;
	string		themeTableName;
	string		narrativeFileName;	
};

struct	VPF_TableDef {

	PlatformType			endian;
	
	string					name;
	string					desc;
	vector<VPF_ColumnDef>	columns;

	// These accessors allow you to learn things about
	// fields without having to parse the codes from PVF.	
	// This lib simplifies the rich set of VPF types as follows:
	// All char arrays, fixed and var length and all char sets are strings.
	// All numeric types are interchangeable.
	// All ints are type int, all floats are type double (e.g. bits are added in memory.
	// Also please note that 'is array' will return false for strings, since they're really
	// arrays of chars.
	bool	IsFieldString(int) const;
	int		IsFieldArray(int) const;
	int		IsFieldNumeric(int) const;
	int		IsFieldInt(int) const;
	int		IsFieldFloat(int) const;
	int		IsFieldTwoTuple(int) const;
	int		IsFieldThreeTuple(int) const;	
	int		IsFieldTripletKey(int) const;
	
	int		GetColumnByName(const string& inName) const;

};

bool	ReadVPFTableHeader(MFMemFile * inFile, VPF_TableDef& outDef);
void	DumpVPFTableHeader(const VPF_TableDef& inDef);
void	DumpVPFTable(MFMemFile * inFile, const VPF_TableDef& inDef);

class	VPFTableIterator {
public:
	VPFTableIterator(MFMemFile * inFile, const VPF_TableDef& inDef);
	
	bool	Done(void);
	void	Next(void);
	bool	Error(void);
	
	int		GetFieldCount(void);

	// The following coersions are done for you:
	// - All numeric types are interchangeable.
	// - All forms of chars (variable and fixed) are handled as strings.
	// - Coord triples and pairs can be swapped; the extra Z field is added or truncated.
	// - Dates appear as fixed-length 20 char strings
	// What is not done for you:
	// - Array vs. single must be precise for any non-string type.

	bool	GetNthFieldAsString(int, string&);

	bool	GetNthFieldAsInt(int, int&);
	bool	GetNthFieldAsDouble(int, double&);
	bool	GetNthFieldAsCoordPair(int, Point2&);
	bool	GetNthFieldAsCoordTriple(int, Point3&);

	bool	GetNthFieldAsIntArray(int, vector<int>&);
	bool	GetNthFieldAsDoubleArray(int, vector<double>&);
	bool	GetNthFieldAsCoordPairArray(int, vector<Point2>&);
	bool	GetNthFieldAsCoordTripleArray(int, vector<Point3>&);
	
	bool	GetNthFieldAsTripletKey(int, VPF_TripletKey&);
	
private:

	void	ParseCurrent(void);

	MFMemFile *		mFile;				// Mem in file
	const char *	mCurrentRecord;		// Base addr of current record
	const char *	mNextRecord;		// Base addr of next record, defines our length
	vector<int>		mFieldOffsets;		// Offsets in cur record of all fields (in bytes)
	VPF_TableDef	mDef;				// Def of our table
};

#endif

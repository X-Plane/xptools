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
#ifndef XCARBONEUTILS_H
#define XCARBONEUTILS_H

// XCarBone - a single entry in the bone table, describing
// the movement of one element of static geometry.

struct	XCarBone {

	// Parameters are stored in key tables, providing
	// values over time from t = 0.0 to 1.0.
	typedef map<double, double>	KeyTable;

	int				parent;			// Table index # of our parent
	vector<int>		child_cache;	// Table index # of our children, a cache for speed
	void *			ref;			// Reference to external data, useful when working in AC3D (holds an ACObject)
	string			name;			// Name of the file that contains this part
	int				part_type;		// Enum for the type of part, used in sim
	int				animation_type;	// Enum for the sim param animation follows
	double			preview_time;	// The time we are showing now, used to transform geometry.

	KeyTable		xyz[3];			// Keyable translations in x, y, z
	KeyTable		rot[3];			// Keyable rotations around phi, the and psi (Eulers)

};

// XCarBones - a collection of car bones with index and access utilities

struct	XCarBones {

	vector<XCarBone>	bones;		// A table of our bones
	map<void *, int>	index;		// A reverse index for finding the entry for an ACObject

	bool		IsValid(void *);
	void *		GetParent(void *);
	void		GetChildren(void *, vector<void *>&);
	string		GetBoneName(void *);

	void		GetDeboneMatrix(void *, double[16]);
	void		GetReboneMatrix(void *, double[16]);

	void		RebuildChildCache(void);
	void		RebuildIndex(void);

};

// Given a key table, get value at time.  Handles off-the-end cases and tweening.
double	GetValueForTime(double inTime, const XCarBone::KeyTable& inTable);

// Test routines to write the bone files in our temp file format
bool	ReadBonesFromFile(const char * inFileName, XCarBones& outBones);
bool	WriteBonesToFile(const char * inFileName, XCarBones& outBones);

#endif

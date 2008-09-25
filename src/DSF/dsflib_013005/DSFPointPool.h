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
#ifndef DSFPOINTPOOL_H
#define DSFPOINTPOOL_H

#include <vector>
#if LIN
#include <map>
using namespace std;
#endif

/* A tuple - an N dimensional coordinate.  Real
 * handy because it can be used for any kind of DSF coordinate. */

/* Of course, in practice, variable sized arrays are way too
 * expensive to manipulate, so we're fixed sized.  Right now
 * 10 planes covers the worst case - XYZ, norm, STx2 + A */
 #define	MAX_TUPLE_LEN 10

class	DSFTuple {
public:

	DSFTuple();
	DSFTuple(int planes);
	DSFTuple(const DSFTuple& rhs);
	DSFTuple(const double * values, int length);
	~DSFTuple();

	DSFTuple& operator=(const DSFTuple& rhs);
	bool operator==(const DSFTuple& rhs) const;
	bool operator< (const DSFTuple& rhs) const;

	DSFTuple& operator += (const DSFTuple& rhs);
	DSFTuple& operator -= (const DSFTuple& rhs);
	DSFTuple& operator *= (const DSFTuple& rhs);
	DSFTuple& operator /= (const DSFTuple& rhs);

	DSFTuple operator+ (const DSFTuple& rhs) const;
	DSFTuple operator- (const DSFTuple& rhs) const;
	DSFTuple operator* (const DSFTuple& rhs) const;
	DSFTuple operator/ (const DSFTuple& rhs) const;

	bool in_range(const DSFTuple& offset, const DSFTuple& scale) const;
	bool encode(const DSFTuple& offset, const DSFTuple& scale);
	bool encode32(const DSFTuple& offset, const DSFTuple& scale);

	inline int size() const { return mLen; }
	inline double operator[](int n) const { return mData[n]; }
	inline double& operator[](int n) { return mData[n]; }
	inline double * begin() { return mData; }
	inline const double * begin() const { return mData; }
	inline double * end() { return mData+mLen; }
	inline const double * end() const { return mData+mLen; }
	void push_back(double v);
	void insert(double * ptr, double v);

	void dump(void);
	void dumphex(void);

private:

	int		mLen;
	double	mData[MAX_TUPLE_LEN];

};

typedef	vector<DSFTuple>			DSFTupleVector;
typedef vector<DSFTupleVector>		DSFTupleVectorVector;

/* A shared point poo.  Every point is pooled, and the
 * points are sorted spatially.  The shared point pool
 * is really N sub-point-pools, so each point ends up
 * with a pair of indices. */

typedef	pair<int, int>	DSFPointPoolLoc;
typedef vector<DSFPointPoolLoc>	DSFPointPoolLocVector;

class	DSFSharedPointPool {
public:

	DSFSharedPointPool();
	DSFSharedPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);


	void			AddPool(DSFTuple& minFrac, DSFTuple& maxFrac);

	// This returns true if the set of points can be kept as a run...
	// it tests only whether the points span subpools, and can be
	// run before any points are accepted.
	bool			CanBeContiguous(const DSFTupleVector& inPoints);

	// This routine accepts a run as a contiguous set in one pool, and
	// returns the pool and index, or -1, -1 if any of the points are
	// already in one of the point pools (and thus it should be shared).
	// You can also limit these to a single pool if you have a pool #
	// that you know is good (from above).
	DSFPointPoolLoc	AcceptContiguous(const DSFTupleVector& inPoints);
	DSFPointPoolLoc	AcceptContiguousPool(int pool, const DSFTupleVector& inPoints);
	// This routine accepts a single point, sharing if possible.
	DSFPointPoolLoc	AcceptShared(const DSFTuple& inPoint);

	void			ProcessPoints(void);
	int				MapPoolNumber(int);	// From full to used pool #s

	int				WritePoolAtoms(FILE * fi, int id);
	int				WriteScaleAtoms(FILE * fi, int id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	struct	SharedSubPool {

		DSFTuple					mOffset;
		DSFTuple					mScale;

		DSFTupleVector			mPoints;			// These are our points
		map<DSFTuple, int>			mPointsIndex;		// This is used to see if we already have a point.

	};

	vector<SharedSubPool>		mPools;
	vector<int>					mUsageMapping;


};

class	DSFContiguousPointPool {
public:

	DSFContiguousPointPool();
	DSFContiguousPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);

	void			AddPool(DSFTuple& minFrac, DSFTuple& maxFrac);
	DSFPointPoolLoc	AccumulatePoint(const DSFTuple& inPoint);
	DSFPointPoolLoc	AccumulatePoints(const DSFTupleVector& inPoints);
	void			ProcessPoints(void);
	int				MapPoolNumber(int);	// From full to used pool #s

	int				WritePoolAtoms(FILE * fi, int id);
	int				WriteScaleAtoms(FILE * fi, int id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	struct	ContiguousSubPool {

		DSFTuple					mOffset;
		DSFTuple					mScale;

		DSFTupleVector			mPoints;

	};

	vector<ContiguousSubPool>	mPools;
	vector<int>					mUsageMapping;
};


class	DSF32BitPointPool {
public:

	DSF32BitPointPool();
	DSF32BitPointPool(
				const DSFTuple& 		min,
				const DSFTuple& 		max);
	void			SetRange(
				const DSFTuple& 		min,
				const DSFTuple& 		max);

	int				CountShared(const DSFTupleVector& inPoints);
	DSFPointPoolLoc	AcceptContiguous(const DSFTupleVector& inPoints);
	DSFPointPoolLoc	AcceptShared(const DSFTuple& inPoint);

	int				WritePoolAtoms(FILE * fi, int id);
	int				WriteScaleAtoms(FILE * fi, int id);

private:

	DSFTuple			mMin;
	DSFTuple			mMax;

	DSFTuple					mOffset;
	DSFTuple					mScale;

	DSFTupleVector				mPoints;			// These are our points
	map<DSFTuple, int>			mPointsIndex;		// This is used to see if we already have a point.

};

#endif

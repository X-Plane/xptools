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

#include "CoverageFinder.h"
#include "AssertUtils.h"
#include <stdio.h>

CoverageFinder::CoverageFinder(int inNumAxes)
{
	mIsEnum.resize(inNumAxes, false);
	mFloatValues.resize(inNumAxes);
	mEnumValues.resize(inNumAxes);
	mAxisName.resize(inNumAxes, "Unnamed");
	mAxisWidth.resize(inNumAxes, 3);
}

CoverageFinder::~CoverageFinder()
{
}

void	CoverageFinder::NameAxis(int inAxis, const string& inName)
{
	mAxisName[inAxis] = inName;
	mAxisWidth[inAxis] = max(mAxisWidth[inAxis], (int) inName.size()+1);
}

void	CoverageFinder::AddAxisRange(int inAxis, float inMin, float inMax)
{
	mAxisWidth[inAxis] = max(mAxisWidth[inAxis], 14);
	DebugAssert(mEnumValues[inAxis].empty());
	mIsEnum[inAxis] = false;
	mFloatValues[inAxis].insert(inMin);
	mFloatValues[inAxis].insert(inMax);
}

void	CoverageFinder::AddAxisEnum(int inAxis, int inEnum, const string& inName)
{
	mAxisWidth[inAxis] = max(mAxisWidth[inAxis], (int) inName.size()+1);
	DebugAssert(mFloatValues[inAxis].empty());
	mIsEnum[inAxis] = true;
	mEnumValues[inAxis].insert(map<int, string>::value_type(inEnum, inName));
}

void	CoverageFinder::FinishAxes(void)
{
	int slots = 1;
	for (int a = 0 ; a < mIsEnum.size(); ++a)
	{
		slots *= AxisSize(a);
	}
	printf("Have %d slots.\n", slots);
	mCoverage.resize(slots, false);
}

void	CoverageFinder::StartRule(void)
{
	mRule.resize(mIsEnum.size());
	for (int a = 0; a < mIsEnum.size(); ++a)
	{
		mRule[a].resize(AxisSize(a), false);
	}
}

void	CoverageFinder::AddRuleRange(int inAxis, float inMin, float inMax)
{
	int s1 = RangeToIndex(inAxis, inMin);
	int s2 = RangeToIndex(inAxis, inMax);
	for (int s = s1; s < s2; ++s)
		mRule[inAxis][s] = true;
}

void	CoverageFinder::AddRuleEnum(int inAxis, int inEnum)
{
	int s = EnumToIndex(inAxis, inEnum);
		mRule[inAxis][s] = true;

}

void	CoverageFinder::AddRuleAny(int inAxis)
{
	int s1 = AxisSize(inAxis);
	for (int s = 0; s < s1; ++s)
		mRule[inAxis][s] = true;
}

void	CoverageFinder::EndRule(bool keep)
{
	if (keep)
	for (int s = 0; s < mCoverage.size(); ++s)
	{
		vector<int> indices(mIsEnum.size());
		FindIndicesFromPt(s, indices);
		bool has = true;
		for (int i = 0; i < indices.size(); ++i)
		{
			if (!mRule[i][indices[i]])
			{
				has = false;
				break;
			}
		}
		if (has)
			mCoverage[s] = true;
	}
	mRule.clear();
}


void	CoverageFinder::OutputGaps(void)
{
	printf("         ");
	for (int i = 0; i < mAxisName.size(); ++i)
	{
		printf("%-*s ", mAxisWidth[i], mAxisName[i].c_str());
	}
	printf("\n");
	for (int s = 0; s < mCoverage.size(); ++s)
	if (!mCoverage[s])
	{
		printf("%4d %c   ", s, mCoverage[s] ? '*' : '.');
		vector<int>	indices;
		FindIndicesFromPt(s, indices);
		for (int i = 0; i < indices.size(); ++i)
		{
			if (mIsEnum[i])
			{
				string e = IndexToEnumString(i, indices[i]);
				printf("%-*s ", mAxisWidth[i], e.c_str());
			} else {
				float r1 = IndexToRange(i, indices[i]);
				float r2 = IndexToRange(i, indices[i]+1);
				printf("%-6.1f %-6.1f ", r1, r2);
				for (int n = 13; n < mAxisWidth[i]; ++n)
					printf(" ");
			}
		}
		printf("\n");
	}
}


int		CoverageFinder::EnumToIndex(int inAxis, int inEnum)
{
	int ctr = 0;
	for (map<int, string>::iterator e = mEnumValues[inAxis].begin();
		e != mEnumValues[inAxis].end(); ++e, ++ctr)
	{
		if (e->first == inEnum)
			return ctr;
	}
	DebugAssert(!"Enum not found.");
	return ctr;
}

int		CoverageFinder::RangeToIndex(int inAxis, float inVal)
{
	int ctr = 0;
	for (set<float>::iterator e = mFloatValues[inAxis].begin();
		e != mFloatValues[inAxis].end(); ++e, ++ctr)
	{
		if (*e == inVal)
			return ctr;
	}
	DebugAssert(!"Float not found.");
	return ctr;
}

int CoverageFinder::IndexToEnum(int inAxis, int inIndex)
{
	map<int, string>::iterator e = mEnumValues[inAxis].begin();
	while (inIndex--)
		++e;
	return e->first;
}

string CoverageFinder::IndexToEnumString(int inAxis, int inIndex)
{
	map<int, string>::iterator e = mEnumValues[inAxis].begin();
	while (inIndex--)
		++e;
	return e->second;
}

float CoverageFinder::IndexToRange(int inAxis, int inIndex)
{
	set<float>::iterator e = mFloatValues[inAxis].begin();
	while (inIndex--)
		++e;
	return *e;
}



int		CoverageFinder::AxisSize(int a)
{
	if (mIsEnum[a]) {
		if (mEnumValues[a].empty())
			return 1;
		return mEnumValues[a].size();
	} else {
		if (mFloatValues[a].size() < 2) return 1;
		return mFloatValues[a].size()-1;
	}
}

int		CoverageFinder::AxisSizeCume(int a)
{
	int n = 1;
	for (int i = a+1; i < mIsEnum.size(); ++i)
		n *= AxisSize(i);
	return n;
}

void	CoverageFinder::FindIndicesFromPt(int p, vector<int>& indices)
{
	indices.resize(mIsEnum.size());
	for (int i = 0; i < mIsEnum.size(); ++i)
	{
		int c = AxisSizeCume(i);
		indices[i] = p / c;
		p = p % c;
	}
}

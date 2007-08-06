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

#ifndef COVERAGEFINDER_H
#define COVERAGEFINDER_H

#include <set>
#include <map>
#include <vector>
#include <string>
using std::string;
using std::vector;
using std::set;
using std::map;

class CoverageFinder {
public:

			CoverageFinder(int inNumAxes);
			~CoverageFinder();
			
	void	NameAxis(int inAxis, const string& inName);
	void	AddAxisRange(int inAxis, float inMin, float inMax);
	void	AddAxisEnum(int inAxis, int inEnum, const string& inName);
	void	FinishAxes(void);
	
	void	StartRule(void);
	void	AddRuleRange(int inAxis, float inMin, float inMax);
	void	AddRuleEnum(int inAxis, int inEnum);
	void	AddRuleAny(int inAxis);
	void	EndRule(bool keep);
	
	void	OutputGaps(void);
	
private:

	int		EnumToIndex(int inAxis, int inEnum);
	int		RangeToIndex(int inAxis, float inVal);
	int		IndexToEnum(int inAxis, int inIndex);
	string	IndexToEnumString(int inAxis, int inIndex);
	float	IndexToRange(int inAxis, int inIndex);
	int		AxisSize(int a);
	int		AxisSizeCume(int a);
	void	FindIndicesFromPt(int p, vector<int>& indices);
	
	vector<bool>				mIsEnum;
	vector<set<float> >			mFloatValues;
	vector<map<int, string> >	mEnumValues;
	
	vector<bool>				mCoverage;
	vector<vector<bool> >		mRule;
	
	vector<string>				mAxisName;
	vector<int>					mAxisWidth;
	
};


#endif /* COVERAGEFINDER_H */

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

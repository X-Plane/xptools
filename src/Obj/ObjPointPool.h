#ifndef OBJPOINTPOOL_H
#define OBJPOINTPOOL_H

#include <vector>
#include <map>

using std::map;
using std::vector;

template <class T>
struct lex_compare_vector : public binary_function<vector<T>, vector<T>, bool> {

	bool operator()(const vector<T>& lhs, const vector<T>& rhs) const {
		return lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
};

class ObjPointPool {
public:
	ObjPointPool();
	~ObjPointPool();

	void	clear(int depth);	// Set zero points and number of floats per pt
	void	resize(int pts);	// Set a lot of pts

	int		accumulate(const float pt[]);	// Add a pt, extend if needed
	int		append(const float pt[]);		// Add a pt to the end
	void	set(int n, float pt[]);			// Set an existing pt
	
	int		count(void) const;
	float *	get(int index);
	const float *	get(int index) const;

private:

	typedef	vector<float>									key_type;
	typedef map<key_type, int, lex_compare_vector<float> >	index_type;

	vector<float>	mData;
	index_type		mIndex;
	int				mDepth;

};

#endif

/* 
 * Copyright (c) 2005, Laminar Research.
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

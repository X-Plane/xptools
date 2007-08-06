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

#ifndef RLEREGION_H
#define RLEREGION_H

#include <vector>
using std::vector;

class RLERegion {
public:

	RLERegion();
	RLERegion(int x1, int y1, int x2, int y2);
	RLERegion(const RLERegion& rhs);

	// OPERATORS
		
	RLERegion& operator=(const RLERegion& rhs);
	bool operator==(const RLERegion& rhs) const;
	bool operator!=(const RLERegion& rhs) const;

	// Union
	RLERegion& operator+=(const RLERegion& rhs);
	RLERegion operator+(const RLERegion& rhs) const { RLERegion tmp(*this); tmp += rhs; return tmp; }
	// Assymetric Difference
	RLERegion& operator-=(const RLERegion& rhs);
	RLERegion operator-(const RLERegion& rhs) const { RLERegion tmp(*this); tmp += rhs; return tmp; }
	// Intersection
	RLERegion& operator*=(const RLERegion& rhs);
	RLERegion operator*(const RLERegion& rhs) const { RLERegion tmp(*this); tmp += rhs; return tmp; }

	// MANIPULATORS
		
	void	border(RLERegion& outBorder);
	void	set_rect(int x1, int y1, int x2, int y2);
	void	clear(void);
	void	offset(int x, int y);
	void	insert_pt(int x, int y) { RLERegion tmp(x,y,x+1,y+1); (*this) += tmp; }
	void	remove_pt(int x, int y) { RLERegion tmp(x,y,x+1,y+1); (*this) -= tmp; }

	// ACCESSORS
	
	bool	empty(void) const;
	bool	is_rect(void) const;

	int		left(void) const { return x1_; }
	int		right(void) const { return x2_; }
	int		bottom(void) const { return y1_; }
	int		top(void) const { return y2_; }

	int		width(void) const { return x2_ - x1_; }
	int		height(void) const { return y2_ - y1_; }

	friend	class	RLERegionScanner;
	friend	class	RLERegionDualScanner;
private:

	void	trim(void);
	void	extend_x(int x1, int x2);
	void	extend_y(int y1, int y2);

	typedef vector<int>	run;
	typedef vector<run>	runarray;

	int			x1_;
	int			x2_;
	int			y1_;
	int			y2_;
	runarray	runs_;

};
	
class	RLERegionScanner {
public:
	RLERegionScanner(const RLERegion& region);
	
	void		reset(void);
	void		next_row(void);
	void		next_run(void);
	bool		done(void) const;
	
	int			cur_row(void) const { return y_ + region_.y1_; }
	int			cur_run_start(void) const { return x_ + region_.x1_; }
	int			cur_run_stop (void) const { return x_ + region_.x1_ + region_.runs_[y_][r_]; }

private:
	const RLERegion&	region_;
	int					y_;
	int					r_;
	int					x_;
	
	RLERegionScanner();
	RLERegionScanner(const RLERegionScanner&);
	RLERegionScanner& operator=(const RLERegionScanner&);

};

#endif
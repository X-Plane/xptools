#include "RLERegion.h"
#include <utility>

using std::min;
using std::max;
	
class RLERegionDualScanner {
	const RLERegion& rgn1_;
	const RLERegion& rgn2_;
	int x1_;
	int x2_;
	int r1_;
	int r2_;
	int p1_;
	int p2_;
	const RLERegion::run * run1_;
	const RLERegion::run * run2_;
		
public:
	RLERegionDualScanner(const RLERegion& r1, const RLERegion& r2) : rgn1_(r1), rgn2_(r2) { }
	void set_rows(int y1, int y2) { 
		run1_ = &rgn1_.runs_[y1]; 
		run2_ = &rgn2_.runs_[y2];
		r1_ = r2_ = 0;
		x1_ = min(rgn1_.x1_, rgn2_.x1_);
		p1_ = rgn1_.x1_;
		p2_ = rgn2_.x1_;
		x2_ = min(rgn1_.x1_ + run1_->at(0), rgn2_.x1_ + run2_->at(0));				
	}
	bool done(void) { return x1_ >= rgn1_.x2_ && x1_ >= rgn2_.x2_; }
	void next(void)
	{
		int rs = max(rgn1_.x2_, rgn2_.x2_);
		
		// This warrants some explanation: if we are an even-number of runs,
		// our run ends with filled area.  In that case, we need to return the maximum
		// right bound if we are beyond the end of our runs, so that the other guy
		// can go.  If we are an odd number, our last right side is white space; basically
		// ignore this and return the max right so that we don't stop the runs from the transition
		// from our white space to the space beyond our white space.
		int n1 = (r1_ >= (run1_->size() & ~1)) ? rs : (p1_ + run1_->at(r1_));
		int n2 = (r2_ >= (run2_->size() & ~1)) ? rs : (p2_ + run2_->at(r2_));
		
		if (n1 < n2)
		{
			x1_ = n1;
			++r1_;
			p1_ = n1;
			n1 = (r1_ >= (run1_->size() & ~1)) ? rs : (p1_ + run1_->at(r1_));
			n2 = (r2_ >= (run2_->size() & ~1)) ? rs : (p2_ + run2_->at(r2_));
			x2_ = min(n1, n2);
		}
		else if (n1 > n2)
		{
			x1_ = n2;
			++r2_;
			p2_ = n2;
			n1 = (r1_ >= (run1_->size() & ~1)) ? rs : (p1_ + run1_->at(r1_));
			n2 = (r2_ >= (run2_->size() & ~1)) ? rs : (p2_ + run2_->at(r2_));
			x2_ = min(n1, n2);
		} else {
			++r1_;
			++r2_;
			p1_ = p2_ = x1_ = n1;
			n1 = (r1_ >= (run1_->size() & ~1)) ? rs : (p1_ + run1_->at(r1_));
			n2 = (r2_ >= (run2_->size() & ~1)) ? rs : (p2_ + run2_->at(r2_));
			x2_ = min(n1, n2);
		}
	}
	
	bool first_on(void) { return r1_ < run1_->size() && (r1_ % 2); }
	bool second_on(void) { return r2_ < run2_->size() && (r2_ % 2); }
	int  run_start(void) { return x1_; }
	int	 run_stop(void) { return x2_; }
	int  run_length(void) { return x2_ - x1_; }
};	


RLERegion::RLERegion() : x1_(0), x2_(0), y1_(0), y2_(0)
{
}

RLERegion::RLERegion(int x1, int y1, int x2, int y2) : x1_(0), x2_(0), y1_(0), y2_(0)
{
	set_rect(x1, y1, x2, y2);
}

RLERegion::RLERegion(const RLERegion& rhs) : 
	x1_(rhs.x1_),	x2_(rhs.x2_),
	y1_(rhs.y1_),	y2_(rhs.y2_),
	runs_(rhs.runs_)
{
}

RLERegion& RLERegion::operator=(const RLERegion& rhs)
{
	x1_ = rhs.x1_;
	y1_ = rhs.y1_;
	x2_ = rhs.x2_;
	y2_ = rhs.y2_;
	runs_ = rhs.runs_;
	return *this;
}

bool RLERegion::operator==(const RLERegion& rhs) const
{
	return  x1_ == rhs.x1_ && x2_ == rhs.x2_ &&
			y1_ == rhs.y1_ && y2_ == rhs.y2_ &&
			runs_ == rhs.runs_;
}

bool RLERegion::operator!=(const RLERegion& rhs) const
{
	return  x1_ != rhs.x1_ || x2_ != rhs.x2_ ||
			y1_ != rhs.y1_ || y2_ != rhs.y2_ ||
			runs_ != rhs.runs_;
}

RLERegion& RLERegion::operator+=(const RLERegion& rhs)
{
	extend_y(rhs.y1_, rhs.y2_);
	extend_x(rhs.x1_, rhs.x2_);
	
	int offset = rhs.y1_ - y1_;
	int hang = y2_ - rhs.y2_;
	
	runarray	new_runs;
	if (offset > 0)
		new_runs.insert(new_runs.end(), runs_.begin(), runs_.begin() + offset);

	
	RLERegionDualScanner	scanner(*this, rhs);
	for (int y = 0; y < rhs.runs_.size(); ++y)
	{
		scanner.set_rows(y + offset, y);
		new_runs.push_back(run(1, 0));
		while (!scanner.done())
		{
			if (scanner.first_on() || scanner.second_on())
			{
				if (new_runs.back().size() % 2)
					new_runs.back().push_back(scanner.run_length());
				else
					new_runs.back().back() += scanner.run_length();
			} else {
				if (new_runs.back().size() % 2)
					new_runs.back().back() += scanner.run_length();
				else
					new_runs.back().push_back(scanner.run_length());
			}
			
			scanner.next();
		}
	}

	if (hang > 0)
		new_runs.insert(new_runs.end(), runs_.end() - hang, runs_.end());
	
	swap(runs_, new_runs);
	trim();
	
	return *this;
}

RLERegion& RLERegion::operator-=(const RLERegion& rhs)
{
	extend_y(rhs.y1_, rhs.y2_);
	extend_x(rhs.x1_, rhs.x2_);
	
	int offset = rhs.y1_ - y1_;
	int hang = y2_ - rhs.y2_;
	
	runarray	new_runs;
	if (offset > 0)
		new_runs.insert(new_runs.end(), runs_.begin(), runs_.begin() + offset);

	
	RLERegionDualScanner	scanner(*this, rhs);
	for (int y = 0; y < rhs.runs_.size(); ++y)
	{
		scanner.set_rows(y + offset, y);
		new_runs.push_back(run(1, 0));
		while (!scanner.done())
		{
			if (scanner.first_on() && !scanner.second_on())
			{
				if (new_runs.back().size() % 2)
					new_runs.back().push_back(scanner.run_length());
				else
					new_runs.back().back() += scanner.run_length();
			} else {
				if (new_runs.back().size() % 2)
					new_runs.back().back() += scanner.run_length();
				else
					new_runs.back().push_back(scanner.run_length());
			}
			
			scanner.next();
		}
	}

	if (hang > 0)
		new_runs.insert(new_runs.end(), runs_.end() - hang, runs_.end());
	
	swap(runs_, new_runs);
	trim();
	
	return *this;
}

RLERegion& RLERegion::operator*=(const RLERegion& rhs)
{
	extend_y(rhs.y1_, rhs.y2_);
	extend_x(rhs.x1_, rhs.x2_);
	
	int offset = rhs.y1_ - y1_;
	int hang = y2_ - rhs.y2_;
	
	runarray	new_runs;
	if (offset > 0)
		new_runs.insert(new_runs.end(), offset, run(1, x2_ - x1_));

	
	RLERegionDualScanner	scanner(*this, rhs);
	for (int y = 0; y < rhs.runs_.size(); ++y)
	{
		scanner.set_rows(y + offset, y);
		new_runs.push_back(run(1, 0));
		while (!scanner.done())
		{
			if (scanner.first_on() && scanner.second_on())
			{
				if (new_runs.back().size() % 2)
					new_runs.back().push_back(scanner.run_length());
				else
					new_runs.back().back() += scanner.run_length();
			} else {
				if (new_runs.back().size() % 2)
					new_runs.back().back() += scanner.run_length();
				else
					new_runs.back().push_back(scanner.run_length());
			}
			
			scanner.next();
		}
	}

	if (hang > 0)
		new_runs.insert(new_runs.end(), hang, run(1, x2_ - x1_));
	
	swap(runs_, new_runs);
	trim();
	
	return *this;
}


// MANIPULATORS
	
void	RLERegion::border(RLERegion& outBorder)
{
	static int dirs_x[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dirs_y[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	outBorder = *this;
	for (int n = 0; n < 8; ++n)
	{
		RLERegion foo(*this);
		foo.offset(dirs_x[n],dirs_y[n]);
		outBorder += foo;
	}
	outBorder -= *this;	
}

void	RLERegion::set_rect(int x1, int y1, int x2, int y2)
{
	x1_ = x1; x2_ = x2; y1_ = y1; y2_ = y2;
	runs_.resize(y2-y1);
	for (int y = 0; y < runs_.size(); ++y)
	{
		runs_[y].resize(2);
		runs_[y][0] = 0;
		runs_[y][1] = x2 - x1;
	}
}

void	RLERegion::clear(void)
{
	set_rect(0, 0, 0, 0);
}

void	RLERegion::offset(int x, int y)
{
	x1_ += x;
	x2_ += x;
	y1_ += y;
	y2_ += y;
}

void	RLERegion::trim(void)
{
	int ctr;
		
	// TRIM OFF TOP
	ctr = runs_.size();
	while (ctr > 0 && runs_[ctr - 1].size() < 2)
		--ctr;
	y2_ = y1_ + ctr;
	runs_.erase(runs_.begin() + ctr, runs_.end());

	// TRIM OFF BOTTOM
	ctr = 0;
	while (ctr < runs_.size() && runs_[ctr].size() < 2)
		++ctr;
	if (ctr != 0)
	{
		y1_ += ctr;
		runs_.erase(runs_.begin(), runs_.begin() + ctr);
	}
}

void	RLERegion::extend_y(int y1, int y2)
{
	if (y1 < y1_)
	{
		runs_.insert(runs_.begin(), y1_ - y1, run(1, x2_ - x1_));
		y1_ = y1;
	}
	if (y2 > y2_)
	{
		runs_.insert(runs_.end(), y2 - y2_, run(1, x2_ - x1_));
		y2_ = y2;
	}
}

void	RLERegion::extend_x(int x1, int x2)
{
	int y, grow_x = x1_ - x1;
	if (grow_x > 0)
	{
		for (y = 0; y < runs_.size(); ++y)
		{
			runs_[y][0] += grow_x;
		}
		x1_ = x1;
	}
	grow_x = x2 - x2_;
	if (grow_x > 0)
	{
		for (y = 0; y < runs_.size(); ++y)
		{
			if (runs_[y].size() % 2)
				runs_[y].back() += grow_x;
			else
				runs_[y].push_back(grow_x);
		}
		x2_ = x2;
	}
}

// ACCESSORS
	

bool	RLERegion::empty(void) const
{	
	for (int y = 0; y < runs_.size(); ++y)
	for (int x = 1; x < runs_[y].size(); x += 2)
		if (runs_[y][x] > 0)
			return false;
	return true;
}

bool	RLERegion::is_rect(void) const
{
	for (int y = 0; y < runs_.size(); ++y)
	for (int x = 0; x < runs_[y].size(); x += 2)
		if (runs_[y][x] > 0)
			return false;
	return true;
}

// RLERegionScanner

RLERegionScanner::RLERegionScanner(const RLERegion& region) : region_(region)
{
	reset();
}
	
void		RLERegionScanner::reset(void)
{
	y_ = 0;
	while (1)
	{
		while (y_ < region_.runs_.size() && region_.runs_[y_].size() < 2)
			++y_;

		if (y_ == region_.runs_.size())
			break;

		if (region_.runs_[y_].size() == 2 && region_.runs_[y_][1] == 0)
			continue;
		
		r_ = 1;
		x_ = region_.runs_[y_][0];
		break;
	}		
}

void		RLERegionScanner::next_row(void)
{
	++y_;
	while (1)
	{
		while (y_ < region_.runs_.size() && region_.runs_[y_].size() < 2)
			++y_;

		if (y_ == region_.runs_.size())
			break;

		if (region_.runs_[y_].size() == 2 && region_.runs_[y_][1] == 0)
			continue;
		
		r_ = 1;
		x_ = region_.runs_[y_][0];
		break;
	}			
}

void		RLERegionScanner::next_run(void)
{
	x_ += region_.runs_[y_][r_];
	++r_;
	if (r_ == region_.runs_[y_].size())
	{
		next_row();
		return;
	}
	x_ += region_.runs_[y_][r_];
	++r_;
	if (r_ == region_.runs_[y_].size())
	{
		next_row();
		return;
	}
}

bool		RLERegionScanner::done(void) const
{
	return y_ == region_.runs_.size();
}

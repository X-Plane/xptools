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

#ifndef WED_RAMPPOSITION_H
#define WED_RAMPPOSITION_H

#include "WED_GISPoint_Heading.h"

struct	AptGate_t;

class	WED_RampPosition : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_RampPosition)

public:

	void	SetType(int		ramp_type);
	void	SetEquipment(const set<int>&	et);
	void	SetWidth(int		width);
	void	SetRampOperationType(int ait);
	void	SetAirlines(const string& airlines);

	string  GetAirlines() const;
	int		GetType() const;
	int		GetWidth() const;
	void	GetEquipment(set<int>& out_eq) const;
	
	static string CorrectAirlinesString(const string &a);

	void	Import(const AptGate_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptGate_t& x) const;

	virtual const char *	HumanReadableType(void) const { return "Ramp Start"; }

private:

	WED_PropIntEnum			ramp_type;
	WED_PropIntEnumBitfield	equip_type;
	WED_PropIntEnum			width;
	WED_PropIntEnum			ramp_op_type;
	WED_PropStringText		airlines;

};

#endif /* WED_RAMPPOSITION_H */

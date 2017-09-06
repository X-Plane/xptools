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

#include "WED_ATCFrequency.h"
#include "WED_EnumSystem.h"
#include "AptDefs.h"


DEFINE_PERSISTENT(WED_ATCFrequency)
TRIVIAL_COPY(WED_ATCFrequency, WED_Thing)

WED_ATCFrequency::WED_ATCFrequency(WED_Archive * a, int i) : WED_Thing(a, i),
	freq_type	(this, "Type",		XML_Name("atc_frequency",	"kind"), ATCFrequency, atc_Tower),
	freq		(this, "Frequency",	XML_Name("atc_frequency",	"freq"), 128.8, 6, 2)
{
}

WED_ATCFrequency::~WED_ATCFrequency()
{
}

void	WED_ATCFrequency::Import(const AptATCFreq_t& info, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetName(info.name);
	freq.AssignFrom10Khz(info.freq);
	freq_type = ENUM_Import(ATCFrequency, info.atc_type);
	if (freq_type == -1)
	{
		print_func(ref,"Error importing runway: ATC frequecny code %d is illegal (not a member of type %s).\n", info.atc_type, DOMAIN_Desc(freq_type.domain));
		freq_type = atc_Tower;
	}

}

void	WED_ATCFrequency::Export(		 AptATCFreq_t& info) const
{
	GetName(info.name);
	info.freq = freq.GetAs10Khz();
	info.atc_type = ENUM_Export(freq_type.value);
}


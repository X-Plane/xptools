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

#include "WED_UIMeasurements.h"
#include "GUI_Resources.h"
#include "MemFileUtils.h"

typedef map<string,int>	UI_Measurement_t;

static UI_Measurement_t	sMeasurements;

int		WED_UIMeasurement(const char * measurement)
{
	if (sMeasurements.empty())
	{
		GUI_Resource	res = GUI_LoadResource("ui_measurements.txt");

		MFScanner	s;
		MFS_init(&s, GUI_GetResourceBegin(res), GUI_GetResourceEnd(res));

		while (!MFS_done(&s))
		{
			string token;
			MFS_string(&s,&token);
			if (!token.empty() && token[0] != '#')
			{
				sMeasurements[token] = MFS_int(&s);
			}
			MFS_string_eol(&s,NULL);
		}
		GUI_UnloadResource(res);

	}

	return sMeasurements[string(measurement)];
}

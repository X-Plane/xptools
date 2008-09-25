/*
 * Copyright (c) 2004, Laminar Research.
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
#include "TIGERProcess.h"
#include "TIGERTypes.h"
#define XUTILS_EXCLUDE_MAC_CRAP 1
#include "XUtils.h"

// For maps, the hash key for an env is lat * 360 + lon.

void	ReadTigerIndex(const char * inFileName, TigerMap& outMap)
{
	outMap.clear();
	for (StTextFileScanner	scanner(inFileName, true); !scanner.done(); scanner.next())
	{
		string	line = scanner.get();
		char	fname[32];
		TigerAreaInfo_t	area;

		if (sscanf(line.c_str(), "%s %f %f %f %f",
			fname, &area.lat_min, &area.lat_max, &area.lon_min, &area.lon_max) == 5)
		{
			area.name = fname;
			int	yMin = floor(area.lat_min);
			int yMax = ceil(area.lat_max);
			int xMin = floor(area.lon_min);
			int xMax = ceil(area.lon_max);

			for (int y = yMin; y < yMax; ++y)
			for (int x = xMin; x < xMax; ++x)
			{
				long hash = y * 360 + x;
				outMap[hash].push_back(area);
			}
		}
	}
}

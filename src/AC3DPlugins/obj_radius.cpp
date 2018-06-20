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

#include <stdio.h>

#include "obj_radius.h"
#include "XObjDefs.h"

/* IMPL NOTE: this routine is isolated from obj_export.c because ObjUtils.h includes
 * CompGeomUtils.h which includes CompGeomDefs2.h, whose Point2 definition conflicts
 * with AC3D's plugin header.  */

#if 0
void	GetObjDimensions(const XObj& inObj,
						float	minCoords[3],
						float	maxCoords[3])
{
	minCoords[0] = minCoords[1] = minCoords[2] = 0.0;
	maxCoords[0] = maxCoords[1] = maxCoords[2] = 0.0;

	bool assigned = false;

	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd)
	{
		for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
		{
			if (!assigned)
			{
				for (int n = 0; n < 3; ++n)
				{
					minCoords[n] = st->v[n];
					maxCoords[n] = st->v[n];
				}
				assigned = true;
			}

			for (int n = 0; n < 3; ++n)
			{
				minCoords[n] = min(minCoords[n], st->v[n]);
				maxCoords[n] = max(maxCoords[n], st->v[n]);
			}
		}

		for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
		{
			if (!assigned)
			{
				for (int n = 0; n < 3; ++n)
				{
					minCoords[n] = rgb->v[n];
					maxCoords[n] = rgb->v[n];
				}
				assigned = true;
			}

			for (int n = 0; n < 3; ++n)
			{
				minCoords[n] = min(minCoords[n], rgb->v[n]);
				maxCoords[n] = max(maxCoords[n], rgb->v[n]);
			}
		}
	}
}
#endif

/*
 * GetObjectLesserRadius
 *
 * This routine attempts to calculate the 'lesser radius' of an object, which is
 * loosely defined as the radius of the object in the dimension that is responsible
 * for its vanishing when far away.
 *
 * Our logic is this: given any 2-d view of an angle of the object, the lesser
 * distance of that face defines when you stop seeing it.  Given a long one floor
 * building viewed from the side, it disappears because it is so short, so the
 * lesser radius is the height.
 *
 * This routine simply works on the orthagonal axes...this provides a reasonable
 * calculation and is fast enough to run while loading objects in the sim too.
 *
 * Basically for the front, top and left face we find our lesser dimension.  The
 * largest of those three lesser dimensions is the last constraint on the
 * object vanishing from all angles.
 *
 * One hokey note though: we do divide the lesser dimension from the top by
 * a factor of 7.  This is an 'atmospheric hase fudge factor'.  While a very
 * tall object viewed from a long way away might be visible at 30 miles, in
 * practice things disappear a lot faster when viewed from directly above...
 * there's something about the way that they blend with the terrain, the
 * possibility of clouds or haze in the way, etc. that make them hide.  For
 * example, a 600 foot tall building might be visible from 30 miles away along
 * the ground, but it is not visible from 30 miles out in space!!!
 *
 * Also, many x-plane objects are buildings, which are fairly solid when viewed
 * from the side, but have L-shaped or other sparse floorplans from the top,
 * making them disappear faster from above.  It is very computationally difficult
 * to work out this 'not quite solid' factor, but the fudge factor helps us out.
 *
 * So we divide the axes when viewed from the top by 7...this seems to work well
 * as a fudge factor for now.
 *
 */
float GetObjectLesserRadius(float mins[3], float maxes[3])
{
	float	difs[3], views[3];
	for (int n = 0; n < 3; ++n)
		difs[n] = maxes[n] - mins[n];

	views[0] = min(difs[1], difs[2]);
	views[1] = min(difs[0], difs[2]) / 7.0;
	views[2] = min(difs[0], difs[1]);

	printf("Dimensions x=%f,y=%f,z=%f\n", difs[0], difs[1], difs[2]);
	printf("Smaller dim from left=%f,top=%f,front=%f (top reduced by 7x for haze)\n",views[0], views[1], views[2]);
	return 0.5 * max(max(views[0], views[1]), views[2]);
}


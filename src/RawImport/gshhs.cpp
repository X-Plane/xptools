/*	$Id$

 *

 * PROGRAM:	gshhs.c

 * AUTHOR:	Paul Wessel (pwessel@hawaii.edu)

 * CREATED:	JAN. 28, 1996

 * PURPOSE:	To extract ASCII data from binary shoreline data

 *		as described in the 1996 Wessel & Smith JGR Data Analysis Note.

 * VERSION:	1.1 (Byte flipping added)

 *		1.2 18-MAY-1999:

 *		   Explicit binary open for DOS systems

 *		   POSIX.1 compliant

 *		1.3 08-NOV-1999: Released under GNU GPL

 *		1.4 05-SEPT-2000: Made a GMT supplement; FLIP no longer needed

 *		1.5 14-SEPT-2004: Updated to deal with latest GSHHS database (1.3)

 *

 *	Copyright (c) 1996-2004 by P. Wessel and W. H. F. Smith

 *	See COPYING file for copying and redistribution conditions.

 *

 *	This program is free software; you can redistribute it and/or modify

 *	it under the terms of the GNU General Public License as published by

 *	the Free Software Foundation; version 2 of the License.

 *

 *	This program is distributed in the hope that it will be useful,

 *	but WITHOUT ANY WARRANTY; without even the implied warranty of

 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

 *	GNU General Public License for more details.

 *

 *	Contact info: www.soest.hawaii.edu/pwessel */



#include "gshhs.h"

#include "MapDefs.h"

#include "MapAlgs.h"

#include "ParamDefs.h"

#include <list>



using std::list;



bool	HandleSeg(Pmwx& inMap, double clip[4], const Point2& p1, const Point2& p2)

{

	if (p1.x() < clip[0] && p2.x() < clip[0]) return false;

	if (p1.y() < clip[1] && p2.y() < clip[1]) return false;

	if (p1.x() > clip[2] && p2.x() > clip[2]) return false;

	if (p1.y() > clip[3] && p2.y() > clip[3]) return false;


	CGAL::insert_curve(inMap,Curve_2(Segment_2(Point_2(p1.x(),p1.y()),Point_2(p2.x(),p2.y()))));
//	inMap.insert_edge(p1, p2, NULL, NULL);

	return true;

}



bool ImportGSHHS(const char * inFile, Pmwx& outMap, double clip[4])

{

	printf("Will crop to: %lf,%lf -> %lf, %lf\n", clip[0], clip[1], clip[2], clip[3]);

	outMap.clear();



	double w, e, s, n, area, lon, lat;

	char source;

	FILE	*fp;

	int	k, max_east = 270000000, n_read, flip;

	struct	_POINT p;

	struct GSHHS h;



	if ((fp = fopen (inFile, "rb")) == NULL ) {

		fprintf (stderr, "gshhs:  Could not find file %s.\n", inFile);

		return false;

	}



	n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);

	flip = (! (h.level > 0 && h.level < 5));	/* Take as sign that byte-swabbing is needed */



	// The face stack - GSHHS faces are embedded in containing order, so keep track of who is embedded in what.



	int tot = 0, keep = 0;



	while (n_read == 1) {



		if (flip) {

			h.id = swabi4 ((unsigned int)h.id);

			h.n  = swabi4 ((unsigned int)h.n);

			h.level = swabi4 ((unsigned int)h.level);

			h.west  = swabi4 ((unsigned int)h.west);

			h.east  = swabi4 ((unsigned int)h.east);

			h.south = swabi4 ((unsigned int)h.south);

			h.north = swabi4 ((unsigned int)h.north);

			h.area  = swabi4 ((unsigned int)h.area);

			h.version  = swabi4 ((unsigned int)h.version);

			h.greenwich = swabi2 ((unsigned int)h.greenwich);

			h.source = swabi2 ((unsigned int)h.source);

		}

		w = h.west  * 1.0e-6;	/* Convert from microdegrees to degrees */

		e = h.east  * 1.0e-6;

		s = h.south * 1.0e-6;

		n = h.north * 1.0e-6;

		source = (h.source == 1) ? 'W' : 'C';	/* Either WVS or CIA (WDBII) pedigree */

		area = 0.1 * h.area;			/* Now im km^2 */



//		printf ("P %6d%8d%2d%2c%13.3f%10.5f%10.5f%10.5f%10.5f\n", h.id, h.n, h.level, source, area, w, e, s, n);



//		me->mTerrainType = ((h.level % 2) == 1) ? terrain_Natural : terrain_Water;



		if (h.n < 3)

		{

			fprintf (stderr, "gshhs:  BAD polygon has only two vertices.\n");

			return false;



		}



		Point2	last, now, orig;



		for (k = 0; k < h.n; k++) {



			if (fread ((void *)&p, (size_t)sizeof(struct _POINT), (size_t)1, fp) != 1) {

				fprintf (stderr, "gshhs:  Error reading file %s for polygon %d, point %d.\n", inFile, h.id, k);

				return false;

			}

			if (flip) {

				p.x = swabi4 ((unsigned int)p.x);

				p.y = swabi4 ((unsigned int)p.y);

			}

			lon = (h.greenwich && p.x > max_east) ? p.x * 1.0e-6 - 360.0 : p.x * 1.0e-6;

			lat = p.y * 1.0e-6;

//			printf ("%10.5f%9.5f\n", lon, lat);



			if (now.x() > 180.0) now.x_ -= 360.0;

			if (k > 0)

			{

				if (HandleSeg(outMap, clip, last, now))

					++keep;

			} else {

				orig = now;

			}



			last = now;

			++tot;

		}

		if (HandleSeg(outMap, clip, now, orig))

			++keep;



		max_east = 180000000;	/* Only Eurasiafrica needs 270 */

		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);

	}



	fclose (fp);



	printf("Read %d points, used %d points, map has %d halfedges\n", tot, keep, outMap.number_of_halfedges());

	CropMap(outMap, clip[0], clip[1], clip[2], clip[3], false , NULL);



	return true;

}


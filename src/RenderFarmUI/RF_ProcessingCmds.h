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
#ifndef RF_PROCESSINGCMDS_H
#define RF_PROCESSINGCMDS_H

struct ProcessingPrefs_t {
	int		do_upsample_environment;
	int		do_calc_slope;
	int		do_hydro_correct;
	int		do_hydro_simplify;
	int		do_derive_dems;
	int		do_add_urban_roads;
	int		do_build_roads;
	int		do_airports;
	int		do_zoning;
	int		do_triangulate;
	int		do_assign_landuse;
	int		remove_duplicate_objs;
	int		build_3d_forests;
	int		place_buildings;
};

extern ProcessingPrefs_t	gProcessingCmdPrefs;

void	RegisterProcessingCommands(void);

#endif
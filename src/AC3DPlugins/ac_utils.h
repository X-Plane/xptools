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

#ifndef AC_UTILS_H
#define AC_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TclStubs.h"
#include <ac_plugin.h>
#include <vector>
#include <set>

using std::set;
using std::vector;

#define CAST_CMD(x) reinterpret_cast<void*>(x)

const char * strstrnocase(const char * haystack, const char * needle);

int		is_parent_of(ACObject * parent, ACObject * child);

void	find_all_objects(ACObject * root, vector<ACObject *>& output);
void	find_all_selected_objects(vector<ACObject *>& output);
void	find_all_selected_objects_stable(vector<ACObject *>& output);
void	find_all_selected_objects_flat(vector<ACObject *>& output);
void	find_all_selected_objects_parents(vector<ACObject *>& output);

Surface *	find_single_selected_surface(void);
ACObject *	find_single_selected_object(void);

void	offset_object_textures(ACObject * ob, double dx, double dy, double sx, double sy);

void 	add_tri_to_obj(ACObject * obj, Vertex * v1, Vertex * v2, Vertex * v3);

void	latlonel2xyz(double latlonel[3],
					double latref, double lonref, double cos_scale,
					double xyz[3]);


int 	pull_int_attr_recursive(ACObject * obj, const char * attr, int defv, ACObject * root);

int 	pull_int_attr(ACObject * ob, const char * attr, int * value);
char * 	pull_str_attr(ACObject * ob, const char * attr);

void rotate_surface_y(Surface * surface, float angle, float x_ctr, float z_ctr);
void surface_set_normals(Surface * surface, Point3* rml);
int get_selection_bounds(float minv[3], float maxv[3]);

void move_child_to_head(ACObject * parent, ACObject * child);

Surface * obj_get_first_surf(ACObject * obj);
void	  obj_sel_st(ACObject * obj, float st_bounds[4], vector<Surface *>& out_surf);
void	  surf_sel_st(Surface * obj, float st_bounds[4], vector<Surface *>& out_surf);

void	get_all_used_texes(ACObject * obj, set<int>& out_texes);

void		get_lineage(ACObject * obj, vector<ACObject *>& ancestors);
ACObject *	get_common_parent(const vector<ACObject *>& obj);

#endif

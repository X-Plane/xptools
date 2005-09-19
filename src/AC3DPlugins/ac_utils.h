#ifndef AC_UTILS_H
#define AC_UTILS_H

#include <ac_plugin.h>

void	find_all_objects(ACObject * root, vector<ACObject *>& output);
void	find_all_selected_objects(vector<ACObject *>& output);
void	offset_object_textures(ACObject * ob, double dx, double dy, double sx, double sy);
const 	char * strstrnocase(const char * haystack, const char * needle);

void 	add_tri_to_obj(ACObject * obj, Vertex * v1, Vertex * v2, Vertex * v3);

void	latlonel2xyz(double latlonel[3],
					double latref, double lonref, double cos_scale,
					double xyz[3]);

int pull_int_attr(ACObject * ob, const char * attr, int * value);

#endif
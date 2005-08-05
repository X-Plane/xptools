#ifndef AC_UTILS_H
#define AC_UTILS_H

#include <ac_plugin.h>

void	find_all_objects(ACObject * root, vector<ACObject *>& output);
void	find_all_selected_objects(vector<ACObject *>& output);
void	offset_object_textures(ACObject * ob, double dx, double dy, double sx, double sy);
const char * strstrnocase(const char * haystack, const char * needle);

#endif
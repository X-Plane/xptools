#ifndef AC_UTILS_H
#define AC_UTILS_H

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

void	get_all_used_texes(ACObject * obj, set<int>& out_texes);

void		get_lineage(ACObject * obj, vector<ACObject *>& ancestors);
ACObject *	get_common_parent(const vector<ACObject *>& obj);

#endif

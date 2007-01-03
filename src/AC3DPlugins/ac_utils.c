#include "ac_utils.h"
#include <math.h>
#include <string>
using std::string;
using std::min;
using std::max;

int		is_parent_of(ACObject * parent, ACObject * child)
{
	while(child)
	{
		if (child == parent) return 1;
		child = ac_object_get_parent(child);
	}
	return 0;
}

void	find_all_objects(ACObject * root, vector<ACObject *>& output)
{
	List *kids = ac_object_get_childrenlist(root);

	output.push_back(root);

    for (List * p = kids; p != NULL; p = p->next)
        find_all_objects((ACObject *)p->data, output);
}

void	find_all_selected_objects(vector<ACObject *>& output)
{
	output.clear();
	
	List *	objs = ac_selection_get_objects();
	if (objs)
	{	
		for (List * i = objs; i; i=i->next)
		{
			find_all_objects((ACObject *) i->data, output);
		}
		list_free(&objs);
	}
}

void	find_all_selected_objects_flat(vector<ACObject *>& output)
{
	output.clear();
	
	List *	objs = ac_selection_get_objects();
	if (objs)
	{	
		for (List * i = objs; i; i=i->next)
		{
			output.push_back((ACObject *) i->data);
		}
		list_free(&objs);
	}
}



void	find_all_selected_objects_parents(vector<ACObject *>& output)
{
	vector<ACObject *>	parents;
	output.clear();
	List *	objs = ac_selection_get_objects();
	if (objs)
	{	
		for (List * i = objs; i; i=i->next)
		{
			parents.push_back(ac_object_get_parent((ACObject *) i->data));
		}
		list_free(&objs);
	}
	
	for (int i = 0; i < parents.size(); ++i)
	{
		bool is_top = 1;
		for (int j = 0; j < parents.size(); ++j)
		if (i != j)
		if (is_parent_of(parents[j],parents[i]))
		{
			is_top = false;
			break;
		}
		if (is_top)
			output.push_back(parents[i]);
	}
}

void	offset_object_textures(ACObject * ob, double dx, double dy, double sx, double sy)
{
	List * surfaces = ac_object_get_surfacelist(ob);
	for (List * i = surfaces; i != NULL; i = i->next)
	{
        Surface *s = (Surface *)i->data;
        if (surface_get_type(s) == SURFACE_POLYGON)
        {
			for (List * v = s->vertlist; v != NULL; v = v->next)
			{
				SVertex * sv = (SVertex *) v->data;
				
				sv->tx = sv->tx * sx + dx;
				sv->ty = sv->ty * sy + dy;
			}        	
        }		
	}
}


const char * strstrnocase(const char * haystack, const char * needle)
{
	string	hay(haystack);
	string	ndl(needle);
	for (int i = 0; i < hay.size(); ++i)
		hay[i] = toupper(hay[i]);
	for (int i = 0; i < ndl.size(); ++i)
		ndl[i] = toupper(ndl[i]);
	const char * p = strstr(hay.c_str(), ndl.c_str());
	if (p == NULL) return NULL;
	return haystack + (p - hay.c_str());
}

void add_tri_to_obj(ACObject * obj, Vertex * v1, Vertex * v2, Vertex * v3)
{
	Surface * surf = new_surface();
	surface_set_type(surf, SURFACE_POLYGON);
	
	surface_add_vertex_head(surf, v1, 0, 0);
	surface_add_vertex_head(surf, v2, 0, 0);
	surface_add_vertex_head(surf, v3, 0, 0);
	surface_set_shading(surf, 1);
	object_add_surface_head(obj, surf);
}

void	latlonel2xyz(double latlonel[3],
					double latref, double lonref, double cos_ref,
					double xyz[3])
{
	double lat = latlonel[0] - latref;
	double lon = latlonel[1] - lonref;
	
	xyz[0] = lon * 60.0 * 1852.0 * cos_ref;
	xyz[1] = latlonel[2];
	xyz[2] = lat * 60.0 * 1852.0;
}

int 	pull_int_attr_recursive(ACObject * obj, const char * attr, int defv, ACObject * root)
{
	if (obj == NULL) return defv;
	int r;
	if (pull_int_attr(obj, attr, &r))
		return r;
	ACObject * parent = ac_object_get_parent(obj);
	if (parent == root || parent == NULL) return defv;
	return pull_int_attr_recursive(parent, attr, defv, root);	
}


int pull_int_attr(ACObject * ob, const char * attr, int * value)
{
	char * title = ac_object_get_name(ob);
	char * token = strstr(title, attr);
	if (token == NULL) return 0;
	if (strlen(token) <= strlen(attr))	return 0;
	token += strlen(attr);
	if (value) *value = atoi(token);
	return 1;
}

char * pull_str_attr(ACObject * ob, const char * attr)
{
	char * title = ac_object_get_name(ob);
	char * token = strstr(title, attr);
	if (token == NULL) return NULL;
	if (strlen(token) <= strlen(attr))	return NULL;
	token += strlen(attr);
	return token;
}

void rotate_surface_y(Surface * surface, float angle, float x_ctr, float z_ctr)
{
	float cosr = cos(angle * 3.14159265 / 180.0);
	float sinr = sin(angle * 3.14159265 / 180.0);
	
	for (List * vlist = surface->vertlist; vlist; vlist = vlist->next)
	{
		SVertex * sv = (SVertex *) vlist->data;
		Vertex * v = sv->v;
		float ox = v->x - x_ctr;
		float oz = v->z - z_ctr;
		v->x = x_ctr + cosr * ox + sinr * oz;
		v->z = z_ctr + sinr * ox - cosr * oz;
	}
}

void surface_set_normals(Surface * surface, Point3* nrml)
{
	ac_surface_set_normal(surface, nrml);
	for (List * vlist = surface->vertlist; vlist; vlist = vlist->next)
	{
		SVertex * sv = (SVertex *) vlist->data;
		sv->normal.x = nrml->x;
		sv->normal.y = nrml->y;
		sv->normal.z = nrml->z;
	}
}

int get_selection_bounds(float minv[3], float maxv[3])
{
	int inited = 0;
	List * surf_l = ac_selection_get_whole_surfaces_all();
	
	for (List * iter = surf_l; iter; iter = iter->next)
	{
		Surface * surf = (Surface *) iter->data;
		for (List * vlist = surf->vertlist; vlist; vlist = vlist->next)
		{
			SVertex * sv = (SVertex *) vlist->data;
			Vertex * v = sv->v;
			if (inited)
			{
				minv[0] = min(minv[0], v->x);
				minv[1] = min(minv[1], v->y);
				minv[2] = min(minv[2], v->z);

				maxv[0] = max(maxv[0], v->x);
				maxv[1] = max(maxv[1], v->y);
				maxv[2] = max(maxv[2], v->z);
			} else {
				minv[0] = v->x;
				minv[1] = v->y;
				minv[2] = v->z;
				maxv[0] = v->x;
				maxv[1] = v->y;
				maxv[2] = v->z;
				inited = 1;
			}
		}
	}
	list_free(&surf_l);
	return inited;
}

void move_child_to_head(ACObject * parent, ACObject * child)
{
/*
	vector<ACObject *> kids;
	kids.push_back(child);
	int found = 0;
	List *kids_list = ac_object_get_childrenlist(parent);
    for (List * p = kids_list; p != NULL; p = p->next)
	if (child == (ACObject *)p->data)
		found = 1;
	else
        kids.push_back((ACObject *)p->data);
	if (!found) return;

	for (vector<ACObject*>::iterator i = kids.begin(); i != kids.end(); ++i)
		object_remove_child_nocleanup(parent, *i);
	for (vector<ACObject*>::iterator i = kids.begin(); i != kids.end(); ++i)
		object_add_child(parent, *i);
*/
	tcl_command("ac3d object_move_to_head %d", child);
}

Surface * obj_get_first_surf(ACObject * obj)
{
	List * sl = ac_object_get_surfacelist(obj);
	if (sl && sl->data)
	{
		return (Surface *) sl->data;
	}
	return NULL;
}
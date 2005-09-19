#include "ac_utils.h"

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

int pull_int_attr(ACObject * ob, const char * attr, int * value)
{
	char * title = ac_object_get_name(ob);
//	printf("Title = %s\n", title);
	char * token = strstr(title, attr);
//	printf("token = %s\n", token);
	if (token == NULL) return 0;
	if (strlen(token) <= strlen(attr))	return 0;
	token += strlen(attr);
	if (value) *value = atoi(token);
//	printf("Value was: %d\n", *value);
	return 1;
}

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

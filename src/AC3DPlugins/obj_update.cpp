#include "obj_update.h"
#include <ac_plugin.h>
#include "ac_utils.h"
#include "obj_model.h"
#include "obj_anim.h"
#include <string>
using std::string;
using std::swap;

static void obj8_update_light(ACObject *obj);
static void obj8_update_object(ACObject *obj, ACObject * root);
static void do_obj8_update(void);
static void obj8_split_anim(ACObject * obj);



enum {
	update_trans,
	update_rotate,
	update_static_trans
};

struct	update_anim_op {
	int op;
	float xyz1[3];	// OFFSET
	float xyz2[3];	// AXIS
	float r1, r2;
	float v1, v2;
	string dataref;
};


static void obj8_update_light(ACObject *obj)
{
	Point3	rgb = { 1.0, 1.0, 1.0 };
	char * token;
	char * title = ac_object_get_name(obj);
	
	token = strstr(title, "LIGHT_NAMED");
	if (token)
	{
		char lname[256];
		sscanf(token,"LIGHT_NAMED %s", lname);
		OBJ_set_light_named(obj, lname);
		return;
	}

	token = strstr(title, "LIGHT_CUSTOM");
	if (token)
	{
		char lname[256];
		float params[9];
		sscanf(token,"LIGHT_CUSTOM %f %f %f %f %f %f %f %f %f %s", 
			params  , params+1, params+2,
			params+3, params+4, params+5,
			params+6, params+7, params+8, lname);
		OBJ_set_light_named(obj, "custom");
		OBJ_set_light_red(obj, params[0]);
		OBJ_set_light_green(obj, params[1]);
		OBJ_set_light_blue(obj, params[2]);
		OBJ_set_light_alpha(obj, params[3]);
		OBJ_set_light_size(obj, params[4]);
		OBJ_set_light_s1(obj, params[5]);
		OBJ_set_light_t1(obj, params[6]);
		OBJ_set_light_s2(obj, params[7]);
		OBJ_set_light_t2(obj, params[8]);
		OBJ_set_light_dataref(obj, lname);		
		return;
	}
	
	token = strstr(title, "_RGB=");
	if (token != NULL)
	{
		if (strlen(token) > 5)
		{
			token += 5;
			if (sscanf(token, "%f,%f,%f", &rgb.x, &rgb.y, &rgb.z)==3)
			{
				OBJ_set_light_named(obj, "rgb");
				OBJ_set_light_red(obj, rgb.x);
				OBJ_set_light_green(obj, rgb.y);
				OBJ_set_light_blue(obj, rgb.x);
			}
		}
	}	
}

void obj8_split_anim(ACObject * obj)
{
	if (strstr(ac_object_get_name(obj), "ANIMATION") != NULL)
	if (strstr(ac_object_get_name(obj), "ANIMATION") != NULL)
	{
		if (!ac_entity_is_class(obj, AC_CLASS_GROUP))
		{
			ACObject * new_grp = new_object(OBJECT_GROUP);
			object_add_child(ac_object_get_parent(obj), new_grp);
			object_reparent(obj, new_grp);

			object_set_name(new_grp, ac_object_get_name(obj));
			ac_object_set_data(new_grp, ac_object_get_data(obj));

			object_set_name(obj, "object");
			ac_object_set_data(obj, "");

			obj8_split_anim(new_grp);
		}
	}
	
		int 	numvert, numsurf, numkids;
		List 	*vertices, *surfaces, *kids;
		List 	*p;

    ac_object_get_contents(obj, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids); 

    for (p = kids; p != NULL; )
    {
		List * pn = p->next;
    	ACObject * child = (ACObject *)p->data;
	        obj8_split_anim(child);
		p = pn;
	}	
	
}

void obj8_update_object(ACObject * obj, ACObject * root)
{
	if (!ac_object_is_visible(obj)) return;
	
		int 	numvert, numsurf, numkids;
		List 	*vertices, *surfaces, *kids;
		List 	*p;

    ac_object_get_contents(obj, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids); 

	float	lod_start, lod_end;
	if (sscanf(ac_object_get_name(obj), "LOD %f/%f",&lod_start, &lod_end)==2)
	{
		OBJ_set_LOD_near(obj, lod_start);
		OBJ_set_LOD_far(obj, lod_end);
	}
	
	if (strstr(ac_object_get_name(obj), "ANIMATION") != NULL)
	{
	
		OBJ_set_animation_group(obj,1);
		char * startp = ac_object_get_data(obj);
		if (startp)
		{
			// ben says: these are needed for now because we have to insert our animaitons BACKWARD because we insert them at the HEAD of the object.  Yuck!
			vector<update_anim_op>		anim_ops;
		
			char * endp = startp + strlen(startp);
			while (startp != endp)
			{
				char * stopp = startp;
				while (stopp < endp && *stopp != '\n')
					++stopp;
				if (stopp < endp) ++stopp;
				
				char		dataref[512];
				float		xyz1[3], xyz2[3], v1, v2, r1, r2;
				
				if (sscanf(startp, "TRANSLATE %f %f %f %f %f %f %f %f %s",
					xyz1,xyz1+1,xyz1+2,
					xyz2,xyz2+1,xyz2+2,
					&v1,&v2,dataref) == 9)
				{	
					if(xyz1[0] != 0.0 || xyz1[1] != 0.0 || xyz1[2] != 0.0)
					{
						anim_ops.push_back(update_anim_op());					
						anim_ops.back().xyz1[0] = xyz1[0];
						anim_ops.back().xyz1[1] = xyz1[1];
						anim_ops.back().xyz1[2] = xyz1[2];
						anim_ops.back().xyz2[0] = xyz2[0];
						anim_ops.back().xyz2[1] = xyz2[1];
						anim_ops.back().xyz2[2] = xyz2[2];
						anim_ops.back().v1 = v1;
						anim_ops.back().v2 = v2;
						anim_ops.back().dataref = dataref;
						anim_ops.back().op = update_static_trans;
					}
					if(xyz1[0] != xyz2[0] || xyz1[1] != xyz2[1] || xyz1[2] != xyz2[2])
					{
						anim_ops.push_back(update_anim_op());					
						anim_ops.back().xyz1[0] = xyz1[0];
						anim_ops.back().xyz1[1] = xyz1[1];
						anim_ops.back().xyz1[2] = xyz1[2];
						anim_ops.back().xyz2[0] = xyz2[0];	// Ben says - this is NOT a relative animation!  This is the "end point".  If it's off in space, that's okay!
						anim_ops.back().xyz2[1] = xyz2[1];	// 
						anim_ops.back().xyz2[2] = xyz2[2];
						anim_ops.back().v1 = v1;
						anim_ops.back().v2 = v2;
						anim_ops.back().dataref = dataref;
						anim_ops.back().op = update_trans;
					}

				}

				if (sscanf(startp, "ROTATE %f %f %f %f %f %f %f %s",
					xyz1,xyz1+1,xyz1+2,
					&r1,&r2,
					&v1,&v2,dataref) == 8)
				{
					anim_ops.push_back(update_anim_op());
					anim_ops.back().xyz1[0] = 0.0;
					anim_ops.back().xyz1[1] = 0.0;
					anim_ops.back().xyz1[2] = 0.0;
					anim_ops.back().xyz2[0] = xyz1[0];
					anim_ops.back().xyz2[1] = xyz1[1];
					anim_ops.back().xyz2[2] = xyz1[2];
					anim_ops.back().v1 = v1;
					anim_ops.back().v2 = v2;
					anim_ops.back().r1 = r1;
					anim_ops.back().r2 = r2;
					anim_ops.back().dataref = dataref;
					anim_ops.back().op = update_rotate;
				}				
				
				startp = stopp;				
			}
			
//			optimize_animations(anim_ops);
			
			for (vector<update_anim_op>::reverse_iterator i = anim_ops.rbegin(); i != anim_ops.rend(); ++i)
			{
				vector<XObjKey>	keys(2);
				keys[0].key = i->v1;
				keys[1].key = i->v2;
				if(i->op==update_trans) {	keys[0].v[0] = i->xyz1[0];keys[0].v[1] = i->xyz1[1];keys[0].v[2] = i->xyz1[2];
											keys[1].v[0] = i->xyz2[1];keys[1].v[1] = i->xyz2[1];keys[1].v[2] = i->xyz2[2];}
				if(i->op==update_rotate) {	keys[0].v[0] = i->r1; keys[1].v[0] = i->r2; }
				switch(i->op) {
				case update_trans:			anim_add_translate	(obj, 1, keys, i->dataref.c_str(), i->dataref.empty() ? "translate" : i->dataref.c_str());		break;
				case update_rotate:			anim_add_rotate		(obj, 1, i->xyz1, i->xyz2, keys, i->dataref.c_str(), i->dataref.empty() ? "rotate" : i->dataref.c_str());		break;
				case update_static_trans:	anim_add_static		(obj, 1, i->xyz1, "none", "static translate");		break;
				}
			}
		}
	}
	
		int	now_poly_os, now_hard, now_blend;

	now_poly_os = pull_int_attr_recursive(obj, "_POLY_OS=",0,root);
	if (now_poly_os != 0) OBJ_set_poly_os(obj, now_poly_os);

	now_hard = pull_int_attr_recursive(obj, "_HARD=", 0, root);
	if (now_hard != 0) OBJ_set_hard(obj, "object");

	now_blend = pull_int_attr_recursive(obj, "_BLEND=", 1, root);
	OBJ_set_blend(obj, now_blend ? -1.0 : 0.5);

	if (ac_entity_is_class(obj, AC_CLASS_LIGHT))
	{
		obj8_update_light(obj);
	}

    for (p = kids; p != NULL; )
    {
		List * pn = p->next;
    	ACObject * child = (ACObject *)p->data;
	        obj8_update_object(child, root);
		p = pn;
	}	
}

static void do_obj8_update(void)
{
	add_undoable_all("Update object");
	obj8_split_anim(ac_get_world());
	obj8_update_object(ac_get_world(),ac_get_world());
	bake_static_transitions(ac_get_world());	
	redraw_all();
}


void register_updater(void)
{
	ac_add_command_full("xplane_update_selection", CAST_CMD(do_obj8_update), 0, NULL, "ac3d xplane_update_selection", "change obj to new system.");	
}

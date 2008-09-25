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

#include "obj_model.h"
#include <string>
#include <set>
#include "ac_utils.h"

using std::string;
using std::set;
static set<OBJ_change_f> gChangeFuncs;


//--------------------------------------------------------------------------------------------------------------------------------------------
// C ACCESS
//--------------------------------------------------------------------------------------------------------------------------------------------

static int find_first_of_c(const char * s, const char * charset);
int find_first_of_c(const char * s, const char * charset)
{
	const char * c1 = charset;
	const char * c2 = c1 + strlen(charset)+1;
	int n = 0;
	while(1)
	{
		for (const char * c = c1; c < c2; ++c)
			if (s[n] == *c)
				return n;
		++n;
	}
}

static bool	ends_in_crlf(const string& x);
bool	ends_in_crlf(const string& x)
{
	if (x.empty()) return false;
	char c = x[x.size()-1];
	return c == '\n' || c == '\r';
}

static void OBJ_set_property_str(ACObject * obj, const char * in_key, const char * new_value)
{
//	printf("Obj %d setting %s -> %s\n", obj, in_key, new_value);
	string::size_type p, e;

	string data(ac_object_get_data(obj) ? ac_object_get_data(obj) : "");
	string value(new_value);
	string key(in_key);

	p = data.find(key);
	if (p == key.npos)
	{
		if (!data.empty() && !ends_in_crlf(data))	data += '\n';
		data += key;
		data += '=';
		data += value;
	}
	else
	{
		e = data.find_first_of("=\r\n", p);

		if (e==data.npos)
		{
			p += key.length();
			data.insert(p,"=");
			data.insert(p+1,value);
		}
		else if (data[e] != '=')
		{
			p += key.length();
			data.insert(p,"=");
			data.insert(p+1,value);
			data.insert(p+1+value.length(),"\n");
		}
		else
		{
			p = e+1;
			e = data.find_first_of("\r\n",p);
			if(e == data.npos)
				e = data.length();
			data.replace(p,e-p,value);
		}
	}

//	object_set_userdata(obj, (char*) data.c_str());
	ac_object_set_data	(obj, (char*) data.c_str());

	for (set<OBJ_change_f>::iterator f = gChangeFuncs.begin(); f != gChangeFuncs.end(); ++f)
		(*f)(obj);
}

static void OBJ_set_property_int(ACObject * ob, const char * tag, int v)
{
	char buf[32];
	sprintf(buf,"%d",v);
	OBJ_set_property_str(ob,tag,buf);
}

static void OBJ_set_property_flt(ACObject * ob, const char * tag, float v)
{
	char buf[32];
	sprintf(buf,"%f",v);
	OBJ_set_property_str(ob,tag,buf);
}

static const char * OBJ_get_property_str(ACObject * obj, const char * tag, char * buf)
{
//	printf("Obj %d getting %s\n", obj, tag);

	buf[0] = 0;
	const char * d = ac_object_get_data(obj);
	if (d == NULL) return buf;
	const char * t = strstr(d, tag);
	if (t == NULL) return buf;

	t += strlen(tag)+1;
	int n = find_first_of_c(t,"\n\r");
	memcpy(buf,t,n);
	buf[n] = 0;
	return buf;
}

static int OBJ_get_property_int(ACObject * obj, const char * tag)
{
	char temp[50];
	return atoi(OBJ_get_property_str(obj,tag,temp));
}

static float OBJ_get_property_flt(ACObject * obj, const char * tag)
{
	char temp[50];
	return atof(OBJ_get_property_str(obj,tag,temp));
}

//--------------------------------------------------------------------------------------------------------------------------------------------
// TCL ACCESS
//--------------------------------------------------------------------------------------------------------------------------------------------


static void	OBJ_get_prop_tcl(const char * s)
{
	vector<ACObject *>		objs;
	vector<ACObject *>::iterator ob;
	find_all_selected_objects(objs);
	if (objs.empty()) return;
	ACObject * obj = objs.front();

	char	temp[1024];
	command_result_append_string((char*)OBJ_get_property_str(obj, s,temp));
}

static void OBJ_set_prop_tcl(int argc, const char * argv[])
{
	vector<ACObject *>		objs;
	vector<ACObject *>::iterator ob;
	find_all_selected_objects_flat(objs);

	for (ob = objs.begin(); ob != objs.end(); ++ob)
		OBJ_set_property_str(*ob, argv[1], argv[2]);
}


void		OBJ_register_datamodel_tcl_cmds(void)
{
	ac_add_command_full("xplane_get_prop", CAST_CMD(OBJ_get_prop_tcl), 1, "s", "ac3d xplane_get_prop key", "get a property.");
	ac_add_command_full("xplane_set_prop", CAST_CMD(OBJ_set_prop_tcl), 2, "argv", "ac3d xplane_set_prop key value", "set a property.");
}


void		OBJ_register_change_cb(OBJ_change_f func)
{
	gChangeFuncs.insert(func);
}

void		OBJ_unregister_change_cb(OBJ_change_f func)
{
	gChangeFuncs.erase(func);
}



//--------------------------------------------------------------------------------------------------------------------------------------------
// TCL ACCESS
//--------------------------------------------------------------------------------------------------------------------------------------------

#define STR_PROP(x,y) \
	void		OBJ_set_##x(ACObject * obj, const char * v) {		OBJ_set_property_str(obj, #y, v); }	\
	const char *OBJ_get_##x(ACObject * obj, char * buf    ) { return OBJ_get_property_str(obj, #y, buf   ); }
#define INT_PROP(x,y) \
	void		OBJ_set_##x(ACObject * obj, int v) {		 OBJ_set_property_int(obj, #y, v); }	\
	int			OBJ_get_##x(ACObject * obj		 ) { return OBJ_get_property_int(obj, #y   ); }
#define FLT_PROP(x,y) \
	void		OBJ_set_##x(ACObject * obj, float v) {		  OBJ_set_property_flt(obj, #y, v); }	\
	float		OBJ_get_##x(ACObject * obj		   ) { return OBJ_get_property_flt(obj, #y   ); }

void		OBJ_set_name(ACObject * obj, const char * name)
{
	ac_object_set_name(obj,(char*)name);

	redraw_all();
	tcl_command("hier_update");
	for (set<OBJ_change_f>::iterator f = gChangeFuncs.begin(); f != gChangeFuncs.end(); ++f)
		(*f)(obj);

}
const char *	OBJ_get_name(ACObject * obj, char * buf)
{
	strcpy(buf,ac_object_get_name(obj));
	return buf;
}


STR_PROP(hard,hard_surf)
INT_PROP(deck,deck)
FLT_PROP(blend,blend)
FLT_PROP(poly_os,poly_os)
INT_PROP(use_materials,use_materials)

STR_PROP(light_named,light_name)
FLT_PROP(light_red,light_red)
FLT_PROP(light_green,light_green)
FLT_PROP(light_blue,light_blue)
FLT_PROP(light_alpha,light_alpha)
FLT_PROP(light_size,light_size)
FLT_PROP(light_s1,light_s1)
FLT_PROP(light_t1,light_t1)
FLT_PROP(light_s2,light_s2)
FLT_PROP(light_t2,light_t2)
FLT_PROP(light_smoke_size,smoke_size);
STR_PROP(light_dataref,dataref)

FLT_PROP(LOD_near,lod_near)
FLT_PROP(LOD_far,lod_far)

STR_PROP(layer_group,layer_group)
INT_PROP(layer_group_offset,layer_group_offset)

INT_PROP(animation_group,animated)
STR_PROP(anim_dataref,dataref)

INT_PROP(anim_keyframe_count,anim_keyframe_count)
INT_PROP(anim_keyframe_root,anim_keyframe_root)
INT_PROP(anim_type,anim_type)

void		OBJ_set_anim_nth_value(ACObject * obj, int n, float v)
{
	char	tag[25];
	sprintf(tag,"value%d",n);
	OBJ_set_property_flt(obj, tag, v);
}

void		OBJ_set_anim_nth_angle(ACObject * obj, int n, float a)
{
	char	tag[25];
	sprintf(tag,"angle%d",n);
	OBJ_set_property_flt(obj, tag, a);
}

float		OBJ_get_anim_nth_value(ACObject * obj, int n)
{
	char	tag[25];
	sprintf(tag,"value%d",n);
	return OBJ_get_property_flt(obj, tag);
}

float		OBJ_get_anim_nth_angle(ACObject * obj, int n)
{
	char	tag[25];
	sprintf(tag,"angle%d",n);
	return OBJ_get_property_flt(obj, tag);
}

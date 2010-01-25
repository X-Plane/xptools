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


#include "TclStubs.h"
#include <ac_plugin.h>
#include "obj_panel.h"
#include "ac_utils.h"
#include "tcl_utils.h"
#include "bitmap_match.h"
#include "prefs.h"

#define PANEL_REGION_DIMS		4

/*
TCL_linked_variv * panel_sub_l		=NULL;
TCL_linked_variv * panel_sub_r		=NULL;
TCL_linked_variv * panel_sub_b		=NULL;
TCL_linked_variv * panel_sub_t		=NULL;
TCL_linked_vari  * panel_sub_count	=NULL;
TCL_linked_vari  * panel_sub_enable	=NULL;
*/

int		is_panel_tex(int tex_id)
{
	return strstrnocase(texture_id_to_name(tex_id), "-PANELS-/Panel.") != NULL ||
		   strstrnocase(texture_id_to_name(tex_id), "-PANELS-/Panel_") != NULL ||
}

/*
int		is_panel_subtex(int tex_id)
{
	if (panel_sub_enable->get() == 0) return -1;

	const char * tname = texture_id_to_name(tex_id);

	for (int n = 0; n < panel_sub_count->get(); ++n)
	{
		char title[50];
		sprintf(title,"panel%d.sub",n);
		if (strstrnocase(tname, title) != NULL) return n;
	}
	return -1;
}
*/

/*
void	do_make_panel_subtexes(void)
{
	if (!panel_sub_enable->get())
	{
		message_dialog("You cannot make panel sub-textures yet.  Enable panel subtextures first.\n");
		return;
	}
	if (panel_sub_count->get() < 1)
	{
		message_dialog("You have specified no panel sub-textures.  Set ac3d up for at least one panel subtexture first.\n");
		return;
	}

	for (int n = 0; n < panel_sub_count->get(); ++n)
	{
		if (panel_sub_l->get(n) >= panel_sub_r->get(n) ||
			panel_sub_b->get(n) >= panel_sub_t->get(n))
		{
			message_dialog("Sub panel %d has illegal bounds: (%d,%d)x(%d,%d)",
				n,panel_sub_l->get(n),panel_sub_b->get(n),panel_sub_r->get(n),panel_sub_t->get(n));
			return;
		}
	}

	char *filter[] = { "PNG files", "*.png", "BMP files", "*.bmp", "All files", "*", NULL };
	char *filename = ac_get_load_filename("Pick a bitman to use for your models...", filter);
	if (STRINGISEMPTY(filename))
		return;

	int big_panel_id = add_new_texture_reload(filename, filename);

	do_make_panel_subtexes_auto(big_panel_id, NULL);
}

void do_make_panel_subtexes_auto(int big_panel_id, int sub_reg_ids[])
{
	ACImage * big_panel_bitmap = texture_id_to_image(big_panel_id);

	for (int n = 0; n < panel_sub_count->get(); ++n)
	{
		char name[50];
		sprintf(name,"panel%d.sub",n);
		int sub_panel_id = add_new_texture_reload(name, name);
		ACImage * sub_image = texture_id_to_image(sub_panel_id);

		bitmap_subcopy(big_panel_bitmap, sub_image, panel_sub_l->get(n),panel_sub_b->get(n),panel_sub_r->get(n),panel_sub_t->get(n));
		if (sub_reg_ids) sub_reg_ids[n] = sub_panel_id;
	}
}
*/

/*
static ACImage * synthetic_loader(char * filename)
{
	ACImage * i = new_acimage(filename);
	ac_image_set_dim(i,16,16,3);
	ac_image_set_data(i,(unsigned char *)myalloc(16*16*3));
	return i;
}
*/

void	register_panel_vars(void)
{
/*	ac_register_texture_loader(".sub", "hack for x-plane", synthetic_loader);

	panel_sub_l		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_l", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_r		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_r", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_b		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_b", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_t		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_t", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_count	= new TCL_linked_vari(ac_get_tcl_interp(), "xplane_panel_sub_count", NULL, NULL, 1);
	panel_sub_enable	= new TCL_linked_vari(ac_get_tcl_interp(), "xplane_panel_sub_enable", NULL, NULL, 0);
*/
}

void	set_std_panel(void)
{
	set_enable_regions(0);
}

void	add_sub_panel(int l, int b, int r, int t)
{
	if (get_enable_regions() == 0) { set_enable_regions(1); set_region_count(0); }

	int sub = get_region_count();
	switch(sub) {
	case 0:
		set_region_l0(l);
		set_region_b0(b);
		set_region_r0(r);
		set_region_t0(t);
		break;
	case 1:
		set_region_l1(l);
		set_region_b1(b);
		set_region_r1(r);
		set_region_t1(t);
		break;
	case 2:
		set_region_l2(l);
		set_region_b2(b);
		set_region_r2(r);
		set_region_t2(t);
		break;
	case 3:
		set_region_l3(l);
		set_region_b3(b);
		set_region_r3(r);
		set_region_t3(t);
		break;
	}
	set_region_count(sub+1);
//	tcl_command("xplane_sync_panel");
}

int		get_sub_panel_count(void)
{
	if (!get_enable_regions()) return 0;
	return get_region_count();
}

int		get_sub_panel_l(int r)
{
	switch(r) {
	case 0: return get_region_l0();
	case 1: return get_region_l1();
	case 2: return get_region_l2();
	case 3: return get_region_l3();
	default: return 0;
	}
}

int		get_sub_panel_b(int r)
{
	switch(r) {
	case 0: return get_region_b0();
	case 1: return get_region_b1();
	case 2: return get_region_b2();
	case 3: return get_region_b3();
	default: return 0;
	}
}

int		get_sub_panel_r(int r)
{
	switch(r) {
	case 0: return get_region_r0();
	case 1: return get_region_r1();
	case 2: return get_region_r2();
	case 3: return get_region_r3();
	default: return 0;
	}
}

int		get_sub_panel_t(int r)
{
	switch(r) {
	case 0: return get_region_t0();
	case 1: return get_region_t1();
	case 2: return get_region_t2();
	case 3: return get_region_t3();
	default: return 0;
	}
}

int	get_sub_panel_for_mesh(ACObject * obj)
{
	if(!get_enable_regions()) return -1;

	float smin = 0.0f, smax = 0.0f, tmin = 0.0f, tmax = 0.0f;
	bool inited = false;

	List * surfaces = ac_object_get_surfacelist(obj);
	for (List * i = surfaces; i != NULL; i = i->next)
	{
        Surface *s = (Surface *)i->data;
        if (surface_get_type(s) == SURFACE_POLYGON)
        {
			for (List * v = s->vertlist; v != NULL; v = v->next)
			{
				SVertex * sv = (SVertex *) v->data;

				if(inited)
				{
					smin = min(smin,sv->tx);
					smax = max(smax,sv->tx);

					tmin = min(tmin,sv->ty);
					tmax = max(tmax,sv->ty);
				}
				else
				{
					inited = true;
					smin = smax = sv->tx;
					tmin = tmax = sv->ty;
				}
			}
        }
	}

	if(!inited)
	{
		printf("Logic error - an empty surface list?\n");
		return -1;
	}

	ACImage * im = texture_id_to_image(ac_object_get_texture_index(obj));
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return -1;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	smin *= (float) panel_x;
	smax *= (float) panel_x;

	tmin *= (float) panel_y;
	tmax *= (float) panel_y;

	int best = -1;
	float best_area = -1;	// go negative so a zero area tri that is INSIDE the region is BETTER than nothing.

	for(int sub = 0; sub < get_region_count(); ++sub)
	{
		float l = max((float)get_sub_panel_l(sub),smin);
		float b = max((float)get_sub_panel_b(sub),tmin);
		float r = min((float)get_sub_panel_r(sub),smax);
		float t = min((float)get_sub_panel_t(sub),tmax);

		float my_area = (r-l) * (t-b);
		if(my_area > best_area)
		{
			best = sub;
			best_area = my_area;
		}
	}
	return best;
}

float	panel_get_texture_repeat_x(int sub, ACObject * obj)
{
	ACImage * im = texture_id_to_image(ac_object_get_texture_index(obj));
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return 1.0;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	// region is 512 wide, panel is 1024 wide
	// we need to DOUBLE our UV map!
	return (float) panel_x / (float) (get_sub_panel_r(sub) - get_sub_panel_l(sub));
}

float	panel_get_texture_repeat_y(int sub, ACObject * obj)
{
	ACImage * im = texture_id_to_image(ac_object_get_texture_index(obj));
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return 1.0;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	return (float) panel_y / (float) (get_sub_panel_t(sub) - get_sub_panel_b(sub));
}


float	panel_get_texture_offset_x(int sub, ACObject * obj)
{
	ACImage * im = texture_id_to_image(ac_object_get_texture_index(obj));
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return 1.0;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	return (float) (-get_sub_panel_l(sub)) / (float)  (get_sub_panel_r(sub) - get_sub_panel_l(sub));
}

float	panel_get_texture_offset_y(int sub, ACObject * obj)
{
	ACImage * im = texture_id_to_image(ac_object_get_texture_index(obj));
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return 1.0;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	return (float) (-get_sub_panel_b(sub)) / (float) (get_sub_panel_t(sub) - get_sub_panel_b(sub));

}

void	panel_get_import_scaling(int tex_id, int sub, float * s_mul, float * t_mul, float * s_add, float * t_add)
{
	*s_mul = *t_mul = 1.0;
	*s_add = *t_add = 0.0;

	ACImage * im = texture_id_to_image(tex_id);
	if(im == NULL)
	{
		printf("Hrm - untextured object?\n");
		return;
	}

	int panel_x, panel_y, dont_care;
	ac_image_get_dim(im,&panel_x,&panel_y,&dont_care);

	*s_add = (float) get_sub_panel_l(sub) / (float) panel_x;
	*t_add = (float) get_sub_panel_b(sub) / (float) panel_y;

	*s_mul = (float) (get_sub_panel_r(sub) - get_sub_panel_l(sub)) / (float) panel_x;
	*t_mul = (float) (get_sub_panel_t(sub) - get_sub_panel_b(sub)) / (float) panel_y;
}

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

#include "obj_panel.h"

#include <ac_plugin.h>
#include "ac_utils.h"
#include "tcl_utils.h"
#include "bitmap_match.h"

#define PANEL_REGION_DIMS		4

TCL_linked_variv * panel_sub_l		=NULL;
TCL_linked_variv * panel_sub_r		=NULL;
TCL_linked_variv * panel_sub_b		=NULL;
TCL_linked_variv * panel_sub_t		=NULL;
TCL_linked_vari  * panel_sub_count	=NULL;
TCL_linked_vari  * panel_sub_enable	=NULL;


int		is_panel_tex(int tex_id)
{
	if (panel_sub_enable->get() == 0)
		return strstrnocase(texture_id_to_name(tex_id), "cockpit/-PANELS-/panel.") != NULL;
	else
		return 0;
}

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

static ACImage * synthetic_loader(char * filename)
{
	ACImage * i = new_acimage(filename);
	ac_image_set_dim(i,16,16,3);
	ac_image_set_data(i,(unsigned char *)myalloc(16*16*3));	
	return i;
}


void	register_panel_vars(void)
{
	ac_register_texture_loader(".sub", "hack for x-plane", synthetic_loader);

	panel_sub_l		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_l", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_r		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_r", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_b		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_b", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_t		= new TCL_linked_variv(ac_get_tcl_interp(), "xplane_panel_sub_t", PANEL_REGION_DIMS, NULL, NULL, 0);
	panel_sub_count	= new TCL_linked_vari(ac_get_tcl_interp(), "xplane_panel_sub_count", NULL, NULL, 1);
	panel_sub_enable	= new TCL_linked_vari(ac_get_tcl_interp(), "xplane_panel_sub_enable", NULL, NULL, 0);
	
	
}

void	set_std_panel(void)
{
	panel_sub_enable->set(0);
}

void	add_sub_panel(int l, int b, int r, int t)
{
	if (panel_sub_enable->get() == 0) { panel_sub_enable->set(1); panel_sub_count->set(0); }
	
	panel_sub_l->set(panel_sub_count->get(),l);
	panel_sub_b->set(panel_sub_count->get(),b);
	panel_sub_r->set(panel_sub_count->get(),r);
	panel_sub_t->set(panel_sub_count->get(),t);
	panel_sub_count->set(panel_sub_count->get()+1);
}

int		get_sub_panel_count(void)
{
	return panel_sub_enable->get() ? panel_sub_count->get() : 0;
}

int		get_sub_panel_l(int r) { return panel_sub_l->get(r); }
int		get_sub_panel_b(int r) { return panel_sub_b->get(r); }
int		get_sub_panel_r(int r) { return panel_sub_r->get(r); }
int		get_sub_panel_t(int r) { return panel_sub_t->get(r); }

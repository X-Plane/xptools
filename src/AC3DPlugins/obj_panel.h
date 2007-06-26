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

#ifndef obj_panel_H
#define obj_panel_H

int		is_panel_tex(int tex_id);		// 1 if true, 0 if false
int		is_panel_subtex(int tex_id);	// -1 if false, or 0-3 for index number

void	register_panel_vars(void);
void	do_make_panel_subtexes(void);
void	do_make_panel_subtexes_auto(int panel_tex_id, int sub_reg_ids[]);

void	set_std_panel(void);
void	add_sub_panel(int l, int b, int r, int t);


int		get_sub_panel_count(void);	// 0 if not used
int		get_sub_panel_l(int r);
int		get_sub_panel_b(int r);
int		get_sub_panel_r(int r);
int		get_sub_panel_t(int r);

#endif /* obj_panel_H */

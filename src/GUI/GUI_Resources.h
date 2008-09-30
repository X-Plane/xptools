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

#ifndef GUI_RESOURCES_H
#define GUI_RESOURCES_H

#include <string>
#include <map>
using std::string;
using std::map;
struct		ImageInfo;

typedef void *	GUI_Resource;

GUI_Resource	GUI_LoadResource(const char * in_resource);
void			GUI_UnloadResource(GUI_Resource res);
const char *	GUI_GetResourceBegin(GUI_Resource res);
const char *	GUI_GetResourceEnd(GUI_Resource res);

bool			GUI_GetTempResourcePath(const char * in_resource, string& out_path);


struct	GUI_TexPosition_t {
	int		real_width;
	int		real_height;
	int		tex_width;
	int		tex_height;
	float	s_rescale;
	float	t_rescale;
};

int GUI_GetImageResource(
			const char *		in_resource,
			ImageInfo *			io_image);

int	GUI_GetTextureResource(
			const char *		in_resource,
			int					flags,
			GUI_TexPosition_t *	out_metrics);	// can be null

int		GUI_GetImageResourceWidth(const char * in_resource);
int		GUI_GetImageResourceHeight(const char * in_resource);
int		GUI_GetImageResourceSize(const char * in_resource, int dims[2]);


inline float	GUI_Rescale_S(float s, GUI_TexPosition_t * metrics) { return s * metrics->s_rescale; }
inline float	GUI_Rescale_T(float t, GUI_TexPosition_t * metrics) { return t * metrics->t_rescale; }


#endif

#ifndef GUI_RESOURCES_H
#define GUI_RESOURCES_H

#include <string>
using std::string;
struct		ImageInfo;

typedef void *	GUI_Resource;

GUI_Resource	GUI_LoadResource(const char * in_resource);
void			GUI_UnloadResource(GUI_Resource res);
const char *	GUI_GetResourceBegin(GUI_Resource res);
const char *	GUI_GetResourceEnd(GUI_Resource res);


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

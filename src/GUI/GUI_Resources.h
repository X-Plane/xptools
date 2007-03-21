#ifndef GUI_RESOURCES_H
#define GUI_RESOURCES_H

#include <string>
using std::string;

int	GUI_GetResourcePath(
			const char *		in_resource, 
			string&				out_path);

struct	GUI_TexPosition_t {
	int		real_width;
	int		real_height;
	int		tex_width;
	int		tex_height;
	float	s_rescale;
	float	t_rescale;
};

int	GUI_GetTextureResource(
			const char *		in_resource,
			int					flags,
			GUI_TexPosition_t *	out_metrics);	// can be null

inline float	GUI_Rescale_S(float s, GUI_TexPosition_t * metrics) { return s * metrics->s_rescale; }
inline float	GUI_Rescale_T(float t, GUI_TexPosition_t * metrics) { return t * metrics->t_rescale; }


#endif

#include "prefs.h"
#include <string.h>

#include <ac_plugin.h>
char *	g_default_layer_group	= STRING("");
int		g_default_layer_offset	= 0;
int		g_export_triangles		= 1;
double	g_default_LOD			= 0.0f;
char *	g_export_prefix			= STRING("");
//export_prefix;

//static PrefSpec	export_airport_lights_spec	;
//static PrefSpec	default_LOD_spec			;

#define	MANAGE_PREF(__Var, __Type)	\
	static PrefSpec	spec##__Var;				\
	spec##__Var.name = "x-plane_" #__Var;	\
	spec##__Var.type = __Type;				\
	spec##__Var.addr = &g_##__Var;			\
	prefs_append_prefspec(&spec##__Var);	\
	ac_tcl_link_pref(&spec##__Var);

void	prefs_init(void)
{
//	g_default_layer_group = STRING("none");
	MANAGE_PREF(default_layer_group, PREF_STRING)
	MANAGE_PREF(default_layer_offset, PREF_INT)
	MANAGE_PREF(export_triangles, PREF_INT)
	MANAGE_PREF(default_LOD, PREF_DOUBLE)
	MANAGE_PREF(export_prefix, PREF_STRING);
	
}

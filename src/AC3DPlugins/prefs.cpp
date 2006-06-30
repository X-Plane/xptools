#include "prefs.h"
#include <ac_plugin.h>
int		g_export_airport_lights	= 0;
double	g_default_LOD			= 0.0f;

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
	MANAGE_PREF(export_airport_lights, PREF_INT)
	MANAGE_PREF(default_LOD, PREF_DOUBLE)
	
}
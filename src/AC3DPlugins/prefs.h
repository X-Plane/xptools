#ifndef PREFS_H
#define PREFS_H

#define PREFS_LIST \
	DEFINE_PREF_STRING(default_layer_group, "none") \
	DEFINE_PREF_INT(default_layer_offset, 0) \
	DEFINE_PREF_INT(export_triangles, 1) \
	DEFINE_PREF_DOUBLE(default_LOD, 0.0f) \
	DEFINE_PREF_STRING(export_prefix, "") \
	DEFINE_PREF_STRING(texture_prefix, "") \

#define DEFINE_PREF_STRING(n,d)		const char *	get_##n(void);
#define DEFINE_PREF_INT(n,d)		int				get_##n(void);
#define DEFINE_PREF_DOUBLE(n,d)		double			get_##n(void);

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE

void	prefs_init(void);



#endif
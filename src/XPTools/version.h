#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define XPT_PACKAGE_VER		"9.04"

#define DDSTOOL_VER		10000
#define DDSTOOL_EXTRAVER	"-b1"

#define DSFTOOL_VER		10300
#define DSFTOOL_EXTRAVER	""

#define OBJCONVERT_VER		30000
#define OBJCONVERT_EXTRAVER	"-b1"

#define MESHTOOL_VER		10000
#define MESHTOOL_EXTRAVER	"-b1"

#define AC3DPLUGIN_VER		30200
#define AC3DPLUGIN_EXTRAVER	"-b2"

#define _VERBUF_SIZE		256
static char __gverbuf[_VERBUF_SIZE];

static inline const char*
product_version(uint32_t ver, const char* extra_ver)
{
	uint32_t maj = ver/10000;
	uint32_t min = (ver%10000)/100;
	uint32_t patchlevel = (ver%100);
	memset(__gverbuf, 0, _VERBUF_SIZE);
	sprintf(__gverbuf, "%u.%u.%u%s", maj, min, patchlevel, extra_ver);
	return __gverbuf;
}

static inline void
print_product_version(const char* product, uint32_t ver, const char* extra_ver)
{
	printf("%s %s, Copyight 2009 Laminar Research.  Compiled on "
		__DATE__".\n", product,	product_version(ver, extra_ver));
	printf("Part of X-Plane Scenery Tools release %s\n", XPT_PACKAGE_VER);
}

#endif /* VERSION_H */

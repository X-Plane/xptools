THIS FILE IS OBSOLETE
#ifndef OBJ_EXPORT_H
#define OBJ_EXPORT_H

#include <ac_plugin.h>
struct XObj;

extern XObj	gObj;

int 		do_obj7_save(char * fname, ACObject * obj);
ACObject *	do_obj7_load(char *filename);

void obj7_output_object(ACObject *ob, set<ACObject *> * stopset);

#endif

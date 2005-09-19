#ifndef OBJ8_EXPORT_H
#define OBJ8_EXPORT_H

#include <ac_plugin.h>
struct XObj;

int 		do_obj8_save(char * fname, ACObject * obj);
ACObject *	do_obj8_load(char *filename);

int 		do_obj7_save_convert(char * fname, ACObject * obj);

#endif

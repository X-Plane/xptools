#ifndef OBJ8_EXPORT_H
#define OBJ8_EXPORT_H

#include <ac_plugin.h>

int 		do_obj8_save(char * fname, ACObject * obj);

int 		do_obj8_save_ex(char * fname, ACObject * obj, int do_prefix, int tex_id, int do_misc);

int 		do_obj7_save_convert(char * fname, ACObject * obj);

#endif

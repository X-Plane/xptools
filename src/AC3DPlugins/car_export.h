#ifndef CAR_EXPORT_H
#define CAR_EXPORT_H

void	matrix_transform_object_recursive(ACObject * ob, double m[16], set<ACObject *> * stopset);
void	debone_hierarchy(ACObject * ob, set<ACObject *> * bones);
void	rebone_hierarchy(ACObject * ob, set<ACObject *> * bones);

int 		do_car_save(char * fname, ACObject * obj);
ACObject *	do_car_load(char *filename);


#endif

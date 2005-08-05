/* $Id$ */



/*** (C) Copyright 2003 by Inivis  ***/







// AC3D Undoable class.  Most new commands should be formed around a subclass of this.

// requires C++(!)





#ifndef UNDOABLE

#define UNDOABLE



class Undoable

{

public:



	Prototype Undoable(); // set up local vars in the construtor

	Prototype virtual ~Undoable();



	Prototype virtual void execute(); // (save the current selection?) and do the actual stuff

	Prototype virtual void undo(); // should restore the world to the previous state

	Prototype virtual void redo(); // if this is not overridden, execute() is called

	Prototype virtual void repeat(); // currently not used



	Prototype Boolean is_repeatable(); // not used

	Prototype Boolean is_redoable();

	Prototype Boolean allows_further_undos();



	Prototype char *get_name();

	Prototype void set_name(char *str); // called in AC3D from add_undoable, use lowercase, short descriptions e.g. "select all"



	Prototype void abort(); // call in construtor if something fails (e.g. no objects are selected), the Undoable will not be added to the main list

	Prototype Boolean was_aborted();



	Prototype Boolean user_break(); // Windows only, returns TRUE is ESC key is down when called



protected:

	Boolean repeatable; // not used

	Boolean redoable; // default TRUE, change in constructor if you need to

	Boolean allow_further_undos; // default TRUE, change in construtor if you need to



private:

	char *name;

	Boolean aborted;

};









extern "C" // visible from outside in plugins etc

{

Prototype void add_undoable(char *name, Undoable *ud);



Prototype void add_undoable_all(char *name);

Prototype void add_undoable_vertex_positions(char *name, List *vertexlist);

Prototype void add_undo_texture_coords(char *name, List *sl, List *svl);

Prototype void add_undoable_world_add_objectlist(char *name, List *obs);

Prototype void add_undoable_world_add_object(char *name, ACObject *ob);







Prototype void add_undoable_group(List *obs);

Prototype void add_undoable_ungroup(List *obs);





Prototype void add_undoable_change_selection(char *name);

Prototype void add_undoable_object_centres(char *name, List *oblist);



}



#endif






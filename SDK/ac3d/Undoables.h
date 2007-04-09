#ifndef UNDOABLES_H
#define UNDOABLES_H



// undo for adding an object to the current world
// undo() will remove and free the new object


#include "Undoable.h"


class UndoableSelection : public Undoable
{
public: 

	UndoableSelection();
	~UndoableSelection();
	void execute();
	void undo();
	void redo();

private:

	ACSelection *oldselection, *newselection;
};


class UndoableAddObjectWorld : public UndoableSelection
{
public:

	UndoableAddObjectWorld(ACObject *ob);
	~UndoableAddObjectWorld();

	void execute();
	void undo();
	void redo();

private:

	ACObject *newobject;
	Boolean freetheobject;

};



class UndoableVertexPositions : public Undoable
{
public:

	UndoableVertexPositions(List *vlist);
	~UndoableVertexPositions();

	void execute();
	void undo();
	void redo();

private:

	List *vertposlist;
	List *newvertposlist;
};




class UndoableChangeTextureCoords: public Undoable
// code defined in changetexturecoords.cpp
{
protected:

	List *svlist; // list of sv structures that contain backup info of texture coors
	List *surfaces, *svs; // list of surfaces and svs that are going to be affected
	List *backupsvs; // list of the new texture coors that are recorded when undo happens

public:

	List *record_texture_coords(); // record the texture coors of the surfaces and svs that we have 
	UndoableChangeTextureCoords(List *surface_list, List *svl);

	void execute();
	void undo();
	void redo();

	~UndoableChangeTextureCoords();
};







class UndoableModifyGeometry : public Undoable
{
protected:
	List *newvertices, *newsurfaces;  // vertices and surfaces that were added (to be removed on undo)
	List *oldvertices, *oldsurfaces; // vertices and surfaces that were removed (to be re-added on undo)
	ACSelection *oldselection, *newselection;
	Boolean newstuffactive; // is the new stuff current in the world?  (indicates what to free when deleting the undoable)
	List *svbackuplist; 

	void swap_svlists();
	void free_svlists();

public:

	UndoableModifyGeometry();
	~UndoableModifyGeometry();

	void record_selection(); // must be called in the superclass execute, to save the selection
	void backup_svlists(List *affectedsurfaces); // call this with a list of surfaces if those surface are going to have their svlists altered

	void undo();
	void redo();
};




#endif // UNDOABLES




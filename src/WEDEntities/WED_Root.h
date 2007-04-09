#ifndef WED_ROOT_H
#define WED_ROOT_H

/*
	WED_Root - THEORY OF OPERATION

	WED_Root provides a non-entity thing that is used to contain everything in a document.
	Typically the children are referenced by special names.  
	
	(Note -- this exists because we prefer not to mix derivation and final classes...thati s, if WED_Thing
	is a base class for others, it does NOT act as a final class.)
	
	WED_Root is a non-entity in that:
	- It does not really have a notion of "spatial extent".
	- Some of its children are not entities.
	- It doesn't implement IGISEntity
	
*/

#include "WED_Thing.h"

class WED_Root : public WED_Thing {

DECLARE_PERSISTENT(WED_Root)

};

#endif

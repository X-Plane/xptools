#ifndef IRESOLVER_H
#define IRESOLVER_H

#include "IBase.h"

/*
	IRESOLVER - THEORY OF OPERATION
	
	A resolver can return objects by a string path, in the form of 
	
	obj.subcomp[idx].recursively[used][if].desired
	
	HOW the resolver resolves these is completely opaque - that is, the resolver interface may not be based
	on anything in particular.
	
	See: WED_Document for an implementation.

*/

class	IResolver : public virtual IBase {
public:

	virtual	IBase *	Resolver_Find(const char * path)=0;
	
};

#endif /* IRESOLVER_H */

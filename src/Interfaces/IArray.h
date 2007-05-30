#ifndef IARRAY_H
#define IARRAY_H

/*

	This abstract interface lets us find ordered sub-elements of any interface.

*/

#include "IBase.h"

class IArray : public virtual IBase {
public:

	virtual	int				Array_Count (void )=0;
	virtual IBase *			Array_GetNth(int n)=0;
	
};

#endif /* IARRAY_H */

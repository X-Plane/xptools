#ifndef IARRAY_H
#define IARRAY_H

/*

	This abstract interface lets us find ordered sub-elements of any interface.

*/

#include "IUnknown.h"

class IArray : public virtual IUnknown {
public:

	virtual	int				Array_Count (void )=0;
	virtual IUnknown *		Array_GetNth(int n)=0;
	
};

#endif /* IARRAY_H */

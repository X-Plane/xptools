#ifndef IARRAY_H
#define IARRAY_H

#include "IUnknown.h"

class IArray : public virtual IUnknown {
public:

	virtual	int				Array_Count (void )=0;
	virtual IUnknown *		Array_GetNth(int n)=0;
	
};

#endif /* IARRAY_H */

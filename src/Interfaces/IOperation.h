#ifndef IOPERATION_H
#define IOPERATION_H

/*

	IOPERATION - THEORY OF OPERATION
	
	IOperation provides an interface to start and end "operations" on a series of objects.
	
	(NOTE: this interface is still a bit of a hack, since operations are implemented archive-wide but 
	provided per-object. hrm.)

*/

#include "IBase.h"

class	IOperation : public virtual IBase {
public:

	virtual	void		StartOperation(const char * op_name)=0;
	virtual	void		CommitOperation(void)=0;
	virtual	void		AbortOperation(void)=0;
	
};	


#endif

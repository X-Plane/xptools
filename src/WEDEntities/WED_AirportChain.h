#ifndef WED_AIRPORTCHAIN_H
#define WED_AIRPORTCHAIN_H

#include "WED_GISChain.h"

class	WED_AirportChain : public WED_GISChain {

DECLARE_PERSISTENT(WED_AirportChain)

public:
	
	virtual	IGISPoint *		SplitSide   (int n	)		;		// Split the side from pt N to pt N + 1 in half. Return the new pt.
	virtual	bool			IsClosed	(void	) const	;
	
			void			SetClosed(int closure);

	// WED_Persistent
	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);	

private:

	int		closed;
	
};

#endif /* WED_AIRPORTCHAIN_H */

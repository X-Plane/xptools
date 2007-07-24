#ifndef WED_AIRPORTCHAIN_H
#define WED_AIRPORTCHAIN_H

#include "WED_GISChain.h"

struct	AptMarking_t;

class	WED_AirportChain : public WED_GISChain {

DECLARE_PERSISTENT(WED_AirportChain)

public:
	
//	virtual	IGISPoint *		SplitSide   (int n	)		;		// Split the side from pt N to pt N + 1 in half. Return the new pt.
	virtual	bool			IsClosed	(void	) const	;
	
			void			SetClosed(int closure);

	// WED_Persistent
	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db, const map<int,int>& mapping);
	virtual void			ToDB(sqlite3 * db);	

			void			Import(const AptMarking_t& x, void (* print_func)(void *, const char *, ...), void * ref);
			void			Export(		 AptMarking_t& x) const;

private:

	WED_PropIntEnumSetUnion	lines;
	WED_PropIntEnumSetUnion	lights;

	int		closed;
	
};

#endif /* WED_AIRPORTCHAIN_H */

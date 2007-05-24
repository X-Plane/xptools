#ifndef WED_PACKAGE_H
#define WED_PACKAGE_H

#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"

class	WED_Document;

enum {

	status_None,
	status_XES,
	status_DSF,
	status_Stale,
	status_UpToDate
	
};


class	WED_Package : public GUI_Broadcaster, public GUI_Listener{
public:

	WED_Package(const char * inPath, bool inCreate);
	~WED_Package();
	
	int				GetTileStatus(int lon, int lat);
	WED_Document *	GetTileDocument(int lon, int lat);
	
	WED_Document *	OpenTile(int lon, int lat);
	WED_Document *	NewTile(int lon, int lat);
	
	void			Rescan(void);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:


				 WED_Package(const WED_Package&);
	WED_Package& operator=  (const WED_Package&);

	string			mPackageBase;	
	int				mStatus[360*180];
	WED_Document *	mTiles[360*180];
	
};

#endif

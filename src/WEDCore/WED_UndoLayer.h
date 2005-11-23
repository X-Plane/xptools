#ifndef WED_UNDOLAYER_H
#define WED_UNDOLAYER_H

#include "WED_GUID.h"
class	WED_Archive;
class	WED_Buffer;
class	WED_Persistent;

class	WED_UndoLayer {
public:

				WED_UndoLayer(WED_Archive * inArchive, const string& inName);
				~WED_UndoLayer(void);
	
		void 	ObjectCreated(WED_Persistent * inObject);
		void	ObjectChanged(WED_Persistent * inObject);
		void	ObjectDestroyed(WED_Persistent * inObject);

		void	Execute(void);
		
		string	GetName(void) { return mName; }

private:

	enum LayerOp {
			op_Created,
			op_Changed,
			op_Destroyed
	};
	
	struct ObjInfo {
		LayerOp			op;
		WED_GUID		guid;
		const char *	the_class;
		WED_Buffer *	buffer;
	};
	
	typedef hash_map<WED_GUID, ObjInfo>		ObjInfoMap;
	
	ObjInfoMap		mObjects;
	WED_Archive *	mArchive;
	string			mName;	

	// Things we do not allow
	WED_UndoLayer();
	WED_UndoLayer(const WED_UndoLayer&);
	WED_UndoLayer& operator=(const WED_UndoLayer&);
	
};

#endif /* WED_UNDOLAYER_H */

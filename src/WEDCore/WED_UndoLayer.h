#ifndef WED_UNDOLAYER_H
#define WED_UNDOLAYER_H

class	WED_Archive;
class	WED_Buffer;
class	WED_FastBuffer;
class	WED_FastBufferGroup;
class	WED_Persistent;

#define 	UNDO_DISCARD	((WED_UndoLayer *) -1)


class	WED_UndoLayer {
public:

				WED_UndoLayer(WED_Archive * inArchive, const string& inName);
				~WED_UndoLayer(void);
	
		void 	ObjectCreated(WED_Persistent * inObject);
		void	ObjectChanged(WED_Persistent * inObject, int change_kind);
		void	ObjectDestroyed(WED_Persistent * inObject);

		void	Execute(void);
		
		string	GetName(void) { return mName; }
		
		bool	Empty(void) const { return mObjects.empty(); }

		int		GetChangeMask(void) { return mChangeMask; }

private:

	enum LayerOp {
			op_Created,
			op_Changed,
			op_Destroyed
	};
	
	struct ObjInfo {
		LayerOp				op;
		int					id;
		const char *		the_class;
		WED_FastBuffer *	buffer;
	};
	
	typedef hash_map<int, ObjInfo>		ObjInfoMap;
	
	ObjInfoMap				mObjects;
	WED_Archive *			mArchive;
	string					mName;	
	int						mChangeMask;
	WED_FastBufferGroup *	mStorage;

	// Things we do not allow
	WED_UndoLayer();
	WED_UndoLayer(const WED_UndoLayer&);
	WED_UndoLayer& operator=(const WED_UndoLayer&);
	
};

#endif /* WED_UNDOLAYER_H */

#ifndef WED_GUID_H
#define WED_GUID_H

#include <hash_fun>

class	IOReader;
class	IOWriter;

class WED_GUID {
public:

					WED_GUID();
					WED_GUID(void *) : mCounter(0), mTicks(0), mClock(0), mNode(0) { }
					WED_GUID(const WED_GUID& rhs) : mCounter(rhs.mCounter), mTicks(rhs.mTicks), mClock(rhs.mClock), mNode(rhs.mNode) { }
	WED_GUID&		operator=(const WED_GUID& rhs) { mCounter = rhs.mCounter; mTicks = rhs.mTicks; mClock = rhs.mClock; mNode = rhs.mNode; return *this; }
	
	bool			operator==(const WED_GUID& rhs) const { return mCounter == rhs.mCounter && mTicks == rhs.mTicks && mClock == rhs.mClock && mNode == rhs.mNode; }
	bool			operator!=(const WED_GUID& rhs) const { return mCounter != rhs.mCounter || mTicks != rhs.mTicks || mClock != rhs.mClock || mNode != rhs.mNode; }
	bool			operator< (const WED_GUID& rhs) const { return mCounter == rhs.mCounter ? (
																		mTicks == rhs.mTicks ? (
																			mClock == rhs.mClock ? mNode < rhs.mNode : mClock < rhs.mClock
																		) : mTicks < rhs.mTicks 
																	) : mCounter < rhs.mCounter; }
	
	void			ReadFrom(IOReader * reader);
	void			WriteTo(IOWriter * writer);
	
	size_t			hash(void) const { return mCounter ^ (mTicks << 4) ^ (mClock >> 3) ^ (mNode >> 10); }
	
	int		mCounter;	// Unique sequence counter for this instance of the app
	int		mTicks;		// Ticks since app run
	int		mClock;		// Curernt world time
	int		mNode;		// Node ID of machine on net

};

template <>
struct hash<WED_GUID>
	: std::unary_function<WED_GUID, std::size_t>
{
	std::size_t operator()(const WED_GUID& key) const { return key.hash(); }
};

extern const WED_GUID	NULL_GUID;

#endif /* WED_GUID_H */

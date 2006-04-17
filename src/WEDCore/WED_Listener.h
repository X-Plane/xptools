#ifndef WED_LISTENER_H
#define WED_LISTENER_H

class	WED_Broadcaster;

class	WED_Listener {
public:

					 WED_Listener();
	virtual			~WED_Listener();
	
	virtual	void	ReceiveMessage(
							WED_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)=0;

private:

	friend	class	WED_Broadcaster;

	set<WED_Broadcaster *>	mBroadcasters;
	
};

#endif

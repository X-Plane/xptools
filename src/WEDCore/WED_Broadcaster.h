#ifndef WED_BROADCASTER_H
#define WED_BROADCASTER_H

class	WED_Listener;

class	WED_Broadcaster {
public:

			 WED_Broadcaster();
	virtual	~WED_Broadcaster();
	
	void	BroadcastMessage(int inMsg, int inParam);
	
	void	AddListener(WED_Listener * inListener);
	void	RemoveListener(WED_Listener * inListener);
	
private:

	friend class	WED_Listener;

	set<WED_Listener *>		mListeners;

};

#endif

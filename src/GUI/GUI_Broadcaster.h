#ifndef GUI_BROADCASTER_H
#define GUI_BROADCASTER_H

class	GUI_Listener;

class	GUI_Broadcaster {
public:

			 GUI_Broadcaster();
	virtual	~GUI_Broadcaster();
	
	void	BroadcastMessage(int inMsg, int inParam);
	
	void	AddListener(GUI_Listener * inListener);
	void	RemoveListener(GUI_Listener * inListener);
	
private:

	friend class	GUI_Listener;

	set<GUI_Listener *>		mListeners;

};

#endif

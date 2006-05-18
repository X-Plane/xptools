#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H

class	GUI_Broadcaster;

class	GUI_Listener {
public:

					 GUI_Listener();
	virtual			~GUI_Listener();
	
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)=0;

private:

	friend	class	GUI_Broadcaster;

	set<GUI_Broadcaster *>	mBroadcasters;
	
};

#endif

#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H


class	GUI_Application {
public:
					 GUI_Application();
	virtual			~GUI_Application();

	void			Run(void);	
	void			Quit(void);
	
	virtual	void	OpenFiles(const vector<string>& inFiles)=0;

private:

	bool		mDone;

};

#endif



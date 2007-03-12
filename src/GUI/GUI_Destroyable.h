#ifndef GUI_DESTROYABLE_H
#define GUI_DESTROYABLE_H

class	GUI_Destroyable {
public:

	virtual			~GUI_Destroyable();	
			void	AsyncDestroy(void);
			
};

#endif /* GUI_DESTROYABLE_H */
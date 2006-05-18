#ifndef GUI_CONTROL_H
#define GUI_CONTROL_H

#include "GUI_Pane.h"
#include "GUI_Broadcaster.h"

class GUI_Control : public GUI_Pane, public GUI_Broadcaster {
public:

			 		 GUI_Control();
	virtual			~GUI_Control();
	
			float	GetValue(void) const;
			float	GetMin(void) const;
			float	GetMax(void) const;
			float	GetPageSize(void) const;
	
	virtual	void	SetValue(float inValue);
	virtual	void	SetMin(float inMin);
	virtual	void	SetMax(float inMax);
	virtual	void	SetPageSize(float inPageSize);

	// Overrides from GUI_Pane.

private:

		float		mValue;
		float		mMin;
		float		mMax;
		float		mPageSize;

};

#endif /* GUI_CONTROL_H */

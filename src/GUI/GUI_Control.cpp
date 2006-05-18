#include "GUI_Control.h"
#include "GUI_Messages.h"

GUI_Control::GUI_Control() :
	mMin(0), mMax(1), mValue(0), mPageSize(1)
{
}

GUI_Control::~GUI_Control()
{
}

float	GUI_Control::GetValue(void) const
{
	return mValue;
}

float	GUI_Control::GetMin(void) const
{
	return mMin;
}

float	GUI_Control::GetMax(void) const
{
	return mMax;
}

float	GUI_Control::GetPageSize(void) const
{
	return mPageSize;
}
	
void	GUI_Control::SetValue(float inValue)
{
	mValue = inValue;
	BroadcastMessage(GUI_CONTROL_VALUE_CHANGED, 0);
}

void	GUI_Control::SetMin(float inMin)
{
	mMin = inMin;
}

void	GUI_Control::SetMax(float inMax)
{
	mMax = inMax;
}	

void	GUI_Control::SetPageSize(float inPageSize)
{
	mPageSize = inPageSize;
}

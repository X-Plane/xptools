#ifndef GUI_GRAPHSTATE_H
#define GUI_GRAPHSTATE_H

class GUI_GraphState {
public:

	void		Init(void);
	void		Reset(void);
	
	void		EnableLighting(bool lighting);
	void		SetTexUnits(int count);
	void		EnableFog(bool fog);
	void		EnableAlpha(bool test, bool blend);
	void		EnableDepth(bool read, bool write);

	void		BindTex(int id, int unit);

	inline void	SetState(bool lit, int tex, bool fog, bool test, bool blend, bool read, bool write)
	{
		EnableLighting(lit);
		SetTexUnits(tex);
		EnableFog(fog);
		EnableAlpha(test,blend);
		EnableDepth(read,write);
	}
	
};
	

#endif

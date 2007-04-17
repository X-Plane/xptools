NOT SED
#ifndef WED_LAYERTABLE_H
#define WED_LAYERTABLE_H

#include "GUI_Listener.h"
#include "GUI_Table.h"
class	WED_AbstractLayers;

class WED_LayerTable : public GUI_TableContent, public GUI_Listener {
public:

						 WED_LayerTable();
	virtual				~WED_LayerTable();

			void		SetLayers(WED_AbstractLayers * inLayers);

	virtual	void		CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  );
	virtual	int			CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);
	virtual	void		CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);
	virtual	void		CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:

		WED_AbstractLayers *	mLayers;

};


class	WED_LayerTableGeometry : public GUI_TableGeometry {
public:

						 WED_LayerTableGeometry();
	virtual				~WED_LayerTableGeometry();

			void		SetLayers(WED_AbstractLayers * inLayers);

	virtual	int			GetColCount(void);
	virtual	int			GetRowCount(void);	
	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);
	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);
	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);
	
private:

		vector<int>				mWidths;
		WED_AbstractLayers *	mLayers;

};

#endif

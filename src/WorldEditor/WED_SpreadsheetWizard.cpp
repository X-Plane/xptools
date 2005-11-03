#include "WED_SpreadsheetWizard.h"
#include "XPWidgets.h"
#include "XPWidgetDialogs.h"
#include "WED_Globals.h"
#include "WED_Notify.h"
#include "WED_Msgs.h"

static	XPWidgetID		sWizard = NULL;

struct	WED_WizardParams {
	float	elev_min;	float	elev_max;
	float	slop_min;	float	slop_max;
	float	temp_min;	float	temp_max;
	float	tmpr_min;	float	tmpr_max;
	float	rain_min;	float	rain_max;
	float	sdir_min;	float	sdir_max;
	float	relv_min;	float	relv_max;
	float	erng_min;	float	erng_max;
	float	uden_min;	float	uden_max;
	float	ucen_min;	float	ucen_max;
	float	utrn_min;	float	utrn_max;
};

static WED_WizardParams	sWizardParams = { 0 };

void	WED_WizardAction(XPWidgetID);

void	WED_ShowSpreadsheetWizard(void)
{
	if (sWizard != NULL)
	{
		if (!XPIsWidgetVisible(sWizard))
			XPShowWidget(sWizard);
		else
			XPBringRootWidgetToFront(sWizard);
	} else {
		sWizard = XPCreateWidgetLayout(
			0, XP_DIALOG_BOX, "Spreadsheet Params", XP_DIALOG_CLOSEBOX, 1, 0, NULL,
				XP_COLUMN,
					// Landuse
					// Climate
					// Rain
					
					XP_ROW, XP_CAPTION, "Elev Min/Max (M)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.elev_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.elev_max, XP_END,
					XP_ROW, XP_CAPTION, "Slope Min/Max (D)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.slop_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.slop_max, XP_END,
					XP_ROW, XP_CAPTION, "Temp Min/Max (M)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.temp_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.temp_max, XP_END,
					XP_ROW, XP_CAPTION, "Temp Range Min/Max (M)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.tmpr_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.tmpr_max, XP_END,
					XP_ROW, XP_CAPTION, "Rain Min/Max (mm)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.rain_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.rain_max, XP_END,
					XP_ROW, XP_CAPTION, "Slope Dir Min/Max (D)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.sdir_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.sdir_max, XP_END,
					XP_ROW, XP_CAPTION, "Relative Elevation Min/Max", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.relv_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.relv_max, XP_END,
					XP_ROW, XP_CAPTION, "Elevation Range Min/Max (M)", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.erng_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.erng_max, XP_END,
					XP_ROW, XP_CAPTION, "Urban Density Min/Max", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.uden_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.uden_max, XP_END,
					XP_ROW, XP_CAPTION, "Urban Centrality Min/Max", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.ucen_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.ucen_max, XP_END,
					XP_ROW, XP_CAPTION, "Urban Transport Min/Max", XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.utrn_min, XP_EDIT_FLOAT, 6, 6, 0, &sWizardParams.utrn_max, XP_END,

					XP_ROW, XP_BUTTON_ACTION, "Apply", WED_WizardAction, XP_END,
				XP_END,
			XP_END);
		XPShowWidget(sWizard);
	}
}

inline float	InRange(float minv, float maxv, float vv) { return minv <= vv && vv <= maxv; }
inline float	FetchEquiv(const DEMGeo& master, const DEMGeo& slave, int x, int y)
{
	if (master.mWidth == slave.mWidth && master.mHeight == slave.mHeight)
		return slave.get(x,y);
	else
		return slave.get(slave.lon_to_x(master.x_to_lon(x)), slave.lat_to_y(master.y_to_lat(y)));
}

#define	TEST_RULE(__DEM, __MIN, __MAX)							\
			if (gDem.count(__DEM) == 0 ||						\
			sWizardParams.__MIN == sWizardParams.__MAX ||		\
			InRange(sWizardParams.__MIN, sWizardParams.__MAX, FetchEquiv(wizard, gDem[__DEM], x, y)))
	

inline double cosdeg(double deg)
{
	if (deg == 0.0) return 1.0;
	if (deg == 90.0) return 0.0;
	if (deg == 180.0) return -1.0;
	if (deg == -90.0) return 0.0;
	if (deg == -180.0) return -1.0;
	return cos(deg * DEG_TO_RAD);
}

void	WED_WizardAction(XPWidgetID)
{
	XPSendMessageToWidget(sWizard, xpMsg_DataFromDialog, xpMode_Recursive, 0, 0);

	float	old_slop_min = sWizardParams.slop_min;
	float	old_slop_max = sWizardParams.slop_max;
	float	old_sdir_min = sWizardParams.sdir_min;
	float	old_sdir_max = sWizardParams.sdir_max;

	sWizardParams.slop_min = 1.0 - cosdeg(sWizardParams.slop_min);
	sWizardParams.slop_max = 1.0 - cosdeg(sWizardParams.slop_max);

	sWizardParams.sdir_min = cosdeg(sWizardParams.sdir_min);
	sWizardParams.sdir_max = cosdeg(sWizardParams.sdir_max);
	

	DEMGeo *	widest = NULL;
	for (DEMGeoMap::iterator dem = gDem.begin(); dem != gDem.end(); ++dem)
	{
		if (widest == NULL || (widest->mWidth * widest->mHeight) < (dem->second.mWidth * dem->second.mHeight))
			widest = &dem->second;			
	}
	if (widest == NULL) return;
	DEMGeo&	wizard(gDem[dem_Wizard]);
	wizard = *widest;
	
	for (int y = 0; y < wizard.mHeight; ++y)
	for (int x = 0; x < wizard.mWidth ; ++x)
	{
		wizard(x,y) = 0.0;
		TEST_RULE(dem_Elevation, 			elev_min, elev_max)
		TEST_RULE(dem_Slope, 				slop_min, slop_max)
		TEST_RULE(dem_Temperature, 			temp_min, temp_max)
		TEST_RULE(dem_TemperatureRange, 		tmpr_min, tmpr_max)
		TEST_RULE(dem_Rainfall, 			rain_min, rain_max)
		TEST_RULE(dem_SlopeHeading,			sdir_min, sdir_max)
		TEST_RULE(dem_RelativeElevation, 	relv_min, relv_max)
		TEST_RULE(dem_ElevationRange, 		erng_min, erng_max)
		TEST_RULE(dem_UrbanDensity, 		uden_min, uden_max)
		TEST_RULE(dem_UrbanRadial, 			ucen_min, ucen_max)
		TEST_RULE(dem_UrbanTransport,	 	utrn_min, utrn_max)
		{
			wizard(x,y) = 1.0;
		}
	}
	
	sWizardParams.slop_min = old_slop_min;
	sWizardParams.slop_max = old_slop_max;
	sWizardParams.sdir_min = old_sdir_min;
	sWizardParams.sdir_max = old_sdir_max;
	
	WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);	
}

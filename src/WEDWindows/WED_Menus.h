/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef WED_MENUS_H
#define WED_MENUS_H

#include "GUI_Menus.h"

enum {

	// File Menu
	wed_NewPackage = GUI_APP_MENUS,
	wed_OpenPackage,
	wed_ChangeSystem,
	wed_Validate,
	wed_ImportApt,
	wed_ExportApt,
	wed_ExportPack,
#if HAS_GATEWAY
	wed_ExportToGateway,
#endif
	wed_ImportDSF,
	wed_ImportOrtho,
#if HAS_GATEWAY
	wed_ImportGateway,
#endif
#if GATEWAY_IMPORT_FEATURES
	wed_ImportGatewayExtract,
#endif
	// Export Target Submenu
	wed_Export900,
	wed_Export1000,
	wed_Export1021,
	wed_Export1050,
	wed_Export1100,
	wed_Export1130,
	wed_ExportGateway,
	// Edit Menu,
	wed_Group,
	wed_Ungroup,
	wed_Crop,
	wed_Overlay,
	wed_CopyToAirport,
//	wed_MakeRouting,
	wed_Split,
	wed_Align,
	wed_MatchBezierHandles,
	wed_Orthogonalize,
	wed_RegularPoly,
	wed_Merge,
	wed_Reverse,
	wed_Rotate,
	wed_MoveFirst,
	wed_MovePrev,
	wed_MoveNext,
	wed_MoveLast,
	wed_BreakApartSpecialAgps,
	wed_ReplaceVehicleObj,
	// Convert To menu
	wed_ConvertToPolygon,
	wed_ConvertToTaxiway,
	wed_ConvertToTaxiline,
	wed_ConvertToLine,
	// Pavement menu
	wed_Pavement0,
	wed_Pavement25,
	wed_Pavement50,
	wed_Pavement75,
	wed_Pavement100,
	// Density Menu
	wed_ObjDensity1,
	wed_ObjDensity2,
	wed_ObjDensity3,
	wed_ObjDensity4,
	wed_ObjDensity5,
	wed_ObjDensity6,
	// View menu
	wed_ZoomWorld,
	wed_ZoomAll,
	wed_ZoomSelection,

	wed_Map3D,
	wed_MapATC,
	wed_MapPavement,
	wed_MapSelection,
//	wed_UnitFeet,
//	wed_UnitMeters,
	wed_ToggleLines,
	wed_ToggleVertices,
	wed_PickOverlay,
//	wed_ToggleOverlay,
	wed_ToggleWorldMap,
	wed_ToggleNavaidMap,
	wed_SlippyMapNone,
	wed_SlippyMapOSM,
	wed_SlippyMapESRI,
	wed_SlippyMapCustom,
#if WITHNWLINK
	wed_ToggleLiveView,
#endif
	wed_TogglePreview,
	wed_RestorePanes,
	// Select Menu
	wed_SelectParent,
	wed_SelectChild,
	wed_SelectVertex,
	wed_SelectPoly,
	wed_SelectConnected,
	wed_SelectZeroLength,
	wed_SelectDoubles,
	wed_SelectCrossing,
	wed_SelectLocalObjects,
	wed_SelectLibraryObjects,
	wed_SelectDefaultObjects,
	wed_SelectThirdPartyObjects,
	wed_SelectMissingObjects,
	// Airport Menu
	wed_CreateApt,
	wed_EditApt,
	wed_AddATCFreq,
	wed_AddATCFlow,
	wed_AddATCRunwayUse,
	wed_AddATCTimeRule,
	wed_AddATCWindRule,
	wed_UpgradeRamps,
	wed_AlignApt,
	//-- Add Metadata Keys Menu--
	//Organized by alphabetical order
	wed_AddMetaDataBegin,//WARNING: DO NOT USE!
	wed_AddMetaDataCity,// or Locality
	wed_AddMetaDataCountry,
	wed_AddMetaDataDatumLat,
	wed_AddMetaDataDatumLon,
	wed_AddMetaDataFAA,
	wed_AddMetaDataLGuiLabel,
	wed_AddMetaDataIATA,
	wed_AddMetaDataICAO,
	wed_AddMetaDataLocal,
	wed_AddMetaDataLocAuth,
	wed_AddMetaDataRegionCode,
	wed_AddMetaDataState,// or Province
	wed_AddMetaDataTransitionAlt,//Altitude
	wed_AddMetaDataTransitionLevel,
	wed_AddMetaDataEnd,//WARNING: DO NOT USE!
	//---------------------------//
	wed_UpdateMetadata, //Open up dialogbox
	wed_autoOpenLibPane,
	wed_autoOpenPropPane,
	wed_autoClosePane,
	// Help Menu
	wed_HelpManual,
	wed_HelpScenery,
	wed_OSMFixTheMap,
	wed_ESRIUses
};

class	GUI_Application;

void WED_MakeMenus(GUI_Application * inApp);

#endif

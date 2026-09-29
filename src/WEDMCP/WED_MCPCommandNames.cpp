/*
 * Copyright (c) 2026, Laminar Research.
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

#include "WED_MCPCommandNames.h"
#include "WED_Menus.h"

// Every menu command, by its enum name.  Command numbers shift with build flags (HAS_GATEWAY, ROAD_EDITING...), so the
// MCP interface addresses commands by name only.  Keep the #if guards in sync with WED_Menus.h.
#define C(x) { #x, x },

static const WED_MCPCommandName kCommands[] = {
	C(gui_About)
	C(gui_Prefs)
	C(gui_Quit)
	C(gui_New)
	C(gui_Open)
	C(gui_Close)
	C(gui_Save)
	C(gui_Revert)
	C(gui_Undo)
	C(gui_Redo)
	C(gui_Cut)
	C(gui_Copy)
	C(gui_Paste)
	C(gui_Clear)
	C(gui_Duplicate)
	C(gui_SelectAll)
	C(gui_SelectNone)
	C(wed_NewPackage)
	C(wed_OpenPackage)
	C(wed_ChangeSystem)
	C(wed_Validate)
	C(wed_ImportApt)
	C(wed_ExportApt)
	C(wed_ExportPack)
#if HAS_GATEWAY
	C(wed_ExportToGateway)
#endif
	C(wed_ImportDSF)
	C(wed_ImportScenery)
#if ROAD_EDITING
	C(wed_ImportRoads)
#endif
	C(wed_ImportOrtho)
	C(wed_ImportDem)
#if HAS_GATEWAY
	C(wed_ImportGateway)
#endif
#if GATEWAY_IMPORT_FEATURES
	C(wed_ImportGatewayExtract)
#endif
	C(wed_Export900)
	C(wed_Export1000)
	C(wed_Export1021)
	C(wed_Export1050)
	C(wed_Export1100)
	C(wed_Export1130)
	C(wed_Export1200)
	C(wed_Export1212)
	C(wed_ExportGateway)
	C(wed_Group)
	C(wed_Ungroup)
	C(wed_Crop)
	C(wed_CopyToAirport)
	C(wed_Split)
	C(wed_Align)
	C(wed_MatchBezierHandles)
	C(wed_Orthogonalize)
	C(wed_RegularPoly)
	C(wed_Merge)
	C(wed_Reverse)
	C(wed_Rotate)
	C(wed_MoveFirst)
	C(wed_MovePrev)
	C(wed_MoveNext)
	C(wed_MoveLast)
	C(wed_BreakApartAgps)
	C(wed_ReplaceVehicleObj)
	C(wed_ConvertToPolygon)
	C(wed_ConvertToTaxiway)
	C(wed_ConvertToTaxiline)
	C(wed_ConvertToLine)
	C(wed_ConvertToString)
	C(wed_ConvertToForest)
	C(wed_ConvertToShape)
	C(wed_Pavement0)
	C(wed_Pavement25)
	C(wed_Pavement50)
	C(wed_Pavement75)
	C(wed_Pavement100)
	C(wed_ObjDensity1)
	C(wed_ObjDensity2)
	C(wed_ObjDensity3)
	C(wed_ObjDensity4)
	C(wed_ObjDensity5)
	C(wed_ObjDensity6)
	C(wed_ZoomWorld)
	C(wed_ZoomAll)
	C(wed_ZoomSelection)
	C(wed_Map3D)
	C(wed_MapATC)
	C(wed_MapPavement)
	C(wed_MapSelection)
	C(wed_ToggleLines)
	C(wed_ToggleVertices)
	C(wed_PickOverlay)
	C(wed_ToggleWorldMap)
	C(wed_ToggleNavaidMap)
	C(wed_ToggleTerrainMap)
	C(wed_SlippyMapNone)
	C(wed_SlippyMapOSM)
	C(wed_SlippyMapESRI)
	C(wed_SlippyMapCustom)
#if WITHNWLINK
	C(wed_ToggleLiveView)
#endif
	C(wed_TogglePreview)
	C(wed_TogglePreviewWindow)
	C(wed_ShowMapAreaInPreviewWindow)
	C(wed_CenterMapOnPreviewCamera)
	C(wed_RestorePanes)
	C(wed_SelectParent)
	C(wed_SelectChild)
	C(wed_SelectVertex)
	C(wed_SelectPoly)
	C(wed_SelectConnected)
	C(wed_SelectZeroLength)
	C(wed_SelectDoubles)
	C(wed_SelectCrossing)
	C(wed_SelectLocalObjects)
	C(wed_SelectLibraryObjects)
	C(wed_SelectDefaultObjects)
	C(wed_SelectThirdPartyObjects)
	C(wed_SelectMissingObjects)
	C(wed_CreateApt)
	C(wed_EditApt)
	C(wed_AddATCFreq)
	C(wed_AddATCFlow)
	C(wed_AddATCRunwayUse)
	C(wed_AddATCTimeRule)
	C(wed_AddATCWindRule)
	C(wed_UpgradeRamps)
	C(wed_UpgradeJetways)
	C(wed_UpgradeArt)
	C(wed_AgePavement)
	C(wed_EdgePavement)
	C(wed_MowGrass)
	C(wed_AlignApt)
	C(wed_AddMetaDataAltimeterSetting)
	C(wed_AddMetaDataCity)
	C(wed_AddMetaDataCircuits)
	C(wed_AddMetaDataCountry)
#if GATEWAY_IMPORT_FEATURES
	C(wed_AddMetaDataCredits)
#endif
	C(wed_AddMetaDataDatumLat)
	C(wed_AddMetaDataDatumLon)
	C(wed_AddMetaDataFAA)
	C(wed_AddMetaDataLGuiLabel)
	C(wed_AddMetaDataIATA)
	C(wed_AddMetaDataICAO)
	C(wed_AddMetaDataLocal)
	C(wed_AddMetaDataLocAuth)
	C(wed_AddMetaDataOilrig)
	C(wed_AddMetaDataRegionCode)
	C(wed_AddMetaDataState)
	C(wed_AddMetaDataTowerCaps)
	C(wed_AddMetaDataTransitionAlt)
	C(wed_AddMetaDataTransitionLevel)
	C(wed_UpdateMetadata)
	C(wed_autoOpenLibPane)
	C(wed_autoOpenPropPane)
	C(wed_autoClosePane)
	C(wed_HelpManual)
	C(wed_HelpScenery)
	C(wed_OSMFixTheMap)
	C(wed_ESRIUses)
	{ nullptr, 0 }
};

#undef C

const WED_MCPCommandName *	WED_MCP_GetCommandNames(void)
{
	return kCommands;
}

int		WED_MCP_FindCommand(const string& name)
{
	for(const WED_MCPCommandName * c = kCommands; c->name; ++c)
		if (name == c->name)
			return c->cmd;
	return 0;
}

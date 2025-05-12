cmake_minimum_required(VERSION 3.10)
project(xptools)

include(cmake/xib_helper.cmake)

macro(mac_copy_bundle_files __TARGET __LOCATION __RESOURCES)

    foreach(__RESOURCE ${__RESOURCES})
        get_filename_component(__NAME ${__RESOURCE} NAME)

        if(IS_DIRECTORY "${__RESOURCE}")
            add_custom_command(TARGET ${__TARGET} POST_BUILD
                    COMMAND rm -rf "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    COMMAND cp -a "${__RESOURCE}" "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    VERBATIM)
        else()
            add_custom_command(TARGET ${__TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${__RESOURCE}" "$<TARGET_FILE_DIR:${__TARGET}>/../${__LOCATION}/${__NAME}"
                    COMMENT "Copying {__RESOURCE} to bundle")
        endif()
    endforeach()
endmacro()

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
	find_library(CARBON_FRAMEWORK Carbon REQUIRED)
	find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
	find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
endif()

find_package(expat REQUIRED)
find_package(proj REQUIRED)
find_package(freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(geotiff)
find_package(JPEG REQUIRED)
find_package(libsquish)

add_xib(${CMAKE_SOURCE_DIR}/src/WEDResources/WEDMainMenu.xib)
finalize_xibs(XIBS)

set (WED_SOURCES
	# ifdef PLAT_MINGW
	# src/UI/XWin.win.cpp
	# src/UI/XWinGL.win.cpp
	# src/UI/XWin32DND.cpp
	# src/Utils/PlatformUtils.win.cpp
	# src/Network/PCSBSocket.win.cpp
	# src/Network/PCSBSocketUDP.win.cpp
	# endif #PLAT_MINGW

	# ifdef PLAT_LINUX
	# src/linuxinit/initializer.cpp
	# src/Utils/PlatformUtils.lin.cpp
	# #src/Network/PCSBSocket.lin.cpp
	# #src/Network/PCSBSocketUDP.lin.cpp
	# src/UI/XWin.lin.cpp
	# src/UI/XWinGL.lin.cpp
	# src/Utils/glew.c
	# endif #PLAT_LINUX

	# ifdef PLAT_DARWIN
	${XIBS}
	src/UI/XWin.mac.mm
	src/UI/XWinGL.mac.mm
	src/UI/ObjCUtils.mm
	src/Utils/PlatformUtils.mac.mm
	src/Network/PCSBSocket.lin.cpp
	src/Network/PCSBSocketUDP.lin.cpp
	# endif #PLAT_DARWIN

	src/WEDCore/WED_AppMain.cpp
	src/WEDCore/WED_Application.cpp
	src/WEDCore/WED_PackageMgr.cpp
	src/WEDCore/WED_Archive.cpp
	src/WEDCore/WED_Buffer.cpp
	src/WEDCore/WED_Clipping.cpp
	src/WEDCore/WED_Document.cpp
	src/WEDCore/WED_EnumSystem.cpp
	src/WEDCore/WED_Errors.cpp
	src/WEDCore/WED_FastBuffer.cpp
	src/WEDCore/WED_Globals.cpp
	src/WEDCore/WED_HierarchyUtils.cpp
	src/WEDCore/WED_LibraryMgr.cpp
	src/WEDCore/WED_Persistent.cpp
	src/WEDCore/WED_PropertyHelper.cpp
	src/WEDCore/WED_TexMgr.cpp
	src/WEDCore/WED_UndoLayer.cpp
	src/WEDCore/WED_UndoMgr.cpp
	src/WEDCore/WED_Assert.cpp
	src/WEDCore/WED_ResourceMgr.cpp
	#src/WEDCore/WED_Routing.cpp
	src/WEDCore/WED_Sign_Parser.cpp
	src/WEDCore/WED_GISUtils.cpp
	src/WEDCore/WED_Orthophoto.cpp
	src/WEDCore/WED_Validate.cpp
	src/WEDCore/WED_ValidateATCRunwayChecks.cpp
	src/WEDCore/WED_ValidateList.cpp
	src/WEDCore/WED_XMLReader.cpp
	src/WEDCore/WED_XMLWriter.cpp
	src/WEDEntities/WED_AirportBeacon.cpp
	src/WEDEntities/WED_AirportBoundary.cpp
	src/WEDEntities/WED_AirportChain.cpp
	src/WEDEntities/WED_Airport.cpp
	src/WEDEntities/WED_AirportNode.cpp
	src/WEDEntities/WED_AirportSign.cpp
	src/WEDEntities/WED_AutogenPlacement.cpp
	src/WEDEntities/WED_AutogenNode.cpp
	src/WEDEntities/WED_ATCFlow.cpp
	src/WEDEntities/WED_ATCFrequency.cpp
	src/WEDEntities/WED_ATCRunwayUse.cpp
	src/WEDEntities/WED_ATCTimeRule.cpp
	src/WEDEntities/WED_ATCWindRule.cpp
	src/WEDEntities/WED_DrapedOrthophoto.cpp
	src/WEDEntities/WED_ExclusionPoly.cpp
	src/WEDEntities/WED_ExclusionZone.cpp
	src/WEDEntities/WED_TextureBezierNode.cpp
	src/WEDEntities/WED_TruckDestination.cpp
	src/WEDEntities/WED_TruckParkingLocation.cpp
	src/WEDEntities/WED_TerPlacement.cpp
	src/WEDEntities/WED_GISBoundingBox.cpp
	src/WEDEntities/WED_Entity.cpp
	src/WEDEntities/WED_FacadeNode.cpp
	src/WEDEntities/WED_FacadePlacement.cpp
	src/WEDEntities/WED_FacadeRing.cpp
	src/WEDEntities/WED_FacadePreview.cpp
	src/WEDEntities/WED_ForestPlacement.cpp
	src/WEDEntities/WED_ForestRing.cpp
	src/WEDEntities/WED_GISChain.cpp
	src/WEDEntities/WED_GISComposite.cpp
	src/WEDEntities/WED_GISEdge.cpp
	src/WEDEntities/WED_GISLine.cpp
	src/WEDEntities/WED_GISLine_Width.cpp
	src/WEDEntities/WED_GISPoint_Bezier.cpp
	src/WEDEntities/WED_GISPoint.cpp
	src/WEDEntities/WED_GISPoint_Heading.cpp
	src/WEDEntities/WED_GISPoint_HeadingWidthLength.cpp
	src/WEDEntities/WED_GISPolygon.cpp
	src/WEDEntities/WED_Group.cpp
	src/WEDEntities/WED_Helipad.cpp
	src/WEDEntities/WED_KeyObjects.cpp
	src/WEDEntities/WED_LightFixture.cpp
	src/WEDEntities/WED_LinePlacement.cpp
	src/WEDEntities/WED_ObjPlacement.cpp
	src/WEDEntities/WED_TerPlacement.cpp
	src/WEDEntities/WED_OverlayImage.cpp
	src/WEDEntities/WED_PolygonPlacement.cpp
	src/WEDEntities/WED_RampPosition.cpp
	src/WEDEntities/WED_Ring.cpp
	src/WEDEntities/WED_RoadEdge.cpp
	src/WEDEntities/WED_RoadNode.cpp
	src/WEDEntities/WED_Root.cpp
	src/WEDEntities/WED_Runway.cpp
	src/WEDEntities/WED_RunwayNode.cpp
	src/WEDEntities/WED_Sealane.cpp
	src/WEDEntities/WED_Select.cpp
	src/WEDEntities/WED_SimpleBezierBoundaryNode.cpp
	src/WEDEntities/WED_SimpleBoundaryNode.cpp
	src/WEDEntities/WED_StringPlacement.cpp
	src/WEDEntities/WED_ShapePlacement.cpp
	src/WEDEntities/WED_ShapeNode.cpp
	src/WEDEntities/WED_TaxiRoute.cpp
	src/WEDEntities/WED_TaxiRouteNode.cpp
	src/WEDEntities/WED_Taxiway.cpp
	src/WEDEntities/WED_TextureNode.cpp
	src/WEDEntities/WED_Thing.cpp
	src/WEDEntities/WED_TowerViewpoint.cpp
	src/WEDEntities/WED_Windsock.cpp
	src/WEDFileCache/CACHE_CacheObject.cpp
	src/WEDFileCache/CACHE_DomainPolicy.cpp
	src/WEDFileCache/WED_FileCache.cpp
	src/WEDImportExport/WED_AptIE.cpp
	src/WEDImportExport/WED_AptImportDialog.cpp
	src/WEDImportExport/WED_AptTable.cpp
	src/WEDImportExport/WED_DSFExport.cpp
	src/WEDImportExport/WED_DSFImport.cpp
	src/WEDImportExport/WED_GatewayExport.cpp
	src/WEDImportExport/WED_GatewayImport.cpp
	src/WEDImportExport/WED_ICAOTable.cpp
	src/WEDImportExport/WED_MetaDataDefaults.cpp
	src/WEDImportExport/WED_MetaDataKeys.cpp
	src/WEDImportExport/WED_MetadataUpdate.cpp
	src/WEDImportExport/WED_OrthoExport.cpp
	src/WEDImportExport/WED_SceneryImport.cpp
	src/WEDImportExport/WED_SceneryPackExport.cpp
	src/WEDImportExport/WED_VerTable.cpp
	src/WEDLibrary/WED_LibraryListAdapter.cpp
	src/WEDLibrary/WED_LibraryPreviewPane.cpp
	src/WEDLibrary/WED_LibraryPane.cpp
	src/WEDMap/WED_Colors.cpp
	src/WEDMap/WED_CreateEdgeTool.cpp
	src/WEDMap/WED_CreateLineTool.cpp
	src/WEDMap/WED_CreatePointTool.cpp
	src/WEDMap/WED_CreatePolygonTool.cpp
	src/WEDMap/WED_CreateToolBase.cpp
	src/WEDMap/WED_CreateBoxTool.cpp
	src/WEDMap/WED_DebugLayer.cpp
	src/WEDMap/WED_HandleToolBase.cpp
	src/WEDMap/WED_Map.cpp
	src/WEDMap/WED_MapBkgnd.cpp
	src/WEDMap/WED_MapLayer.cpp
	src/WEDMap/WED_MapPane.cpp
	src/WEDMap/WED_MapPreviewPane.cpp
	src/WEDMap/WED_MapPreviewWindow.cpp
	src/WEDMap/WED_MapToolNew.cpp
	src/WEDMap/WED_MapZoomerNew.cpp
	src/WEDMap/WED_MarqueeTool.cpp
	src/WEDMap/WED_PerspectiveCamera.cpp
	src/WEDMap/WED_StructureLayer.cpp
	#src/WEDMap/WED_TerraserverLayer.cpp
	src/WEDMap/WED_ToolInfoAdapter.cpp
	src/WEDMap/WED_ToolUtils.cpp
	src/WEDMap/WED_UIMeasurements.cpp
	src/WEDMap/WED_VertexTool.cpp
	src/WEDMap/WED_WorldMapLayer.cpp
	src/WEDMap/WED_NavaidLayer.cpp
	src/WEDMap/WED_TerrainLayer.cpp
	src/WEDMap/WED_DrawUtils.cpp
	src/WEDMap/WED_PreviewLayer.cpp
	src/WEDMap/WED_ATCLayer.cpp
	src/WEDMap/WED_BoundaryLayer.cpp
	src/WEDMap/WED_SlippyMap.cpp
	#src/WEDNetwork/WED_Connection.cpp
	#src/WEDNetwork/WED_NWInfoLayer.cpp
	#src/WEDNetwork/WED_NWLinkAdapter.cpp
	src/WEDNetwork/RAII_Classes.cpp
	#src/WEDNetwork/WED_Server.cpp
	src/WEDTCE/WED_TCE.cpp
	src/WEDCore/WED_TCEDebugLayer.cpp
	src/WEDTCE/WED_TCELayer.cpp
	src/WEDTCE/WED_TCEPane.cpp
	src/WEDTCE/WED_TCEToolAdapter.cpp
	src/WEDTCE/WED_TCEToolNew.cpp
	src/WEDTCE/WED_TCEMarqueeTool.cpp
	src/WEDTCE/WED_TCEVertexTool.cpp
	src/WEDProperties/WED_PropertyPane.cpp
	src/WEDProperties/WED_PropertyTable.cpp
	src/WEDWindows/WED_AboutBox.cpp
	src/WEDWindows/WED_DocumentWindow.cpp
	src/WEDWindows/WED_LibraryFilterBar.cpp
	src/WEDWindows/WED_ConvertCommands.cpp
	src/WEDWindows/WED_GroupCommands.cpp
	src/WEDWindows/WED_Menus.cpp
	src/WEDWindows/WED_PackageListAdapter.cpp
	src/WEDWindows/WED_StartWindow.cpp
	src/WEDWindows/WED_Sign_Editor.cpp
	src/WEDWindows/WED_Line_Selector.cpp
	src/WEDWindows/WED_Road_Selector.cpp
	src/GUI/GUI_Broadcaster.cpp
	src/GUI/GUI_Button.cpp
	src/GUI/GUI_ChangeView.cpp
	src/GUI/GUI_Commander.cpp
	src/GUI/GUI_Control.cpp
	src/GUI/GUI_Destroyable.cpp
	src/GUI/GUI_DrawUtils.cpp
	src/GUI/GUI_FilterBar.cpp
	src/GUI/GUI_Fonts.cpp
	src/GUI/GUI_FormWindow.cpp
	src/GUI/GUI_Label.cpp
	src/GUI/GUI_GraphState.cpp
	src/GUI/GUI_Listener.cpp
	src/GUI/GUI_MemoryHog.cpp
	src/GUI/GUI_Packer.cpp
	src/GUI/GUI_PopupButton.cpp
	src/GUI/GUI_Prefs.cpp
	src/GUI/GUI_Resources.cpp
	src/GUI/GUI_ScrollBar.cpp
	src/GUI/GUI_ScrollerPane.cpp
	src/GUI/GUI_SimpleScroller.cpp
	src/GUI/GUI_SimpleTableGeometry.cpp
	src/GUI/GUI_Splitter.cpp
	src/GUI/GUI_TabControl.cpp
	src/GUI/GUI_Table.cpp
	src/GUI/GUI_TabPane.cpp
	src/GUI/GUI_TextField.cpp
	src/GUI/GUI_TextTable.cpp
	src/GUI/GUI_Timer.cpp
	src/GUI/GUI_ToolBar.cpp
	src/GUI/GUI_Window.cpp
	src/GUI/GUI_Pane.cpp
	src/GUI/GUI_Help.cpp
	src/GUI/GUI_Clipboard.cpp
	src/GUI/GUI_Unicode.cpp
	src/GUI/GUI_Application.cpp
	src/Utils/AssertUtils.cpp
	src/Utils/CmdLine.cpp
	src/Utils/MemFileUtils.cpp
	src/Utils/FileUtils.cpp
	src/Utils/GISUtils.cpp
	src/Utils/BitmapUtils.cpp
	src/Utils/TexUtils.cpp
	src/Utils/EndianUtils.c
	src/Utils/md5.c
	src/Utils/XChunkyFileUtils.cpp
	src/Utils/CompGeomUtils.cpp
	src/Utils/zip.c
	src/Utils/unzip.c
	src/Utils/BWImage.cpp
	src/Utils/ObjUtils.cpp
	src/Utils/MatrixUtils.cpp
	src/Utils/CSVParser.cpp
	src/Utils/STLUtils.cpp
	src/Obj/ObjPointPool.cpp
	src/Obj/XObjDefs.cpp
	src/Obj/XObjReadWrite.cpp
	src/Obj/ObjDraw.cpp
	src/Network/b64.c
	src/Network/curl_http.cpp
	src/DSF/DSFLib.cpp
	src/DSF/DSFLibWrite.cpp
	src/DSF/DSFPointPool.cpp
	src/DSF/tri_stripper_101/tri_stripper.cpp
	src/DSFTools/DSF2Text.cpp
	src/OGLE/ogle.cpp
	src/XESCore/AptIO.cpp
	src/XESCore/DEMDefs.cpp

	SDK/lib_json/src/lib_json/json_writer.cpp
	SDK/lib_json/src/lib_json/json_reader.cpp
	SDK/lib_json/src/lib_json/json_value.cpp

	src/lzma19/C/7zArcIn.c
	src/lzma19/C/7zAlloc.c
	src/lzma19/C/7zBuf.c
	src/lzma19/C/7zCrc.c
	src/lzma19/C/7zCrcOpt.c
	src/lzma19/C/7zDec.c
	src/lzma19/C/7zFile.c
	src/lzma19/C/7zStream.c
	src/lzma19/C/Bcj2.c
	src/lzma19/C/Bra.c
	src/lzma19/C/Bra86.c
	src/lzma19/C/BraIA64.c
	src/lzma19/C/CpuArch.c
	src/lzma19/C/Delta.c
	src/lzma19/C/LzmaDec.c
	src/lzma19/C/Lzma2Dec.c

	SDK/libtess2/Source/tess.c
	SDK/libtess2/Source/bucketalloc.c
	SDK/libtess2/Source/dict.c
	SDK/libtess2/Source/mesh.c
	SDK/libtess2/Source/priorityq.c
	SDK/libtess2/Source/sweep.c
	SDK/libtess2/Source/geom.c
)

set(WED_RESOURCE_FILES
	# ifdef PLAT_LINUX
	# ${CMAKE_SOURCE_DIR}/src/WEDResources/WED_icon.png
	# else
	${XIBS}
	${CMAKE_SOURCE_DIR}/src/WEDResources/WED.icns
	# endif
	${CMAKE_SOURCE_DIR}/src/WEDResources/sans.ttf
	# ${CMAKE_SOURCE_DIR}/src/WEDResources/pushbutton.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/push_buttons.png
	# ${CMAKE_SOURCE_DIR}/src/WEDResources/checkbutton.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/check_buttons.png
	# ${CMAKE_SOURCE_DIR}/src/WEDResources/radiobutton.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tabs.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/splitter_h.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/splitter_v.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/scrollbar_h.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/scrollbar_v.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/scroll_btn_h.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/scroll_btn_v.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/scrollbar_corner.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/about.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/COPYING
	${CMAKE_SOURCE_DIR}/src/WEDResources/colors.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/taxi_sign.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/ui_measurements.txt
	${CMAKE_SOURCE_DIR}/src/WEDResources/map_tools.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tilt_tool.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tce_tools.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/exclaim.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/startup_bar.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/startup_btns.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/startup_bkgnd.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_vertexsharp.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_cross.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_square.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_vertexround.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_arrowhead.png
	# ${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_arrowhead_lg.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_rotatehead.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_closeloop.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/handles/handle_control.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_heliport.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_missing_obj.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_light.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_rampstart.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_airport.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_beacon.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_helipad.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_windsock.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_seaport.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_taxisign.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_towerview.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_servicedest.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/map_tree.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/nav_ndb.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/nav_vor.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/nav_mark.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/nav_gs.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/eye.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/lock.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/check.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/gradient.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/gradient_light.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/disclose.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/header.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/sider.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/property_bar.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/arrows.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/delete.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/seasons.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/variants.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/regions.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/tables/customized.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/worldmap.jpg
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/dirt.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/gravel.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/water.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/snow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/concrete.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/lake.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/grass.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/pavement/asphalt.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BDoubleSolidYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_RunwayHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_WideBrokenSingle.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_WideBrokenDouble.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_DoubleSolidYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_HoldLights.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BoundaryEdge.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BrokenYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_ILSHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BWideBrokenDouble.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_TaxiCenter.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BILSHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_HoldShortCenter.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_HoldLightsPulse.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_OtherHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_TaxiEdge.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_ILSCriticalCenter.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BBrokenYellow.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BRunwayHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BOtherHold.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BWideBrokenSingle.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BILSCriticalCenter.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_Chequered.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidWhite.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BrokenWhite.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_Unknown.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidRed.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidRed.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BrokenRed.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BBrokenRed.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidOrange.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidOrange.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidGreen.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidGreen.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidBlue.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidBlue.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_SolidYellowW.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidYellowW.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_ILSCriticalCenterW.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BILSCriticalCenterW.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_TaxiCenterUni.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_HoldShortCenterUni.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/line_types/line_BSolidWhite.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassA.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassB.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassC.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassD.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassE.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/parking_spots/ClassF.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/navmap_airport.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/icons/navmap_seaport.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/ArrowHeadRoadS.png
	${CMAKE_SOURCE_DIR}/src/WEDResources/ArrowHeadRoadE.png
)

add_executable(WED MACOSX_BUNDLE ${WED_SOURCES})
target_compile_definitions(WED PRIVATE -DLIN=0 -DIBM=0 -DAPL=1 -DDEV=1 -DUSE_JPEG=1 -DUSE_TIF=1 -DWED=1 -DNO_CGAL=1)
target_compile_options(WED PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
target_include_directories(WED PRIVATE
	SDK/lib_json/include
	src/lzma19/C
	SDK/libtess2/Include
	src/WEDTCE
	src/DSF
	src/DSFTools
	src/GUI
	src/GUI/mmenu
	src/Interfaces
	src/Obj
	src/ObjEdit
	src/OGLE
	src/RenderFarmUI
	src/UI
	src/WEDCore
	src/WEDEntities
	src/WEDFileCache
	src/WEDImportExport
	src/WEDLibrary
	src/WEDMap
	src/WEDNetwork
	src/WEDProperties
	${CMAKE_SOURCE_DIR}/src/WEDResources
	src/WEDWindows
	src/Utils
	src/XESCore
	src/XESTools
	src/Network
	src/DSF/tri_stripper_101
	src/RawImport
	src/Tiger
	src/SDTS
)

target_link_libraries(WED PRIVATE
	${CARBON_FRAMEWORK}
	${OPENGL_FRAMEWORK}
	${APPKIT_FRAMEWORK}
	expat::expat
	ZLIB::ZLIB
	PNG::PNG
	PROJ::proj
	Freetype::Freetype
	TIFF::TIFF
	geotiff_library
	JPEG::JPEG
	libsquish::libsquish
	curl
)

mac_copy_bundle_files(WED Resources "${WED_RESOURCE_FILES}")

set_target_properties(WED PROPERTIES
	MACOSX_BUNDLE TRUE
	MACOSX_FRAMEWORK_IDENTIFIER org.LaminarResearch.WED
)

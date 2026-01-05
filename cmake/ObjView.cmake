set(obj_view_sources
    src/Obj/XObjDefs.cpp
    src/Obj/XObjDefs.h
    src/Obj/XObjReadWrite.cpp
    src/Obj/XObjReadWrite.h
    src/Obj/ObjDraw.cpp
    src/Obj/ObjDraw.h
    src/Obj/ObjPointPool.cpp
    src/Obj/ObjPointPool.h
    src/ObjEdit/OE_Zoomer3d.cpp
    src/ObjEdit/OE_Zoomer3d.h
    src/Utils/ObjUtils.cpp
    src/Utils/ObjUtils.h
    src/Utils/AssertUtils.cpp
    src/Utils/AssertUtils.h
    src/Utils/BitmapUtils.cpp
    src/Utils/BitmapUtils.h
    src/Utils/EndianUtils.c
    src/Utils/EndianUtils.h
    src/Utils/FileUtils.cpp
    src/Utils/FileUtils.h
    src/Utils/GeoUtils.cpp
    src/Utils/GeoUtils.h
    src/Utils/MatrixUtils.cpp
    src/Utils/MatrixUtils.h
    src/Utils/MemFileUtils.cpp
    src/Utils/MemFileUtils.h
    src/Utils/trackball.c
    src/Utils/trackball.h
    src/Utils/TexUtils.cpp
    src/Utils/TexUtils.h
    src/Utils/XUtils.cpp
    src/Utils/XUtils.h
    src/Utils/unzip.c
    src/Utils/unzip.h
    src/Utils/zip.c
    src/Utils/zip.h
    src/UI/XGUIApp.cpp
    src/UI/XGUIApp.h
    src/XPTools/ViewObj.cpp

    SDK/libtess2/Source/tess.c
    SDK/libtess2/Source/tess.h
    SDK/libtess2/Source/bucketalloc.c
    SDK/libtess2/Source/bucketalloc.h
    SDK/libtess2/Source/dict.c
    SDK/libtess2/Source/dict.h
    SDK/libtess2/Source/mesh.c
    SDK/libtess2/Source/mesh.h
    SDK/libtess2/Source/priorityq.c
    SDK/libtess2/Source/priorityq.h
    SDK/libtess2/Source/sweep.c
    SDK/libtess2/Source/sweep.h
    SDK/libtess2/Source/geom.c
    SDK/libtess2/Source/geom.h
)

if (WIN32)
    set(obj_view_sources ${obj_view_sources}
        src/Utils/PlatformUtils.win.cpp
        src/UI/XWin.win.cpp
        src/UI/XWin32DND.cpp
        src/UI/XWinGL.win.cpp
        src/GUI/GUI_Unicode.cpp
    )
elseif(LINUX)
    set(obj_view_sources ${obj_view_sources}
        src/Utils/PlatformUtils.lin.cpp
        src/UI/XWin.lin.cpp
        src/UI/XWinGL.lin.cpp
        src/Utils/glew.c
    )
elseif(APPLE)
    set(obj_view_sources ${obj_view_sources}
        src/Utils/PlatformUtils.mac.mm
        src/UI/XWin.mac.mm
        src/UI/XWinGL.mac.mm
        src/UI/ObjCUtils.mm
    )
endif()

if(MSVC)
    add_executable(ObjView WIN32 ${obj_view_sources})
	target_compile_options(ObjView PRIVATE /FI ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
else()
    add_executable(ObjView ${obj_view_sources})
	target_compile_options(ObjView PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
endif()

target_compile_definitions(ObjView PRIVATE ${BASIC_PLATFORM_DEFINES} -DUSE_JPEG=1 -DUSE_TIF=1)
target_link_libraries(ObjView PRIVATE
    ZLIB::ZLIB
	PNG::PNG
	TIFF::TIFF
	JPEG::JPEG
	libsquish::libsquish
	opengl::opengl
	GLEW::glew_s
)

target_include_directories(ObjView PRIVATE
	SDK/libtess2/Include
	src/GUI
    src/Obj
    src/ObjEdit
    src/Utils
    src/UI
)

if (APPLE)
	target_link_libraries(ObjView PRIVATE
		${CARBON_FRAMEWORK}
		${APPKIT_FRAMEWORK}
	)
elseif(LINUX)
	target_link_libraries(ObjView PRIVATE fltk::fltk egl::egl)
endif()

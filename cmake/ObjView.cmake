set(obj_view_sources
    src/Obj/XObjDefs.cpp
    src/Obj/XObjReadWrite.cpp
    src/Obj/ObjDraw.cpp
    src/Obj/ObjPointPool.cpp
    src/ObjEdit/OE_Zoomer3d.cpp
    src/Utils/ObjUtils.cpp
    src/Utils/AssertUtils.cpp
    src/Utils/BitmapUtils.cpp
    src/Utils/EndianUtils.c
    src/Utils/FileUtils.cpp
    src/Utils/GeoUtils.cpp
    src/Utils/MatrixUtils.cpp
    src/Utils/MemFileUtils.cpp
    src/Utils/trackball.c
    src/Utils/TexUtils.cpp
    src/Utils/XUtils.cpp
    src/Utils/unzip.c
    src/Utils/zip.c
    src/UI/XGUIApp.cpp
    src/XPTools/ViewObj.cpp

    SDK/libtess2/Source/tess.c
    SDK/libtess2/Source/bucketalloc.c
    SDK/libtess2/Source/dict.c
    SDK/libtess2/Source/mesh.c
    SDK/libtess2/Source/priorityq.c
    SDK/libtess2/Source/sweep.c
    SDK/libtess2/Source/geom.c
)

if (WIN32)
    set(obj_view_sources ${obj_view_sources}
        src/Utils/PlatformUtils.win.cpp
        src/UI/XWin.win.cpp
        src/UI/XWin32DND.cpp
        src/UI/XWinGL.win.cpp
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

add_executable(ObjView ${obj_view_sources})
target_compile_options(ObjView PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
target_compile_definitions(ObjView PRIVATE ${BASIC_PLATFORM_DEFINES} -DUSE_JPEG=1 -DUSE_TIF=1)
target_link_libraries(ObjView PRIVATE
    ZLIB::ZLIB
	PNG::PNG
	TIFF::TIFF
	JPEG::JPEG
	libsquish::libsquish
)

target_include_directories(ObjView PRIVATE
	SDK/libtess2/Include
    Src/Obj
    Src/ObjEdit
    Src/Utils
    Src/UI
)

if (APPLE)
	target_link_libraries(ObjView PRIVATE
		${CARBON_FRAMEWORK}
		${OPENGL_FRAMEWORK}
		${APPKIT_FRAMEWORK}
	)
endif()

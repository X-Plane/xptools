set(XGRINDER_SOURCES
    src/Utils/EndianUtils.c
    src/Utils/zip.c
    src/Utils/AssertUtils.cpp
    src/UI/XGrinderApp.cpp
    src/Utils/XUtils.cpp
    src/Utils/MemFileUtils.cpp
    src/Utils/FileUtils.cpp
    src/GUI/GUI_Unicode.cpp
    src/Utils/unzip.c
    src/XPTools/XGrinderShell.cpp
)

if (APPLE)
    set(XGRINDER_SOURCES ${XGRINDER_SOURCES}
        src/Utils/PlatformUtils.mac.mm
        src/UI/XWin.mac.mm
        src/UI/ObjCUtils.mm
    )
elseif(LINUX)
    set(XGRINDER_SOURCES ${XGRINDER_SOURCES}
        src/Utils/PlatformUtils.lin.cpp
        src/UI/XWin.lin.cpp
    )
elseif(WIN32)
    set(XGRINDER_SOURCES ${XGRINDER_SOURCES}
        src/Utils/PlatformUtils.win.cpp
        src/UI/XWin.win.cpp
        src/UI/XWin32DND.cpp
    )
endif()

add_executable(XGrinder ${XGRINDER_SOURCES})
target_compile_options(XGrinder PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
target_compile_definitions(XGrinder PRIVATE ${BASIC_PLATFORM_DEFINES})
target_link_libraries(XGrinder PRIVATE
    ZLIB::ZLIB
)

target_include_directories(XGrinder PRIVATE
    Src/Utils
    Src/Obj
    Src/UI
)

if (APPLE)
	target_link_libraries(XGrinder PRIVATE
		${CARBON_FRAMEWORK}
		${APPKIT_FRAMEWORK}
	)
endif()

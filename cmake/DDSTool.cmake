set(dds_tool_sources
    src/Utils/AssertUtils.cpp
    src/Utils/AssertUtils.h
    src/XPTools/DDSTool.cpp
    src/Utils/EndianUtils.c
    src/Utils/EndianUtils.h
    src/Utils/zip.c
    src/Utils/zip.h
    src/Utils/unzip.c
    src/Utils/unzip.h
    src/Utils/BitmapUtils.cpp
    src/Utils/BitmapUtils.h
    src/Utils/QuiltUtils.cpp
    src/Utils/QuiltUtils.h
    src/Utils/FileUtils.cpp
    src/Utils/FileUtils.h
    src/GUI/GUI_Unicode.cpp
    src/GUI/GUI_Unicode.h
)

add_executable(DDSTool ${dds_tool_sources})
target_include_directories(DDSTool PRIVATE
	src/Utils
	src/GUI
)

target_compile_definitions(DDSTool PRIVATE
	${BASIC_PLATFORM_DEFINES}
    -DUSE_JPEG=1
    -DUSE_TIF=1
)

target_link_libraries(DDSTool PRIVATE
	libsquish::libsquish
	TIFF::TIFF
	JPEG::JPEG
	PNG::PNG
	ZLIB::ZLIB
)

if(MSVC)
	target_compile_options(DDSTool PRIVATE /FI ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
else()
	target_compile_options(DDSTool PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
endif()

if (APPLE)
	target_link_libraries(DDSTool PRIVATE ${CARBON_FRAMEWORK})
endif()

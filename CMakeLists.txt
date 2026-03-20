cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0091 NEW) # Required by conan 2

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING
    "Minimum macOS deployment version")
endif()

project(xptools)

if (APPLE)
	include(cmake/mac.cmake)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(expat REQUIRED)
find_package(proj REQUIRED)
find_package(freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(geotiff REQUIRED)
find_package(JPEG REQUIRED)
find_package(libsquish REQUIRED)
find_package(shapelib REQUIRED)
find_package(CURL REQUIRED)
find_package(opengl_system REQUIRED)
find_package(glew REQUIRED)
find_package(jsoncpp REQUIRED)

if (APPLE)
	find_library(CARBON_FRAMEWORK Carbon REQUIRED)
	find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
elseif(LINUX)
    find_package(glu REQUIRED)
    find_package(fltk REQUIRED)
    find_package(egl REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client wayland-cursor)
    pkg_check_modules(WAYLAND_EGL REQUIRED IMPORTED_TARGET wayland-egl)
    pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
    pkg_check_modules(PANGO REQUIRED IMPORTED_TARGET pangocairo)
    pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
endif()

if(WIN32)
    set(BASIC_PLATFORM_DEFINES
        LIN=0
        IBM=1
        APL=0
    )
elseif(APPLE)
    set(BASIC_PLATFORM_DEFINES
        LIN=0
        IBM=0
        APL=1
        _LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION=1
    )
elseif(UNIX)
    set(BASIC_PLATFORM_DEFINES
        LIN=1
        IBM=0
        APL=0
        _LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION=1
    )
endif()

# Build-type specific definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(BASIC_PLATFORM_DEFINES "${BASIC_PLATFORM_DEFINES};DEV=1;BIG=0;LIL=1")
else()
	set(BASIC_PLATFORM_DEFINES "${BASIC_PLATFORM_DEFINES};DEV=0;BIG=0;LIL=1")
endif()

include(cmake/DDSTool.cmake)
include(cmake/DSFTool.cmake)
include(cmake/ObjView.cmake)
include(cmake/WED.cmake)
include(cmake/XGrinder.cmake)
include(cmake/OneOffs.cmake)

source_group(TREE "${PROJECT_SOURCE_DIR}")

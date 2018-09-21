cmake_minimum_required(VERSION 3.7)
project(Twist VERSION 2.0 LANGUAGES C CXX)

set(TWIST_VERSION ${PROJECT_VERSION})
set(TWIST_NAME "${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32 OR MINGW)
	add_definitions(-DWINDOWS)
	add_definitions(-DSDL_MAIN_HANDLED)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_definitions(-DJACK_HAS_PORT_RENAME)
add_definitions(-DTWIST_NAME="${TWIST_NAME}")
add_definitions(-DTWIST_VERSION="${TWIST_VERSION}")

set(BUILD_SHARED_LIBS CACHE BOOL OFF)
set(BUILD_TESTING CACHE BOOL OFF)
set(BUILD_PROGRAMS CACHE BOOL OFF)
set(BUILD_EXAMPLES CACHE BOOL OFF)
set(BUILD_REGTEST CACHE BOOL OFF)
set(ENABLE_PACKAGE_CONFIG CACHE BOOL OFF)
set(BUILD_SHARED_LIBS CACHE BOOL OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/rtmidi")
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/libsndfile")

find_package(SDL2 REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/editor/imgui")
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/osdialog")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/twen")

include_directories(
	${SDL2_INCLUDE_DIRS}
	${LIBSNDFILE_INCLUDE_DIRS}
	"deps/libsndfile/src"
	"deps/rtmidi"
	"deps/osdialog"
	"src/"
)

file(GLOB SRC
	"src/editor/*.h"
	"src/editor/*.cpp"
	"src/editor/*.hpp"
	"src/editor/imgui/*.h"
	"src/editor/imgui/*.cpp"
	"src/editor/imgui/*.hpp"
	"src/editor/nodes/*.hpp"
	"src/glad/*.h"
	"src/glad/*.cpp"
	"src/stb/*.h"
	"src/stb/*.cpp"
	"src/main.cpp"
)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARIES}
	sndfile
	osdialog
	rtmidi_static
	ImGUI
	twen
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()

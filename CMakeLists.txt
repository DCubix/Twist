cmake_minimum_required(VERSION 3.7)
project(Twist VERSION 1.0 LANGUAGES C CXX)

set(TWIST_VERSION ${PROJECT_VERSION})
set(TWIST_NAME "${PROJECT_NAME}")

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32 OR MINGW)
	add_definitions(-DWINDOWS)
	add_definitions(-DSDL_MAIN_HANDLED)
endif()

if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	add_compile_options(-msse3)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	add_compile_options(/QxSSE3)
elseif((CMAKE_CXX_COMPILER_ID MATCHES "MSVC") AND NOT CMAKE_CL_64)
	add_compile_options(/arch:SSE2) # VC doesn't support /arch:SSE3
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_definitions(-DJACK_HAS_PORT_RENAME)
add_definitions(-DTWIST_NAME="${TWIST_NAME}")
add_definitions(-DTWIST_VERSION="${TWIST_VERSION}")

set(BUILD_SHARED_LIBS CACHE BOOL OFF)
set(BUILD_TESTING CACHE BOOL OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/rtmidi")

find_package(SDL2 REQUIRED)
find_package(SndFile REQUIRED)
# find_package(RtMIDI REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/editor/imgui")
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/osdialog")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/twen")

include_directories(
	${SDL2_INCLUDE_DIRS}
	${LIBSNDFILE_INCLUDE_DIRS}
	# ${RTMIDI_INCLUDE_DIR}
	"deps/xsimd/include"
	"deps/rtmidi"
	"deps/osdialog"
	"src/twen"
)

file(GLOB_RECURSE INC
	"src/*.h"
	"src/*.hpp"
)

# message(STATUS "INCLUDES: ${INC}")

file(GLOB_RECURSE SRC
	"src/*.c"
	"src/*.cpp"
)

add_executable(${PROJECT_NAME} ${INC} ${SRC})
target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARIES}
	${LIBSNDFILE_LIBRARIES}
	# ${RTMIDI_LIBRARY}
	osdialog
	rtmidi_static
	ImGUI
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()
cmake_minimum_required(VERSION 3.7)
project(twist VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
find_package(SndFile REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/editor/imgui/")

include_directories(
	${SDL2_INCLUDE_DIRS}
	${LIBSNDFILE_INCLUDE_DIRS}
)

file(GLOB SRC
	"src/*.h"
	"src/*.cpp"
	"src/glad/*.h"
	"src/glad/*.cpp"
	"src/editor/*.h"
	"src/editor/*.hpp"
	"src/editor/imgui/*.h"
	"src/editor/imgui/*.cpp"
	"src/editor/nodes/*.h"
	"src/editor/nodes/*.c"
	"src/editor/nodes/*.cpp"
	"src/editor/nodes/*.hpp"
	"src/editor/*.cpp"
	"src/editor/*.c"
	"src/stb/*.h"
	"src/stb/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARIES}
	${LIBSNDFILE_LIBRARIES}
	ImGUI
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()
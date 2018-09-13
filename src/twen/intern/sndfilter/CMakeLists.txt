cmake_minimum_required(VERSION 3.7)
project(sndfilter VERSION 1.0 LANGUAGES C)

file(GLOB_RECURSE SRC
	"src/*.h"
	"src/*.c"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC src/)
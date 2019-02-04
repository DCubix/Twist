cmake_minimum_required(VERSION 3.7)
project(taudio VERSION 1.0 LANGUAGES C CXX)

include(CheckIncludeFiles)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (DEFINED MSVC_VERSION)
	if (MSVC_VERSION GREATER_EQUAL 1500)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
	endif()
	if (MSVC_VERSION GREATER_EQUAL 1910)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	endif()
endif()

file(GLOB SRC
	"*.h"
	"*.cpp"
	"intern/*.h"
	"intern/*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
)

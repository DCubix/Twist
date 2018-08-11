cmake_minimum_required(VERSION 3.7)
project(ImGUI)

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

file(GLOB ImGUISRC
	"*.h"
	"*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ImGUISRC})

add_library(ImGUI STATIC ${ImGUISRC})
target_include_directories(ImGUI PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/)
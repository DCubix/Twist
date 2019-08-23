cmake_minimum_required(VERSION 3.7)
project(twen VERSION 1.0 LANGUAGES C CXX)

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

set (SSE_COMPILE_FLAGS "")
option(TWEN_USE_SSE "Use SSE2/3 Instructions where possible." ON)
if (TWEN_USE_SSE)
	if (NOT MSVC)
		set(CMAKE_REQUIRED_C_FLAGS_SAVE ${CMAKE_REQUIRED_C_FLAGS})
		set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS} "-msse3")
		CHECK_INCLUDE_FILES("pmmintrin.h" HAVE_SSE3_H)
		set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS_SAVE})
		if (HAVE_SSE3_H)
			set(HAVE_SSE2_H 1)
			add_definitions(-DUSING_SSE3)
			if (CMAKE_COMPILER_IS_GNUCC)
			#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3")
			#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
				set (SSE_COMPILE_FLAGS "-msse3")
			endif()
		else()
			set (HAVE_SSE3_H 0)
		endif()

		if (NOT HAVE_SSE3_H)
			set(CMAKE_REQUIRED_C_FLAGS_SAVE ${CMAKE_REQUIRED_C_FLAGS})
			set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS} "-msse2")
			CHECK_INCLUDE_FILES("emmintrin.h" HAVE_SSE2_H)
			set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS_SAVE})
			if(HAVE_SSE2_H)
				add_definitions(-DUSING_SSE2)
				if (CMAKE_COMPILER_IS_GNUCC)
				# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
				# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
					set (SSE_COMPILE_FLAGS "-msse2")
				endif()
			else()
				set (HAVE_SSE2_H 0)
			endif()
		endif()
	else()
		set(CMAKE_REQUIRED_C_FLAGS_SAVE ${CMAKE_REQUIRED_C_FLAGS})
		set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS} "/arch:SSE2")
		CHECK_INCLUDE_FILES("intrin.h" HAVE_INTRIN_H)
		set(CMAKE_REQUIRED_C_FLAGS ${CMAKE_REQUIRED_C_FLAGS_SAVE})
		if (HAVE_INTRIN_H)
			add_definitions(-DUSING_SSE2_MSVC)
			#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2")
			#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
			set (SSE_COMPILE_FLAGS " /arch:SSE2 ")
		else()
			set (HAVE_INTRIN_H 0)
		endif()
	endif()
endif()

file(GLOB SRC
	"*.h"
	"*.cpp"
	"nodes/*.hpp"
	"intern/*.h"
	"intern/*.c"
	"intern/*.cpp"
	"intern/*.hpp"
	"intern/termcolor/*.hpp"
)

add_library(${PROJECT_NAME} STATIC ${SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC taudio)
target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
)

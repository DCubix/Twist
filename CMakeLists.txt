cmake_minimum_required(VERSION 3.7)
project(twist VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIRS}
)

file(GLOB SRC
	"src/*.h"
	"src/*.cpp"
	"src/glad/*.h"
	"src/glad/*.cpp"
	"src/editor/*.h"
	"src/editor/*.cpp"
	"src/stb/*.h"
	"src/stb/*.cpp"
)

if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARIES}
)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()
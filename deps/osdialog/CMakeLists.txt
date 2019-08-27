cmake_minimum_required(VERSION 3.7)
project(osdialog LANGUAGES C)

set(INC "osdialog.h")
set(SRC "osdialog.c")
set(LIBS)
set(INCS)
set(DEFS)

if (WIN32)
	list(APPEND SRC "osdialog_win.c")
	set(LIBS "comdlg32")
elseif(UNIX AND NOT APPLE)
	list(APPEND SRC "osdialog_gtk3.c")
	
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

	set(INCS ${GTK3_INCLUDE_DIRS})
	set(LIBS ${GTK3_LIBRARIES})
elseif(APPLE)
	list(APPEND SRC "osdialog_mac.m")
	find_library(APP_KIT AppKit)
	set(LIBS ${APP_KIT})
endif()

add_library(${PROJECT_NAME} STATIC ${SRC} ${INC})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEFS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

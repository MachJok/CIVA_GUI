cmake_minimum_required(VERSION 3.0)

project(multi-window-test VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DDEBUG)


set(SRC
	
	gui_datarefs.cpp
	gui_functions.cpp
	main.cpp

	)

set(HDR

	gui_datarefs.h
	gui_functions.h
	main.h
	source_includes.h
	gui_structs.h
	gui_variables.h

)
if(WIN32)
    set(ARCH "mingw64")
elseif(APPLE)
    set(ARCH "mac64")
else()
    set(ARCH "lin64")
endif()

set(ALL_SRC ${SRC} ${HDR})

list(SORT ALL_SRC)

add_library(multi-window-test SHARED ${ALL_SRC})

set(LIBACFUTILS "${CMAKE_SOURCE_DIR}/libacfutils")
find_library(LIBACFUTILS_LIBS acfutils "${LIBACFUTILS}/${ARCH}/lib")
find_library(LIBCAIRO_LIBS cairo "${LIBACFUTILS}/${ARCH}/lib")
find_library(LIBFREETYPE_LIBS freetype "${LIBACFUTILS}//lib")
set(LIBGLEWMX_LIBS "${LIBACFUTILS}/${ARCH}/lib/libGLEWmx.a")
include_directories(multi-window-test PUBLIC
    "XPLSDK303/CHeaders/XPLM"
    "XPLSDK303/CHeaders/Widgets"
    "XPLSDK303/CHeaders/Wrappers"
    "libacfutils/include"
    "libacfutils/lin64/include"
)

                
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Werror -pedantic --std=c11 \
    -Wno-unused-local-typedefs -Wno-missing-field-initializers -DGLEW_BUILD=GLEW_STATIC -DGLEW_MX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -Wformat-truncation=0 -Wformat-overflow=0 " )

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
if(WIN32)
	add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
elseif(APPLE)
	add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
else()
	add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()



target_link_libraries(multi-window-test PUBLIC
    ${LIBACFUTILS_LIBS}
	${LIBCAIRO_LIBS}
	${LIBFREETYPE_LIBS}
	${LIBGLEWMX_LIBS}
)
set_target_properties(multi-window-test PROPERTIES PREFIX "")
set_target_properties(multi-window-test PROPERTIES SUFFIX "")
set_target_properties(multi-window-test PROPERTIES OUTPUT_NAME
    "multi-window-test.xpl")


	if(WIN32)
	set_target_properties(multi-window-test PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/win_x64" )
	set_target_properties(multi-window-test PROPERTIES
	    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/win_x64" )
elseif(APPLE)
	set_target_properties(multi-window-test PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/mac_x64" )
	set_target_properties(multi-window-test PROPERTIES
	    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/mac_x64" )
else()
	set_target_properties(multi-window-test PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/lin_x64" )
	set_target_properties(multi-window-test PROPERTIES
	    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/multi-window-test/lin_x64" )
endif()
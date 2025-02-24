cmake_minimum_required(VERSION 3.1.0)

# define module name
set (module isi)

# we want to compile all .c files as default
file(GLOB libsources
     source/*.c
     )

# set public headers, these get installed
file(GLOB pub_headers include/*.h)

# define include paths
include_directories(
    include
    include_priv
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/ebase
    ${LIB_ROOT}/${CMAKE_BUILD_TYPE}/include/common
    )

# module specific defines
###add_definitions(-Wno-error=unused-function)
# add lib to build env

# add lib to build env
if(${STATIC_BUILD} STREQUAL 0)
if(${DUMMY_BUILD} STREQUAL 0)
    add_library(${module}_shared SHARED ${libsources})
else(${DUMMY_BUILD} STREQUAL 0)
    add_library(${module}_shared SHARED)
endif(${DUMMY_BUILD} STREQUAL 0)

if(${CMAKE_COMPILE_WARNING_AS_ERROR} STREQUAL "ON")
add_definitions(-Werror)
elseif(${CMAKE_COMPILE_WARNING_AS_ERROR} STREQUAL "OFF")
remove_definitions(-Werror)
endif()

SET_TARGET_PROPERTIES(${module}_shared PROPERTIES OUTPUT_NAME     ${module})
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES LINK_FLAGS      "-shared -fPIC")
SET_TARGET_PROPERTIES(${module}_shared PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")
install(TARGETS ${module}_shared
        PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
        ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        )

elseif(${STATIC_BUILD} STREQUAL 1)
    if(${DUMMY_BUILD} STREQUAL 0)
        add_library(${module}_static STATIC ${libsources})
    else(${DUMMY_BUILD} STREQUAL 0)
        add_library(${module}_static STATIC)
    endif(${DUMMY_BUILD} STREQUAL 0)

    SET_TARGET_PROPERTIES(${module}_static PROPERTIES OUTPUT_NAME     ${module})
    SET_TARGET_PROPERTIES(${module}_static PROPERTIES LINK_FLAGS     "-static -fPIC")
    SET_TARGET_PROPERTIES(${module}_static PROPERTIES FRAMEWORK       TRUE PUBLIC_HEADER "${pub_headers}")

    install(TARGETS ${module}_static
            PUBLIC_HEADER   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${module}
            ARCHIVE         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
            LIBRARY         DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${module}
        )
endif()

# create common targets for this module
include(${PROJECT_ROOT}/targets.cmake)

/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
 
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

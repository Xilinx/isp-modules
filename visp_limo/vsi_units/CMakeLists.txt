cmake_minimum_required(VERSION 3.1.0)

project(UNITS)

add_definitions(-fPIC)

set (module units)

file(GLOB_RECURSE SOURCES "./source/*.c" )

include_directories(./include
                    ./include/cam_device
                    ./include_priv/cam_device
                    ${MBOX}/include
                    ${MEDIACONTROL}/hal/linux/include
                    )

add_library(${module}_shared SHARED ${SOURCES})

target_link_libraries(${module}_shared
                      mbox_shared
                      )

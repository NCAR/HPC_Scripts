cmake_minimum_required(VERSION 2.4)
project(BENCHER)

##There are lots of scripts with cmake
##for finding external libraries. 
##see /usr/local/share/cmake-2.6/Modules/Find*.cmake for more examples
#find_package(GLUT)
#find_package(OpenGL)
#
#set(CMAKE_CXX_FLAGS "-g -Wall")


##There are lots of scripts with cmake
##for finding external libraries. 
##see /usr/local/share/cmake-2.6/Modules/Find*.cmake for more examples
#find_package(GLUT)
#find_package(OpenGL)
#
#set(CMAKE_CXX_FLAGS "-g -Wall")
#add_subdirectory(w01-cpp)

#SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--rpath -Wl,${CMAKE_CURRENT_BINARY_DIR}/lib")


##http://www.cmake.org/cmake/help/v3.0/module/ExternalProject.html
#${CMAKE_SOURCE_DIR}
#${CMAKE_BINARY_DIR}
#
#CMAKE_CURRENT_BINARY_DIR
#
#include(ExternalProject)
#ExternalProject_Add(
#   CMake-3-0-2
#   GIT_REPOSITORY git://cmake.org/cmake.git
#   GIT_TAG v3.0.2
#   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=/tmp/testor/
#   UPDATE_COMMAND ""
#)



add_subdirectory(mvapich)

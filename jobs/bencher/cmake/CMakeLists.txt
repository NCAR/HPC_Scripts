cmake_minimum_required(VERSION 2.6)
project(CMAKE)

set(CMAKE_VERSION "v3.0.2" CACHE STRING "GIT Tag for cmake version")
set(CMAKE_GIT_URL "git://cmake.org/cmake.git" CACHE STRING "GIT repo URL for cmake")

include(ExternalProject)

ExternalProject_Add(
   "CMake-${CMAKE_VERSION}"
   GIT_REPOSITORY "${CMAKE_GIT_URL}"
   GIT_TAG "${CMAKE_VERSION}"
   CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}"
   UPDATE_COMMAND ""
)


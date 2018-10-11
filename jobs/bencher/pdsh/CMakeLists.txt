cmake_minimum_required(VERSION 2.6)
project(pdsh)

set(PDSH_VERSION "pdsh-2.29" CACHE STRING "GIT Tag for pdsh version")
set(PDSH_GIT_URL "https://code.google.com/p/pdsh/" CACHE STRING "GIT repo URL for pdsh")

include(ExternalProject)

ExternalProject_Add(
   ${PDSH_VERSION}
   GIT_REPOSITORY ${PDSH_GIT_URL}
   GIT_TAG ${PDSH_VERSION}
   PREFIX ${CMAKE_CURRENT_BINARY_DIR}/
   #CONFIGURE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-ssh --without-rsh --with-readline"
   CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/pdsh/src/${PDSH_VERSION}/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-ssh --without-rsh --with-readline
   #BUILD_COMMAND make
)


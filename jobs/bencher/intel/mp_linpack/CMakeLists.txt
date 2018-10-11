cmake_minimum_required(VERSION 2.8)
project(intel_mp_linpack)

set(INTEL_LINPACK_URL "http://registrationcenter.intel.com/irc_nas/4547/l_lpk_p_11.2.0.003.tgz" CACHE STRING "URL for Intel MP Linpack tarball")

include(ExternalProject)

ExternalProject_Add(
   intel_mp_linpack
   URL ${INTEL_LINPACK_URL}
   PREFIX ${CMAKE_CURRENT_BINARY_DIR}/
   #CONFIGURE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-ssh --without-rsh --with-readline"
   CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/pdsh/src/${PDSH_VERSION}/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-ssh --without-rsh --with-readline
   #BUILD_COMMAND make
)


cmake_minimum_required (VERSION 2.8)
PROJECT(ibiki)
set (IBIKI_VERSION_MAJOR 0)
set (IBIKI_VERSION_MINOR 0)

set (MAKE_TESTS 1 CACHE BOOL "Compile Test Unit?")
include_directories ( "${CMAKE_CURRENT_SOURCE_DIR}/include/" )

#Setup JSON Spirit
#The following must be set "#define BOOST_SPIRIT_THREADSAFE" in json_spirit/json_spirit_reader_template.h
add_subdirectory("tools/json_spirit/json_spirit")
link_directories ("${CMAKE_CURRENT_SOURCE_DIR}/tools/json_spirit/json_spirit/")
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/tools/json_spirit/json_spirit/")

#setup BOOST
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS   OFF)
find_package(Boost COMPONENTS system-mt thread-mt date_time-mt REQUIRED)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

# find_package( Gettext REQUIRED )
# http://www.gnu.org/s/hello/manual/gettext/lib_002fgettext_002eh.html

#setup environment config file
configure_file ( "src/env_config.h.in" "src/env_config.h" )

if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_BUILD_TYPE STREQUAL "debug")
  set(CMAKE_CXX_FLAGS "-g -Wall -Wl,-z,relro -D_FORTIFY_SOURCE=2 -fstack-protector")
ENDIF()

#list of the actual application code
set (CCODE src/configuration.cc src/json.cc src/hid.cc src/syslog.cc )
set (CCODE ${CCODE} src/cache.cc src/cache_buffer.cc src/cache_object.cc )
set (CCODE ${CCODE} src/serializable.cc src/hex.cc src/serialize.cc )

# ADD_EXECUTABLE(mon src/main.cc ${CCODE})
# target_link_libraries(mon
#   ${Boost_LIBRARIES}
#   json_spirit
# )

if (MAKE_TESTS)
  add_subdirectory("tools/gtest")
  #set CTEST_OUTPUT_ON_FAILURE to 1 in ENV if you want output
  link_directories ("${CMAKE_CURRENT_SOURCE_DIR}/tools/gtest/")
  include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/tools/gtest/include")

  set (DCODE src/gtest_dispatcher.cc )
  #set (DCODE src/gtest_dispatcher.cc src/gtest_hid.cc src/gtest_netobj.cc src/gtest_serialize.cc )
#   set (DCODE ${DCODE} src/gtest_network.cc src/gtest_lserver.cc)
#  set (DCODE ${DCODE} src/gtest_boost.cc src/gtest_json.cc )

  ADD_EXECUTABLE(montest src/gtest.cc ${CCODE} ${DCODE} )
  ADD_DEPENDENCIES(montest "tools/gtest/libgtest.a")
  target_link_libraries(montest
    ${Boost_LIBRARIES}
    gtest
    json_spirit
#     efence
  )

  ENABLE_TESTING()
  ADD_TEST(testunit montest)

  FILE (GLOB_RECURSE cclist "src/*.cc")
  FILE (GLOB_RECURSE hlist "src/*.h")
  #reformat source is used to fix whitespace issues, and is disabled in cpplint
  #call tools/get_cpplint.sh to retrieve get_cpplint.sh
  ADD_TEST(cpplint_cc tools/cpplint.py "--filter=-whitespace,-build/include,-readability/streams" ${cclist})
  ADD_TEST(cpplint_h tools/cpplint.py "--filter=-whitespace,-build/include,-readability/streams" ${hlist})
  #checks for boost format parameters which should be formated %[1-9]% but dont have final %
  ADD_TEST(boost_format_parameter_check tools/find_invalid_boost_format_string "${hlist} ${cclist}")

endif (MAKE_TESTS)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  #add_custom_target(doc ALL #only compile explicilty
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)


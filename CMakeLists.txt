cmake_minimum_required(VERSION 2.8.11)
# we depend on 2.8.11 introducing target_include_directories

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR AND NOT MSVC_IDE)
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'. Please delete them.")
endif()

project(OSRM C CXX)
set(OSRM_VERSION_MAJOR 5)
set(OSRM_VERSION_MINOR 3)
set(OSRM_VERSION_PATCH 0)

# these two functions build up custom variables:
#   OSRM_INCLUDE_PATHS and OSRM_DEFINES
# These variables we want to pass to
# include_directories and add_definitions for both
# this build and for sharing externally via pkg-config

function(add_dependency_includes includes)
  list(APPEND OSRM_INCLUDE_PATHS "${includes}")
  set(OSRM_INCLUDE_PATHS "${OSRM_INCLUDE_PATHS}" PARENT_SCOPE)
endfunction(add_dependency_includes)

function(add_dependency_defines defines)
  list(APPEND OSRM_DEFINES "${defines}")
  set(OSRM_DEFINES "${OSRM_DEFINES}" PARENT_SCOPE)
endfunction(add_dependency_defines)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CheckCXXCompilerFlag)
include(FindPackageHandleStandardArgs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(bitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(bitness 64)
  message(STATUS "Building on a 64 bit system")
else()
  message(STATUS "Building on a 32 bit system")
endif()

if(WIN32 AND MSVC_VERSION LESS 1900)
  message(FATAL_ERROR "Building with Microsoft compiler needs Latest Visual Studio 2015 (Community or better)")
endif()

option(ENABLE_CCACHE "Speed up incremental rebuilds via ccache" OFF)
option(ENABLE_JSON_LOGGING "Adds additional JSON debug logging to the response" OFF)
option(BUILD_TOOLS "Build OSRM tools" OFF)
option(BUILD_COMPONENTS "Build osrm-components" OFF)
option(ENABLE_ASSERTIONS OFF)
option(COVERAGE OFF)
option(SANITIZER OFF)
option(ENABLE_LTO "Use LTO if available" ON)
option(USE_TBB "Requires TBB - set to true for extract, prepare, etc." OFF)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/include/)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third_party/)

add_custom_target(FingerPrintConfigure ALL ${CMAKE_COMMAND}
  "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
  "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FingerPrint-Config.cmake"
  COMMENT "Configuring revision fingerprint"
  VERBATIM)

set(BOOST_COMPONENTS date_time filesystem iostreams program_options regex system thread)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/util/version.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/util/version.hpp
)
file(GLOB UtilGlob src/util/*.cpp src/util/*/*.cpp)
file(GLOB ExtractorGlob src/extractor/*.cpp src/extractor/*/*.cpp)
file(GLOB ContractorGlob src/contractor/*.cpp)
file(GLOB StorageGlob src/storage/*.cpp)
file(GLOB ServerGlob src/server/*.cpp src/server/**/*.cpp)
file(GLOB EngineGlob src/engine/*.cpp src/engine/**/*.cpp)

add_executable(osrm-extract src/tools/extract.cpp)
add_executable(osrm-contract src/tools/contract.cpp)
add_executable(osrm-routed src/tools/routed.cpp ${ServerGlob} ${UtilGlob})
add_executable(osrm-datastore src/tools/store.cpp ${UtilGlob})
add_library(osrm STATIC src/osrm/osrm.cpp ${EngineGlob} ${UtilGlob} ${StorageGlob})
add_library(osrm_extract STATIC ${ExtractorGlob} ${UtilGlob})
add_library(osrm_contract STATIC ${ContractorGlob} ${UtilGlob})
add_library(osrm_store STATIC ${StorageGlob} ${UtilGlob})

add_dependencies(osrm FingerPrintConfigure)
set_target_properties(osrm PROPERTIES LINKER_LANGUAGE CXX)

# Activate C++11
if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Configuring other platform dependencies
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64")
  message(STATUS "Set Architecture to x64 on OS X")
  exec_program(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
  string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
  if(OSXLIBSTD)
    message(STATUS "linking against ${OSXLIBSTD}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=${OSXLIBSTD}")
  elseif(DARWIN_VERSION GREATER 12)
    message(STATUS "linking against libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

if(UNIX AND NOT APPLE)
  set(MAYBE_RT_LIBRARY rt)
endif()

include_directories(osrm-extract "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libosmium/include")


find_package(Boost 1.49.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

# collect a subset of the boost libraries needed
# by libosrm
foreach(lib ${Boost_LIBRARIES})
  if(NOT WIN32)
    if(lib MATCHES filesystem OR lib MATCHES thread OR lib MATCHES iostreams OR lib MATCHES system)
       list(APPEND BOOST_ENGINE_LIBRARIES "${lib}")
    endif()
  else()
    list(APPEND BOOST_ENGINE_LIBRARIES "${lib}")
  endif()
endforeach(lib)

if(NOT WIN32 AND NOT Boost_USE_STATIC_LIBS)
  add_dependency_defines(-DBOOST_TEST_DYN_LINK)
endif()
add_dependency_defines(-DBOOST_SPIRIT_USE_PHOENIX_V3)
add_dependency_defines(-DBOOST_RESULT_OF_USE_DECLTYPE)
add_dependency_defines(-DBOOST_FILESYSTEM_NO_DEPRECATED)
add_dependency_includes(${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)

if (USE_TBB)
  find_package(TBB REQUIRED)
  add_dependency_includes(${TBB_INCLUDE_DIR})
  if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)
    set(TBB_LIBRARIES ${TBB_DEBUG_LIBRARIES})
  endif()
endif()

add_subdirectory(external_lib/luabind)
include_directories(SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/luabind/external_lib/lua-5.1/src
  ${CMAKE_CURRENT_SOURCE_DIR}/external_lib/luabind)
target_link_libraries(osrm_extract luabind)
target_link_libraries(osrm_contract luabind)

add_subdirectory(external_lib/stxxl)
include_directories(SYSTEM
  external_lib/stxxl/include
  ${PROJECT_BINARY_DIR}/external_lib/stxxl/include)
target_link_libraries(osrm_extract stxxl)
target_link_libraries(osrm_contract stxxl)

set(OpenMP_FIND_QUIETLY ON)
find_package(OpenMP)
if(OPENMP_FOUND)
  message(STATUS "OpenMP support found. Linking just in case for stxxl")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package(ZLIB REQUIRED)
add_dependency_includes(${ZLIB_INCLUDE_DIRS})

if (ENABLE_JSON_LOGGING)
  message(STATUS "Enabling json logging")
  add_dependency_defines(-DENABLE_JSON_LOGGING)
endif()

add_definitions(${OSRM_DEFINES})
include_directories(SYSTEM ${OSRM_INCLUDE_PATHS})

# Binaries
target_link_libraries(osrm-datastore osrm_store ${Boost_LIBRARIES})
target_link_libraries(osrm-extract osrm_extract ${Boost_LIBRARIES})
target_link_libraries(osrm-contract ${Boost_LIBRARIES} ${TBB_LIBRARIES} osrm_contract)
target_link_libraries(osrm-routed osrm ${Boost_LIBRARIES} ${OPTIONAL_SOCKET_LIBS} ${ZLIB_LIBRARY})

set(EXTRACTOR_LIBRARIES
    ${BZIP2_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${EXPAT_LIBRARIES}
    ${LUABIND_LIBRARY}
    ${USED_LUA_LIBRARIES}
    ${OSMIUM_LIBRARIES}
    ${STXXL_LIBRARY}
    ${TBB_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${MAYBE_COVERAGE_LIBRARIES})
set(CONTRACTOR_LIBRARIES
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LUABIND_LIBRARY}
    ${USED_LUA_LIBRARIES}
    ${STXXL_LIBRARY}
    ${TBB_LIBRARIES}
    ${MAYBE_RT_LIBRARY}
    ${MAYBE_COVERAGE_LIBRARIES})
set(ENGINE_LIBRARIES
    ${BOOST_ENGINE_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${TBB_LIBRARIES}
    ${MAYBE_RT_LIBRARY}
    ${MAYBE_COVERAGE_LIBRARIES}
    ${ZLIB_LIBRARY})
set(STORAGE_LIBRARIES
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${TBB_LIBRARIES}
    ${MAYBE_RT_LIBRARY}
    ${MAYBE_COVERAGE_LIBRARIES})
set(UTIL_LIBRARIES
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${STXXL_LIBRARY}
    ${TBB_LIBRARIES}
    ${MAYBE_COVERAGE_LIBRARIES})
# Libraries
target_link_libraries(osrm ${ENGINE_LIBRARIES})
target_link_libraries(osrm_contract ${CONTRACTOR_LIBRARIES})
target_link_libraries(osrm_extract ${EXTRACTOR_LIBRARIES})
target_link_libraries(osrm_store ${STORAGE_LIBRARIES})

if(BUILD_COMPONENTS)
  find_package(GDAL)
  if(GDAL_FOUND)
    add_executable(osrm-components src/tools/components.cpp ${UtilGlob})
    target_link_libraries(osrm-components ${TBB_LIBRARIES})
    include_directories(SYSTEM ${GDAL_INCLUDE_DIR})
    target_link_libraries(osrm-components ${GDAL_LIBRARIES} ${Boost_LIBRARIES})
    install(TARGETS osrm-components DESTINATION bin)
  else()
    message(WARNING "libgdal and/or development headers not found")
  endif()
endif()

if(BUILD_TOOLS)
  message(STATUS "Activating OSRM internal tools")
  add_executable(osrm-io-benchmark src/tools/io-benchmark.cpp ${UtilGlob})
  target_link_libraries(osrm-io-benchmark ${Boost_LIBRARIES})
  add_executable(osrm-unlock-all src/tools/unlock_all_mutexes.cpp ${UtilGlob})
  target_link_libraries(osrm-unlock-all ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  if(UNIX AND NOT APPLE)
    target_link_libraries(osrm-unlock-all rt)
  endif()
  add_executable(osrm-springclean src/tools/springclean.cpp ${UtilGlob})
  target_link_libraries(osrm-springclean ${Boost_LIBRARIES})

  install(TARGETS osrm-io-benchmark DESTINATION bin)
  install(TARGETS osrm-unlock-all DESTINATION bin)
  install(TARGETS osrm-springclean DESTINATION bin)
endif()

if (ENABLE_ASSERTIONS)
  message(STATUS "Enabling assertions")
  add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)
endif()

# Add RPATH info to executables so that when they are run after being installed
# (i.e., from /usr/local/bin/) the linker can find library dependencies. For
# more info see http://www.cmake.org/Wiki/CMake_RPATH_handling
set_property(TARGET osrm-extract PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
set_property(TARGET osrm-contract PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
set_property(TARGET osrm-datastore PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
set_property(TARGET osrm-routed PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

file(GLOB VariantGlob third_party/variant/*.hpp)
file(GLOB LibraryGlob include/osrm/*.hpp)
file(GLOB ParametersGlob include/engine/api/*_parameters.hpp)
set(EngineHeader include/engine/status.hpp include/engine/engine_config.hpp include/engine/hint.hpp include/engine/bearing.hpp include/engine/phantom_node.hpp)
set(UtilHeader include/util/coordinate.hpp include/util/json_container.hpp include/util/typedefs.hpp include/util/strong_typedef.hpp)
set(ExtractorHeader include/extractor/extractor.hpp include/extractor/extractor_config.hpp include/extractor/travel_mode.hpp)
set(ContractorHeader include/contractor/contractor.hpp include/contractor/contractor_config.hpp)
set(StorageHeader include/storage/storage.hpp include/storage/storage_config.hpp)
install(FILES ${EngineHeader} DESTINATION include/osrm/engine)
install(FILES ${UtilHeader} DESTINATION include/osrm/util)
install(FILES ${StorageHeader} DESTINATION include/osrm/storage)
install(FILES ${ExtractorHeader} DESTINATION include/osrm/extractor)
install(FILES ${ContractorHeader} DESTINATION include/osrm/contractor)
install(FILES ${LibraryGlob} DESTINATION include/osrm)
install(FILES ${ParametersGlob} DESTINATION include/osrm/engine/api)
install(FILES ${VariantGlob} DESTINATION include/variant)
install(TARGETS osrm-extract DESTINATION bin)
install(TARGETS osrm-contract DESTINATION bin)
install(TARGETS osrm-datastore DESTINATION bin)
install(TARGETS osrm-routed DESTINATION bin)
install(TARGETS osrm DESTINATION lib)
install(TARGETS osrm_extract DESTINATION lib)
install(TARGETS osrm_contract DESTINATION lib)
install(TARGETS osrm_store DESTINATION lib)

list(GET ENGINE_LIBRARIES 1 ENGINE_LIBRARY_FIRST)
foreach(lib ${ENGINE_LIBRARIES})
  get_filename_component(ENGINE_LIBRARY_PATH "${ENGINE_LIBRARY_FIRST}" PATH)
  get_filename_component(ENGINE_LIBRARY_NAME "${lib}" NAME_WE)
  string(REPLACE "lib" "" ENGINE_LIBRARY_NAME ${ENGINE_LIBRARY_NAME})
  string(REPLACE "-l" "" ENGINE_LIBRARY_NAME ${ENGINE_LIBRARY_NAME})
  set(ENGINE_LIBRARY_LISTING "${ENGINE_LIBRARY_LISTING} -L${ENGINE_LIBRARY_PATH} -l${ENGINE_LIBRARY_NAME}")
endforeach()

if(BUILD_DEBIAN_PACKAGE)
  include(CPackDebianConfig)
  include(CPack)
endif()

function(JOIN VALUES GLUE OUTPUT)
  string (REPLACE ";" "${GLUE}" _TMP_STR "${VALUES}")
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

# Set up variables, then write to pkgconfig file
JOIN("${OSRM_DEFINES}" " " OSRM_DEFINES_STRING)
JOIN("-I${OSRM_INCLUDE_PATHS}" " -I" OSRM_INCLUDE_PATHS_STRING)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkgconfig.in libosrm.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/libosrm.pc DESTINATION lib/pkgconfig)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif()

# prefix compilation with ccache by default if available and on clang or gcc
if(ENABLE_CCACHE AND (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"))
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message(STATUS "Using ccache to speed up incremental builds")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    set(ENV{CCACHE_CPP2} "true")
  endif()
endif()

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)


# Modular build system: each directory registered here provides its own CMakeLists.txt
# add_subdirectory(unit_tests)
# add_subdirectory(src/benchmarks)


add_executable(osrm-eval EXCLUDE_FROM_ALL eval/osrm_eval.cpp)
target_link_libraries(osrm-eval osrm)
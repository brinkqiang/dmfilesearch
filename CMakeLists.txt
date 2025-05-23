cmake_minimum_required(VERSION 3.21)

PROJECT(dmfilesearch)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()
ModuleSetWinCompilerFlags()

SET(DMFILESEARCH_VERSION_MAJOR "1")
SET(DMFILESEARCH_VERSION_MINOR "0")
SET(DMFILESEARCH_VERSION_PATCH "1")
SET(DMFILESEARCH_VERSION "${DMFILESEARCH_VERSION_MAJOR}.${DMFILESEARCH_VERSION_MINOR}.${DMFILESEARCH_VERSION_PATCH}")

MESSAGE(STATUS "VERSION: ${DMFILESEARCH_VERSION}")

OPTION(USE_DMFILESEARCH "use dmfilesearch" OFF)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/dmfilesearch_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/dmfilesearch_config.h)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(MSVC)
    add_compile_definitions("_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR=1")
endif()

ModuleImportAll("thirdparty")

LibImport("libdmfilesearch" "src/libdmfilesearch")

if(PROJECT_IS_TOP_LEVEL)
    ExeImport("tools" "libdmfilesearch")
endif()

#AddInstall("libdmfilesearch" "${CMAKE_CURRENT_SOURCE_DIR}/include")

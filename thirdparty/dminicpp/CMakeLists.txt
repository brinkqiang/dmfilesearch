cmake_minimum_required(VERSION 3.8)

PROJECT(dminicpp)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()
ModuleSetWinCompilerFlags()

ModuleImportAll("thirdparty")

InterfaceImport("dminicpp" "include" "")

if (PROJECT_IS_TOP_LEVEL)
    ExeImport("examples" "dminicpp")
    ExeImport("test" "dminicpp;dmtest")
endif(PROJECT_IS_TOP_LEVEL)


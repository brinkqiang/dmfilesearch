cmake_minimum_required(VERSION 3.21)

PROJECT(dmtest)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()
ModuleSetWinCompilerFlags()

LibImport("dmtest" "src/dmtest")

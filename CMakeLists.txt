cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ULuaPP VERSION 0.1.0)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_AddDep(ULua 0.1.0)
Ubpa_AddDep(USRefl 0.4.3)
Ubpa_AddDep(UTemplate 0.4.6)

Ubpa_AddSubDirsRec(src)

Ubpa_Export(DIRECTORIES "include")

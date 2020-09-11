set(c_options "")
if(MSVC)
  list(APPEND c_options "/wd5030")
endif()

Ubpa_AddTarget(
  MODE INTERFACE
  INC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/_deps"
  C_OPTION
    ${c_options}
  LIB
    Ubpa::USRefl_core
	Ubpa::ULua_core
)

macro(Vantablade_configure_linker project_name)
  set(Vantablade_USER_LINKER_OPTION
    "DEFAULT"
      CACHE STRING "Linker to be used")
    set(Vantablade_USER_LINKER_OPTION_VALUES "DEFAULT" "SYSTEM" "LLD" "GOLD" "BFD" "MOLD" "SOLD" "APPLE_CLASSIC" "MSVC")
  set_property(CACHE Vantablade_USER_LINKER_OPTION PROPERTY STRINGS ${Vantablade_USER_LINKER_OPTION_VALUES})
  list(
    FIND
    Vantablade_USER_LINKER_OPTION_VALUES
    ${Vantablade_USER_LINKER_OPTION}
    Vantablade_USER_LINKER_OPTION_INDEX)

  if(${Vantablade_USER_LINKER_OPTION_INDEX} EQUAL -1)
    message(
      STATUS
        "Using custom linker: '${Vantablade_USER_LINKER_OPTION}', explicitly supported entries are ${Vantablade_USER_LINKER_OPTION_VALUES}")
  endif()

  set_target_properties(${project_name} PROPERTIES LINKER_TYPE "${Vantablade_USER_LINKER_OPTION}")
endmacro()

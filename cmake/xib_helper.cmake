# xib_helpers.cmake by cubicvoid
#
# Include this helper at the beginning of your project to
# simplify handling of Apple .xib files.
# Call add_xib([filename]) from anywhere in the project (including
# nested directories), then call finalize_xibs and apply the results
# to your target:
#   add_xib(MyCustomView.xib)
#   ...
#   finalize_xibs(XIBS)
#   target_sources(mytarget PRIVATE "${XIBS}")
#   set_target_properties(mytarget PROPERTIES RESOURCE "${XIBS}")


find_program(IBTOOL ibtool REQUIRED)

# this isn't pretty but it's the only way to avoid including all
# xibs for all modules explicitly in the main target cmake file
function(add_xib)
  # make absolute paths
  set(xibs)
  foreach(file IN LISTS ARGN)
    get_filename_component(fullpath "${file}" ABSOLUTE)
    list(APPEND xibs "${fullpath}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY XIB_LIST "${xibs}")
endfunction()

# call this in the same file that applies the RESOURCE tag, and after
# any subdirectories/subtargets that might have xib files.
function(finalize_xibs output)
  get_property(xibs GLOBAL PROPERTY XIB_LIST)
  # Xcode handles xib -> nib compilation, for other generators we must
  # do it ourself
  if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set("${output}" "${xibs}" PARENT_SCOPE)
  else()
    set(nibs)
    foreach(fullpath ${xibs})
      get_filename_component(name "${fullpath}" NAME_WE)
      add_custom_command(OUTPUT "${name}.nib"
        COMMAND ${IBTOOL} --compile "${name}.nib" "${fullpath}"
        DEPENDS "${fullpath}"
        VERBATIM
      )
      list(APPEND nibs "${name}.nib")
    endforeach()
    set("${output}" "${nibs}" PARENT_SCOPE)
  endif()
endfunction()

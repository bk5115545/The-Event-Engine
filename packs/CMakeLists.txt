
# This macro makes a list of the subdirectories relative to curdir
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE "${curdir}" "${curdir}/*")
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(PACK_DIRS "${APP_PACK_DIR}")


# Add each pack to the build
# Each pack must use cmake and have a CMakeLists.txt
FOREACH(dir ${PACK_DIRS})
    add_subdirectory("${dir}")
    get_directory_property(PACK_SOURCE DIRECTORY "${dir}" DEFINITION PACK_SOURCE)
    get_directory_property(PACK_INCLUDES DIRECTORY "${dir}" DEFINITION PACK_INCLUDES)
ENDFOREACH()

LIST(APPEND PACK_SOURCE "${PACK_CHILD_SOURCE}")
LIST(APPEND PACK_INCLUDES "${PACK_CHILD_INCLUDES}")

file(GLOB_RECURSE core_source *.cpp)
list(APPEND SRC_FILES ${core_source})

set(SRC_FILES "${SRC_FILES}" PARENT_SCOPE)

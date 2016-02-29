cmake_minimum_required(VERSION 2.6)
project(game_engine)
set(TARGET_NAME game_engine)
set(APP_PACK_DIR "${CMAKE_SOURCE_DIR}/packs")
set(APP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/source")

set(CMAKE_USE_RELATIVE_PATHS TRUE)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Set an output directory for our binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Bump up warning levels appropriately for clang, gcc & msvc
# Also set debug/optimization flags depending on the build type. IDE users choose this when
# selecting the build mode in their IDE
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # Compiler options for GCC and Clang on Linus or OSX

    # Use C++ 14
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread -pg")

    # Set warning levels
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wdouble-promotion -Wno-unknown-pragmas")

    # Set debug and release flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O0")

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    # Compiler options for MSVC on Windows
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    endif()
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

message(STATUS "SDL2_LIBRARY...... ${SDL2_LIBRARY}")
message(STATUS "SDL2_INCLUDE_DIR.. ${SDL2_INCLUDE_DIR}")
message(STATUS "OPENGL_LIBRARY.... ${OPENGL_gl_LIBRARY}")
message(STATUS "GLEW_LIBRARIES.... ${OPENGL_glu_LIBRARY}")

# Include directories with relevant items
add_subdirectory("${APP_SOURCE_DIR}")
add_subdirectory("${APP_PACK_DIR}")
get_directory_property(PACK_SOURCE DIRECTORY "${APP_PACK_DIR}" DEFINITION PACK_SOURCE)
get_directory_property(PACK_INCLUDES DIRECTORY "${APP_PACK_DIR}" DEFINITION PACK_INCLUDES)

# The below must be done AFTER the above line so it can't really go into source/CMakeLists.txt
message(STATUS "SOURCE ........... ${SRC_FILES}")
message(STATUS "PACK_SOURCE .......${PACK_SOURCE}")
message(STATUS "PACK_INCLUDES .....${PACK_INCLUDES}")
add_executable(${TARGET_NAME} ${SRC_FILES} ${PACK_SOURCE})

target_include_directories(${TARGET_NAME} PRIVATE
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    "${OPENGL_INCLUDE_DIR}"
    "${SDL2_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    ${PACK_INCLUDES}
)

target_link_libraries(${TARGET_NAME}
                      "${SDL2_LIBRARY}"
                      "${OPENGL_gl_LIBRARY}"
                      ${GLEW_LIBRARIES}
                      ${GLEW_MX_LIBRARIES}
                      ${APP_PACK_LIBS}
)

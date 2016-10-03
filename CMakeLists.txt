cmake_minimum_required(VERSION 3.1)

project(AsepriteRgbaConvert CXX)

# Default compiler args
if (CMAKE_CXX_COMPILER_ID MATCHES "(GNU|.*Clang)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Werror -Wall -Wextra -Wno-unused-parameter -Wno-error=unused-variable -Wno-error=sign-compare -std=c++14 -Wno-error=deprecated-declarations -Wno-unused-local-typedefs")
    if (CMAKE_CXX_COMPILER_ID MATCHES "(.*Clang)")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers -Qunused-arguments")
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "(GNU)")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-local-typedefs")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-g -Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-g -O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL 'MSVC')
    # /Zi - Produces a program database (PDB) that contains type information and symbolic debugging information for use with the debugger.
    # /FS - Allows multiple cl.exe processes to write to the same .pdb file
    # /DEBUG - Enable debug during linking
    # /Od - Disables optimization
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /FS /DEBUG /Od /MDd")
    # /Ox - Full optimization
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Ox /Zi /FS /DEBUG")
endif()

option(BUILD_SHARED_LIBS "Compile non-static libs?" Off)
option(ZLIB_TESTS    "Compile with zlib examples/tests" Off)
option(AsepriteRgbaConvert_TESTS    "Compile AsepriteRgbaConvert tests" Off)

set(SOURCES
    AsepriteRgbaConvert/loader.cpp
    AsepriteRgbaConvert/blend.cpp
    AsepriteRgbaConvert/loader-detail.cpp
    )

set(INCLUDES
    AsepriteRgbaConvert/blend.h
    AsepriteRgbaConvert/loader.h
    AsepriteRgbaConvert/loader-detail.hpp
    AsepriteRgbaConvert/doc-lite.h
    )

set(SKIP_INSTALL_ALL 1)
set(ZLIB_ROOT ${PROJECT_SOURCE_DIR}/zlib)
set(ZLIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/zlib" "${CMAKE_CURRENT_BINARY_DIR}/zlib")
set(ZLIB_LIBRARIES "zlibstatic")
add_subdirectory(${PROJECT_SOURCE_DIR}/zlib)

add_library(AsepriteRgbaConvert STATIC ${SOURCES} ${INCLUDES})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/AsepriteRgbaConvert)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(AsepriteRgbaConvert ${ZLIB_LIBRARIES})

if(AsepriteRgbaConvert_TESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/integration_test)
endif()

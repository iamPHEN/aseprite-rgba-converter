cmake_minimum_required(VERSION 3.1)

project(asepriteConvertTest)

include_directories(${AsepriteRgbaConvert_SOURCE_DIR})
include_directories(${ZLIB_INCLUDE_DIRS})
add_executable(asepriteConvertTest main.cpp)
add_dependencies(asepriteConvertTest AsepriteRgbaConvert)
target_link_libraries(asepriteConvertTest ${ZLIB_LIBRARIES})

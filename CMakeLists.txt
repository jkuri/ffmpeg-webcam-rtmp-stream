CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(ffmpeg-webcam-stream VERSION 0.1.0 LANGUAGES CXX C)

IF(UNIX AND NOT APPLE)
  SET(LINUX TRUE)
ENDIF()

SET(LINK_TYPE STATIC)

FILE(GLOB ARCHIVE_LIBS ${CMAKE_SOURCE_DIR}/libs/build/lib/*.a)

if(APPLE)
  SET(LINK_FLAGS "-lbz2 \
  -llzma \
  -framework AppKit \
  -framework CoreFoundation \
  -framework AVFoundation \
  -framework CoreMedia \
  -framework CoreVideo \
  -framework CoreImage \
  -framework CoreAudio \
  -framework VideoToolbox \
  -framework AudioToolbox \
  -framework OpenGL\
  -framework Metal")
ELSE()
  SET(LINK_FLAGS "-pthread -ldl -lm -static")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/libs/build/include)
ADD_EXECUTABLE(stream src/stream.c)
TARGET_LINK_LIBRARIES(stream ${ARCHIVE_LIBS} ${LINK_FLAGS})

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(ffmpeg-webcam-stream VERSION 0.1.0 LANGUAGES CXX C)

IF(UNIX AND NOT APPLE)
  SET(LINUX TRUE)
ENDIF()

SET(LINK_TYPE STATIC)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/libs)
FILE(GLOB ARCHIVE_LIBS ${CMAKE_BINARY_DIR}/lib/*.a)
SET(LIBS ${ARCHIVE_LIBS})

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
  -framework OpenGL")
ELSE()
  SET(LINK_FLAGS "")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
ADD_EXECUTABLE(stream src/stream.c)
TARGET_LINK_LIBRARIES(stream ${LIBS} ${LINK_FLAGS})

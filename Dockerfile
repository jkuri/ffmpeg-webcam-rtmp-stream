FROM ubuntu:bionic AS base

RUN apt-get update && apt-get install libavcodec57 libavformat57 libavutil55 libswscale4 libavresample3 libavdevice57 -y && apt-get autoclean -y

FROM base AS build

WORKDIR /tmp/build
RUN apt-get update && apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libavdevice-dev ffmpeg clang build-essential -y
COPY src/ src/
COPY include/ include/
COPY Makefile Makefile
RUN make

FROM base AS image

WORKDIR /
COPY --from=build /tmp/build/build/stream /usr/bin/stream

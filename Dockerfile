FROM ubuntu:plucky AS base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install libavcodec61 libavformat61 libavutil59 libswscale8 libavdevice61 -y && apt-get autoclean -y

FROM base AS build

WORKDIR /tmp/build
RUN apt-get update && apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavdevice-dev ffmpeg clang build-essential -y
COPY src/ src/
COPY include/ include/
COPY Makefile Makefile
RUN make

FROM base AS image

WORKDIR /
COPY --from=build /tmp/build/build/stream /usr/bin/stream

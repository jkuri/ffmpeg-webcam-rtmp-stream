FROM ubuntu:jammy AS base

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install libavcodec58 libavformat58 libavutil56 libswscale5 libavdevice58 -y && apt-get autoclean -y

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

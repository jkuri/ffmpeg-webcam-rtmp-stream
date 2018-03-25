FROM ubuntu:bionic as build

WORKDIR /tmp/build
RUN apt-get update && apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libavdevice-dev ffmpeg clang build-essential -y
COPY src/ src/
COPY include/ include/
COPY Makefile Makefile
RUN make

FROM ubuntu:bionic as release
RUN apt-get update && apt-get install ffmpeg -y && apt-get autoremove -y && apt-get clean -y

WORKDIR /app
COPY --from=build /tmp/build/build/stream /app/stream


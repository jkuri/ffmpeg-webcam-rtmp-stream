all: build

build: checkdir stream

stream:
	clang -Iinclude -o build/stream src/stream.c -lavdevice -lavutil -lavcodec -lavformat -lswscale

static:
	mkdir -p build && cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make -j12

checkdir:
	@mkdir -p build

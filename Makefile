all: build

build: checkdir stream

stream:
	$(CC) -Iinclude -o build/stream src/stream.c -lavdevice -lavutil -lavcodec -lavformat -lswscale

static:
	mkdir -p libs/build && cd libs/build && cmake -D CMAKE_BUILD_TYPE=Release .. && make
	mkdir -p build && cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make

clean:
	@rm -rf build/ libs/build

checkdir:
	@mkdir -p build

all: build

build: checkdir stream

stream:
	clang -Iinclude -o build/stream src/stream.c -lavdevice -lavutil -lavcodec -lavformat -lswscale

checkdir:
	@mkdir -p build

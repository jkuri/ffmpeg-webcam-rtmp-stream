## ffmpeg-webcam-stream

Webcam capture streaming via RTMP or saving into video file. Cross-platform.

### Prerequesites

#### Ubuntu

```sh
sudo apt-get update
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libavdevice-dev -y
sudo apt-get install ffmpeg -y
sudo apt-get install build-essential clang -y
```

#### MacOS

```sh
brew install ffmpeg clang
```

### Build

```sh
make
```

Build artifacts will be stored inside `build/` directory.

### Example Live Stream

On MacOS:

```sh
./build/stream 0 rtmp://localhost/live/stream flv 1920 1080 30
```

On Linux:

```sh
./build/stream /dev/video0 rtmp://localhost/live/stream flv 800 600 24
```

### License

MIT

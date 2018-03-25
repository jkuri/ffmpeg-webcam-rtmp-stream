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

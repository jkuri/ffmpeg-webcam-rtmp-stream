## ffmpeg-webcam-rtmp-stream

Webcam capture streaming via RTMP or saving into video file. Cross-platform.

### Build (static)

```sh
make static
```

### Build (dynamic)

#### Prerequisites (Ubuntu)

```sh
sudo apt-get update
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev libavdevice-dev -y
sudo apt-get install ffmpeg -y
sudo apt-get install build-essential clang -y
```

#### Prerequisites (MacOS)

```sh
brew install ffmpeg
```

After you installed everything for your host OS, run:

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

You can also save webcam stream into file:

```sh
./build/stream 0 out.mp4 mp4 800 600 30
```

### Docker

There are 3 Docker files prepared, choose your preferable one and build an image:

- Ubuntu (dynamic)

```sh
docker build -t ffmpeg-webcam-rtmp-stream .
```

- Ubuntu (static)

```sh
docker build -t ffmpeg-webcam-rtmp-stream -f Dockerfile.ubuntu.static .
```

- Alpine (static)

```sh
docker build -t ffmpeg-webcam-rtmp-stream -f Dockerfile.alpine.static .
```

Then run stream from container (only works on Linux as host):

```sh
docker run --device /dev/video0 -it ffmpeg-webcam-rtmp-stream stream /dev/video0 rtmp://localhost/live/stream flv 1280 720 30
```

### RTMP Server

For testing purposes you can run RTMP server as:

```sh
docker run -it -p 1935:1935 -p 8080:8080 --name rtmp-server jkuri/urtmp
```

Then open your browser at `http://localhost:8080` where you can watch your published streams.

### License

MIT

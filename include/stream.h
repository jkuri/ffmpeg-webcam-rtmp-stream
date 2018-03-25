
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

typedef struct stream_ctx_t
{
  char *output_path, *output_format;
  AVInputFormat *ifmt;
  AVFormatContext *ifmt_ctx, *ofmt_ctx;
  AVCodec *in_codec, *out_codec;
  AVStream *in_stream, *out_stream;
  AVCodecContext *in_codec_ctx, *out_codec_ctx;
} stream_ctx_t;

void stream_video(const char *device_index, const char *output_path, const char *output_format, int width, int height, int fps);
int init_device_and_input_context(stream_ctx_t *stream_ctx, const char *device_family, const char *device_index, int width, int height, int fps);
int init_output_avformat_context(stream_ctx_t *stream_ctx, const char *format_name);
int init_io_context(stream_ctx_t *stream_ctx, const char *output_path);
void set_codec_params(stream_ctx_t *stream_ctx, int width, int height, int fps);
int init_codec_stream(stream_ctx_t *stream_ctx);
struct SwsContext *initialize_sample_scaler(AVCodecContext *codec_ctx, int width, int height);
char *concat_str(const char *s1, const char *s2);
const char *get_device_family();

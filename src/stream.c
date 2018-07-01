#include "stream.h"

int main(int argc, char *argv[])
{
  if (argc != 7)
  {
    fprintf(stderr, "Usage: %s [device] [output_path] [output_format] [width] [height] [fps]\n", argv[0]);
    return 1;
  }

  const char *device = argv[1];
  const char *output_path = argv[2];
  const char *output_format = argv[3];
  int width = atoi(argv[4]);
  int height = atoi(argv[5]);
  int fps = atoi(argv[6]);

  end_stream = false;
  signal(SIGINT, handle_signal);

  stream_video(device, output_path, output_format, width, height, fps);

  return 0;
}

void stream_video(const char *device_index, const char *output_path, const char *output_format, int width, int height, int fps)
{
  av_register_all();
  avdevice_register_all();
  avformat_network_init();

  const char *device_family = get_device_family();

  stream_ctx_t *stream_ctx = malloc(sizeof(stream_ctx_t));
  stream_ctx->output_path = malloc(strlen(output_path) + 1);
  stream_ctx->output_format = malloc(strlen(output_format) + 1);
  stream_ctx->ifmt = NULL;
  stream_ctx->ifmt_ctx = NULL;
  stream_ctx->ofmt_ctx = NULL;
  stream_ctx->out_codec = NULL;
  stream_ctx->out_stream = NULL;
  stream_ctx->out_codec_ctx = NULL;

  memcpy(stream_ctx->output_path, output_path, strlen(output_path));
  stream_ctx->output_path[strlen(output_path)] = '\0';
  memcpy(stream_ctx->output_format, output_format, strlen(output_format));
  stream_ctx->output_format[strlen(output_format)] = '\0';

  if (init_device_and_input_context(stream_ctx, device_family, device_index, width, height, fps) != 0)
  {
    return;
  }

  init_output_avformat_context(stream_ctx, output_format);
  init_io_context(stream_ctx, output_path);

  stream_ctx->out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  stream_ctx->out_stream = avformat_new_stream(stream_ctx->ofmt_ctx, stream_ctx->out_codec);
  stream_ctx->out_codec_ctx = avcodec_alloc_context3(stream_ctx->out_codec);

  set_codec_params(stream_ctx, width, height, fps);
  init_codec_stream(stream_ctx);

  stream_ctx->out_stream->codecpar->extradata = stream_ctx->out_codec_ctx->extradata;
  stream_ctx->out_stream->codecpar->extradata_size = stream_ctx->out_codec_ctx->extradata_size;

  // av_dump_format(stream_ctx->ofmt_ctx, 0, output_path, 1);

  if (avformat_write_header(stream_ctx->ofmt_ctx, NULL) != 0)
  {
    fprintf(stderr, "could not write header to ouput context!\n");
    return;
  }

  AVFrame *frame = av_frame_alloc();
  AVFrame *outframe = av_frame_alloc();
  AVPacket *pkt = av_packet_alloc();

  int nbytes = av_image_get_buffer_size(stream_ctx->out_codec_ctx->pix_fmt, stream_ctx->out_codec_ctx->width, stream_ctx->out_codec_ctx->height, 32);
  uint8_t *video_outbuf = (uint8_t *)av_malloc(nbytes);
  av_image_fill_arrays(outframe->data, outframe->linesize, video_outbuf, AV_PIX_FMT_YUV420P, stream_ctx->out_codec_ctx->width, stream_ctx->out_codec_ctx->height, 1);
  outframe->width = width;
  outframe->height = height;
  outframe->format = stream_ctx->out_codec_ctx->pix_fmt;

  struct SwsContext *swsctx = sws_getContext(stream_ctx->in_codec_ctx->width, stream_ctx->in_codec_ctx->height, stream_ctx->in_codec_ctx->pix_fmt, stream_ctx->out_codec_ctx->width, stream_ctx->out_codec_ctx->height, stream_ctx->out_codec_ctx->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
  av_init_packet(pkt);

  long pts = 0;

  while (av_read_frame(stream_ctx->ifmt_ctx, pkt) >= 0 && !end_stream)
  {
    frame = av_frame_alloc();
    if (avcodec_send_packet(stream_ctx->in_codec_ctx, pkt) != 0)
    {
      fprintf(stderr, "error sending packet to input codec context!\n");
      break;
    }

    if (avcodec_receive_frame(stream_ctx->in_codec_ctx, frame) != 0)
    {
      fprintf(stderr, "error receiving frame from input codec context!\n");
      break;
    }

    av_packet_unref(pkt);
    av_init_packet(pkt);

    sws_scale(swsctx, (const uint8_t *const *)frame->data, frame->linesize, 0, stream_ctx->in_codec_ctx->height, outframe->data, outframe->linesize);
    av_frame_free(&frame);
    outframe->pts = pts++;

    if (avcodec_send_frame(stream_ctx->out_codec_ctx, outframe) != 0)
    {
      fprintf(stderr, "error sending frame to output codec context!\n");
      break;
    }

    if (avcodec_receive_packet(stream_ctx->out_codec_ctx, pkt) != 0)
    {
      fprintf(stderr, "error receiving packet from output codec context!\n");
      break;
    }

    pkt->pts = av_rescale_q(pkt->pts, stream_ctx->out_codec_ctx->time_base, stream_ctx->out_stream->time_base);
    pkt->dts = av_rescale_q(pkt->dts, stream_ctx->out_codec_ctx->time_base, stream_ctx->out_stream->time_base);

    av_interleaved_write_frame(stream_ctx->ofmt_ctx, pkt);
    av_packet_unref(pkt);
    av_init_packet(pkt);
  }

  av_write_trailer(stream_ctx->ofmt_ctx);
  av_frame_free(&outframe);
  avio_close(stream_ctx->ofmt_ctx->pb);
  avformat_free_context(stream_ctx->ofmt_ctx);
  avio_close(stream_ctx->ifmt_ctx->pb);
  avformat_free_context(stream_ctx->ifmt_ctx);

  fprintf(stderr, "done.\n");
}

int init_device_and_input_context(stream_ctx_t *stream_ctx, const char *device_family, const char *device_index, int width, int height, int fps)
{
  char fps_str[5], width_str[5], height_str[5];
  sprintf(fps_str, "%d", fps);
  sprintf(width_str, "%d", width);
  sprintf(height_str, "%d", height);

  char *tmp = concat_str(width_str, "x");
  char *size = concat_str(tmp, height_str);
  free(tmp);

  stream_ctx->ifmt = av_find_input_format(device_family);

  AVDictionary *options = NULL;
  av_dict_set(&options, "video_size", size, 0);
  av_dict_set(&options, "framerate", fps_str, 0);
  av_dict_set(&options, "pixel_format", "uyvy422", 0);
  av_dict_set(&options, "probesize", "7000000", 0);

  free(size);

  if (avformat_open_input(&stream_ctx->ifmt_ctx, device_index, stream_ctx->ifmt, &options) != 0)
  {
    fprintf(stderr, "cannot initialize input device!\n");
    return 1;
  }

  avformat_find_stream_info(stream_ctx->ifmt_ctx, 0);
  // av_dump_format(stream_ctx->ifmt_ctx, 0, device_family, 0);

  stream_ctx->in_codec = avcodec_find_decoder(stream_ctx->ifmt_ctx->streams[0]->codecpar->codec_id);
  stream_ctx->in_stream = avformat_new_stream(stream_ctx->ifmt_ctx, stream_ctx->in_codec);
  stream_ctx->in_codec_ctx = avcodec_alloc_context3(stream_ctx->in_codec);

  AVDictionary *codec_options = NULL;
  av_dict_set(&codec_options, "framerate", fps_str, 0);
  av_dict_set(&codec_options, "preset", "superfast", 0);

  avcodec_parameters_to_context(stream_ctx->in_codec_ctx, stream_ctx->ifmt_ctx->streams[0]->codecpar);
  if (avcodec_open2(stream_ctx->in_codec_ctx, stream_ctx->in_codec, &codec_options) != 0)
  {
    fprintf(stderr, "cannot initialize video decoder!\n");
    return 1;
  }

  return 0;
}

int init_output_avformat_context(stream_ctx_t *stream_ctx, const char *format_name)
{
  if (avformat_alloc_output_context2(&stream_ctx->ofmt_ctx, NULL, format_name, NULL) != 0)
  {
    fprintf(stderr, "cannot initialize output format context!\n");
    return 1;
  }

  return 0;
}

int init_io_context(stream_ctx_t *stream_ctx, const char *output_path)
{
  if (avio_open2(&stream_ctx->ofmt_ctx->pb, output_path, AVIO_FLAG_WRITE, NULL, NULL) != 0)
  {
    fprintf(stderr, "could not open IO context!\n");
    return 1;
  }

  return 0;
}

void set_codec_params(stream_ctx_t *stream_ctx, int width, int height, int fps)
{
  const AVRational dst_fps = {fps, 1};

  stream_ctx->out_codec_ctx->codec_tag = 0;
  stream_ctx->out_codec_ctx->codec_id = AV_CODEC_ID_H264;
  stream_ctx->out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
  stream_ctx->out_codec_ctx->width = width;
  stream_ctx->out_codec_ctx->height = height;
  stream_ctx->out_codec_ctx->gop_size = 12;
  stream_ctx->out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  stream_ctx->out_codec_ctx->framerate = dst_fps;
  stream_ctx->out_codec_ctx->time_base = av_inv_q(dst_fps);
  if (stream_ctx->ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
  {
    stream_ctx->out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
}

int init_codec_stream(stream_ctx_t *stream_ctx)
{
  if (avcodec_parameters_from_context(stream_ctx->out_stream->codecpar, stream_ctx->out_codec_ctx) != 0)
  {
    fprintf(stderr, "could not initialize stream codec parameters!\n");
    return 1;
  }

  AVDictionary *codec_options = NULL;
  av_dict_set(&codec_options, "profile", "high", 0);
  av_dict_set(&codec_options, "preset", "superfast", 0);
  av_dict_set(&codec_options, "tune", "zerolatency", 0);

  // open video encoder
  if (avcodec_open2(stream_ctx->out_codec_ctx, stream_ctx->out_codec, &codec_options) != 0)
  {
    fprintf(stderr, "could not open video encoder!\n");
    return 1;
  }

  return 0;
}

struct SwsContext *initialize_sample_scaler(AVCodecContext *codec_ctx, int width, int height)
{
  struct SwsContext *swsctx = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, codec_ctx->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
  if (!swsctx)
  {
    fprintf(stderr, "could not initialize sample scaler!");
  }

  return swsctx;
}

char *concat_str(const char *s1, const char *s2)
{
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);
  char *result = malloc(len1 + len2 + 1);
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1);
  return result;
}

const char *get_device_family()
{
#ifdef _WIN32
  const char *device_family = "dshow";
#elif __APPLE__
  const char *device_family = "avfoundation";
#elif __linux__
  const char *device_family = "v4l2";
#endif

  return device_family;
}

void handle_signal(int signal)
{
  fprintf(stderr, "Caught SIGINT, exiting now...\n");
  end_stream = true;
}

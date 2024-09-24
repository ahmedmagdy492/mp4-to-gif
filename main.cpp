#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdlib>

#include "include/utils.h"

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/imgutils.h>
  #include <libswscale/swscale.h>

  #include <gif_lib.h>
  
  #include "include/stb_image_write.h"
}

#define FRAMES_DIFF_RATIO 50

void CreateColorMap(ColorMapObject *cmap) {
  for(int i = 0; i < 256; ++i) {
    cmap->Colors[i].Red = i;
    cmap->Colors[i].Green = i;
    cmap->Colors[i].Blue = i;
  }
}

int WriteNetscapeLoopExtension(GifFileType *gifFile, int loopCount) {
    unsigned char nsAppId[] = {'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0'};
    if (EGifPutExtensionLeader(gifFile, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR) {
        return GIF_ERROR;
    }
    if (EGifPutExtensionBlock(gifFile, sizeof(nsAppId), nsAppId) == GIF_ERROR) {
        return GIF_ERROR;
    }

    unsigned char nsLoopBlock[] = {0x01, (unsigned char)(loopCount & 0xFF), (unsigned char)((loopCount >> 8) & 0xFF)};
    if (EGifPutExtensionBlock(gifFile, sizeof(nsLoopBlock), nsLoopBlock) == GIF_ERROR) {
        return GIF_ERROR;
    }

    if (EGifPutExtensionTrailer(gifFile) == GIF_ERROR) {
        return GIF_ERROR;
    }

    return GIF_OK;
}


int main(int argc, char** argv) {

  if(argc != 3) {
    fprintf(stderr, "Usage: %s <video-name.mp4> <no-of-frames-to-extract>\n", argv[1]);
    return 1;
  }

  int noFramesToExtract = atoi(argv[2]);

  AVFormatContext* formatContext = nullptr;
  if(avformat_open_input(&formatContext, argv[1], nullptr, nullptr) < 0) {
    fprintf(stderr, "cannot open file %s\n", argv[1]);
    return 1;
  }

  if(avformat_find_stream_info(formatContext, nullptr) < 0) {
    fprintf(stderr, "cannot extract info from media file\n");
    avformat_close_input(&formatContext);
    return 1;
  }

  printf("Found %u streams\n", formatContext->nb_streams);

  bool isVideo = false;
  int videoStreamIndex = -1;
  for(int i = 0; i < formatContext->nb_streams; ++i) {
    if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      isVideo = true;
      videoStreamIndex = i;
      break;
    }
  }

  if(!isVideo) {
    fprintf(stderr, "The given media container does not contain a video stream\n");
    avformat_close_input(&formatContext);
    return 1;
  }

  printf("Found a video stream\n");
  printf("Found %ld frames\n", formatContext->streams[videoStreamIndex]->nb_frames);

  if(formatContext->streams[videoStreamIndex]->codecpar->codec_id != AV_CODEC_ID_H264) {
    fprintf(stderr, "Only H264 codec is supported\n");
    avformat_close_input(&formatContext);
    return 1;
  }

  printf("Found an H264 encoded stream\n");

  const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if(!codec) {
    fprintf(stderr, "did not find a proper decoder for H264 codec\n");
    avformat_close_input(&formatContext);
    return 1;
  }

  printf("Found a decoder\n");

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if(!codecContext) {
    fprintf(stderr, "Unable to allocate a codec context\n");
    avformat_close_input(&formatContext);
    return 1;
  }

  if(avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar) < 0) {
    fprintf(stderr, "Failed to fill codec with read codec paras\n");
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  int width = formatContext->streams[videoStreamIndex]->codecpar->width;
  int height = formatContext->streams[videoStreamIndex]->codecpar->height;
  printf("Video size: %dx%d\n", width, height);

  if(avcodec_open2(codecContext, codec, nullptr) < 0) {
    fprintf(stderr, "unable to open the decoder\n");
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  if(noFramesToExtract > formatContext->streams[videoStreamIndex]->nb_frames) {
    fprintf(stderr, "No of frames given is larger than the no of frames in the input video stream\n");
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return 1;
  }

  AVPacket packet;
  int i = 0, counter = 0;
  const char* outputFile = "output/out.gif";
  int errCode = 0;
  
  GifFileType* gifFile = EGifOpenFileName(outputFile, false, &errCode);

  if(!gifFile) {
    fprintf(stderr, "Cannot open gif file: %s\n", GifErrorString(errCode));
  }

  int noColors = 256;
  ColorMapObject* colorMapObj = GifMakeMapObject(noColors, nullptr);

  if(!colorMapObj) {
    fprintf(stderr, "Cannot create a color map object\n");
  }

  CreateColorMap(colorMapObj);

  EGifSetGifVersion(gifFile, true);

  int ret = EGifPutScreenDesc(gifFile, width, height, 8, 0, colorMapObj);

  if(ret == GIF_ERROR) {
    fprintf(stderr, "Cannot set screen description: %s\n", GifErrorString(ret));
  }

  int loopCount = 0;
  int res = WriteNetscapeLoopExtension(gifFile, loopCount);
  if(res == GIF_ERROR) {
    fprintf(stderr, "Cannot write loop extension block\n");
  }

  uint8_t* prevFrame = new uint8_t[width * height * 3];
  int frameLimiter = 0;

  while(av_read_frame(formatContext, &packet) == 0 && frameLimiter < noFramesToExtract) {
    if(packet.stream_index == videoStreamIndex) {
      AVFrame * frame = av_frame_alloc();
    
      if(avcodec_send_packet(codecContext, &packet) < 0) {
      	av_frame_free(&frame);
	      continue;
      }

      if(avcodec_receive_frame(codecContext, frame) == 0) {
        if((frameLimiter % 2) == 0) {
          ret = EGifPutImageDesc(gifFile, 0, 0, width, height, false, nullptr);
          for(int j = 0; j < height; ++j) {
            ret = EGifPutLine(gifFile, frame->data[0] + width*j, width);
          }
        
          printf("Written frame %d out of %d frames\n", counter, noFramesToExtract);
          ++counter;
        }
        ++frameLimiter;
      }

      av_frame_free(&frame);
    }
    av_packet_unref(&packet);
    ++i;
  }

  EGifCloseFile(gifFile, NULL);
  GifFreeMapObject(colorMapObj);

  delete[] prevFrame;

  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);
  return 0;
}

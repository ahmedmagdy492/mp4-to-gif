#pragma once

#include <cstdint>
#include <stdexcept>
extern "C" {
  #include <gif_lib.h>
}
#include "color_map_generator.h"


class GifWriter {
private:
  GifFileType* m_gifFile = nullptr;
  int error_code = 0;
  ColorMapObject* cmap = nullptr;

public:
  GifWriter(const char* fileName) {
    m_gifFile = EGifOpenFileName(fileName, false, &error_code);
    if(m_gifFile == nullptr) {
      throw std::runtime_error("Unable to create the gif file");
    }
  }

  int SetupGif(int width, int height, ColorMapGenerator* colorGen) {
    EGifSetGifVersion(m_gifFile, true);

    cmap = colorGen->GenerateColorMap();
    return EGifPutScreenDesc(m_gifFile, width, height, 8, 0, cmap);
  }

  int WriteNetscapeLoopExtension(int loopCount) {
    unsigned char nsAppId[] = {'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0'};
    if (EGifPutExtensionLeader(m_gifFile, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR) {
        return GIF_ERROR;
    }
    if (EGifPutExtensionBlock(m_gifFile, sizeof(nsAppId), nsAppId) == GIF_ERROR) {
        return GIF_ERROR;
    }

    unsigned char nsLoopBlock[] = {0x01, (unsigned char)(loopCount & 0xFF), (unsigned char)((loopCount >> 8) & 0xFF)};
    if (EGifPutExtensionBlock(m_gifFile, sizeof(nsLoopBlock), nsLoopBlock) == GIF_ERROR) {
        return GIF_ERROR;
    }

    if (EGifPutExtensionTrailer(m_gifFile) == GIF_ERROR) {
        return GIF_ERROR;
    }

    return GIF_OK;
  }

  void WriteSingleImage(int width, int height, uint8_t* data) {
    EGifPutImageDesc(m_gifFile, 0, 0, width, height, false, nullptr);
    for(int j = 0; j < height; ++j) {
       EGifPutLine(m_gifFile, data + width*j, width);
    }
  }

  ~GifWriter() {
    EGifCloseFile(m_gifFile, NULL);
    if(cmap) {
      GifFreeMapObject(cmap);
    }
  }
};

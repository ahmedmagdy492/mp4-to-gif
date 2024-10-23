#pragma once

#include <stdexcept>
extern "C" {
  #include <gif_lib.h>
}


class ColorMapGenerator {
public:
  virtual ColorMapObject* GenerateColorMap() = 0;
};

class GrayScaleColorMapGenerator : public ColorMapGenerator {
private:
  ColorMapObject* cmap = nullptr;
  int noColors = 256;

public:
  GrayScaleColorMapGenerator(int noColors) : noColors(noColors) {
    if(noColors > 256) {
      throw std::invalid_argument("no colors should be at most 256");
    }
    cmap = GifMakeMapObject(noColors, nullptr);
  }

  ColorMapObject* GenerateColorMap() {
    for(int i = 0; i < noColors; ++i) {
      cmap->Colors[i].Red = i;
      cmap->Colors[i].Green = i;
      cmap->Colors[i].Blue = i;
    }

    return cmap;
  }
};

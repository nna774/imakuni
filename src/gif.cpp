#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <variant>

#include "gif.h"
#include "byte.h"
#include "read.h"
#include "to_string.h"

using std::begin;
using std::end;

namespace GIF {
  enum class GifType {
    GIF87a,
    GIF89a,
    NOTGIF,
  };

  std::string show(GifType t) {
    switch(t) {
      case GifType::GIF87a:
        return "GIF87a";
      case GifType::GIF89a:
        return "GIF89a";
      case GifType::NOTGIF:
      default:
        return "NOTGIF";
    }
  }

  GifType readType(std::istream& fs) {
    auto head = read<std::array<Byte, 6>>(fs);
    if(!(head[0] == 'G' && head[1] == 'I' && head[2] == 'F')) {
      return GifType::NOTGIF;
    }

    if(head[3] == '8' && head[4] == '7' && head[5] == 'a') {
      return GifType::GIF87a;
    }
    if(head[3] == '8' && head[4] == '9' && head[5] == 'a') {
      return GifType::GIF89a;
    }

    return GifType::NOTGIF;
  }

  struct Header {
    GifType type;
    int width, height;
    bool hasGct;
    int resolution;
    bool gctSorted;
    int gctSize;
    int bgColorIndex;
    double aspectRatio;
    std::vector<Pixel> gct;
  };

  std::unique_ptr<Image> load(std::istream& fs) {
    auto t = readType(fs);
    if(t == GifType::NOTGIF) {
      std::cerr << "not gif file" << std::endl;
      return nullptr;
    }

    return nullptr;
  }

  std::unique_ptr<Image> exportGIF(std::unique_ptr<Image>&& img, std::ostream& os) {
    return std::move(img);
  }

  int readSize(std::istream& fs) {
    auto size = read<std::array<Byte, 2>>(fs);
    return static_cast<int>(size[0]) + (static_cast<int>(size[1]) << 8);
  }

  Header readHeader(std::istream& fs, GifType t) {
    Header header;
    header.type = t;
    header.width = readSize(fs);
    header.height = readSize(fs);
    auto flags = read<Byte>(fs);
    header.hasGct = flags & 0x80;
    header.resolution = ((flags & 0x70) >> 4) + 1;
    header.gctSorted = flags & 0x08;
    header.gctSize = std::pow(2, (flags & 0x07) + 1);
    auto index = read<Byte>(fs);
    header.bgColorIndex = header.hasGct ? index : 0;
    auto aspect = read<Byte>(fs);
    header.aspectRatio = aspect ? ((aspect + 15.0) / 64) : 0;
    if(header.hasGct) {
      header.gct.reserve(header.gctSize);
      for(int i{}; i < header.gctSize; ++i) {
        header.gct[i] = read<Pixel>(fs);
      }
    }
    return header;
  }

  void showInfo(std::istream& fs) {
    auto t = readType(fs);
    if(t == GifType::NOTGIF) {
      std::cout << "not gif file" << std::endl;
      return;
    }
    auto header = readHeader(fs, t);
    std::cout << "this is gif" << std::endl;
    std::cout << "gif type is " << show(t) << std::endl;
    std::cout << "size is " << header.width << 'x' << header.height << std::endl;
    std::cout << "gct follow?: " << header.hasGct << ", resolution: " << header.resolution << ", sorted?: " << header.gctSorted << ", gctsize: " << header.gctSize << std::endl;
    std::cout << "bg index: " << header.bgColorIndex << ", aspect: " << header.aspectRatio << std::endl;

    return;
  }
}

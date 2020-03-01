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

  GifType type(std::istream& fs) {
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

  std::unique_ptr<Image> load(std::istream& fs) {
    auto t = type(fs);
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

  void showInfo(std::istream& fs) {
    auto t = type(fs);
    if(t == GifType::NOTGIF) {
      std::cout << "not gif file" << std::endl;
      return;
    }
    std::cout << "this is gif" << std::endl;
    std::cout << "gif type is " << show(t) << std::endl;

    int width{readSize(fs)};
    int height{readSize(fs)};
    std::cout << "size is " << width << 'x' << height << std::endl;

    auto flsgs = read<Byte>(fs);

    return;
  }
}

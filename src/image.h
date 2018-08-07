#include <vector>

#include "byte.h"

#pragma once

struct Pixel {
  Byte r, g, b;
  Pixel operator-(Pixel other) {
    Pixel p;
    p.r = static_cast<Byte>((static_cast<int>(this->r) - other.r + 256) % 256);
    p.g = static_cast<Byte>((static_cast<int>(this->g) - other.g + 256) % 256);
    p.b = static_cast<Byte>((static_cast<int>(this->b) - other.b + 256) % 256);
    return p;
  }
};

class Image {
public:
  Image(size_t width, size_t height, std::vector<Pixel> pixels) : _width{width}, _height{height}, _pixels{pixels} {}
  size_t width() { return _width; }
  size_t height() { return _height; }
  std::vector<Pixel> const& pixels() { return _pixels; }
private:
  size_t const _width;
  size_t const _height;
  std::vector<Pixel> _pixels;
};

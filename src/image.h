#include <vector>

#include "byte.h"

#pragma once

struct Pixel {
  Byte r, g, b;
  Pixel operator+(Pixel other) {
    Pixel p;
    auto add = [](Byte a, Byte b) { return static_cast<Byte>((static_cast<int>(a) + static_cast<signed char>(b) + 256) % 256); };
    p.r = add(this->r, other.r);
    p.g = add(this->g, other.g);
    p.b = add(this->b, other.b);
    return p;
  }
  Pixel operator-(Pixel other) {
    Pixel p;
    auto sub = [](Byte a, Byte b) { return static_cast<Byte>((static_cast<int>(a) - static_cast<signed char>(b) + 256) % 256); };
    p.r = sub(this->r, other.r);
    p.g = sub(this->g, other.g);
    p.b = sub(this->b, other.b);
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

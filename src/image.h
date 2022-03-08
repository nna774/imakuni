#include <cstddef>
#include <vector>

#include "byte.h"

#pragma once

struct Pixel {
  Byte r, g, b;
  Pixel() : r{}, g{}, b{} {}
  Pixel(Byte r_, Byte g_, Byte b_) : r{r_}, g{g_}, b{b_} {}
};

Pixel operator+(Pixel const& lhs, Pixel const& rhs);
Pixel operator-(Pixel const& lhs, Pixel const& rhs);

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

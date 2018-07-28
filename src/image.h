#include <vector>

#include "byte.h"

#pragma once

struct Pixel {
  Byte r, g, b;
};

class Image {
public:
  virtual size_t width() = 0;
  virtual size_t height() = 0;
  virtual std::vector<Pixel> const& pixels() = 0;
};

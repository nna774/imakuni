#include <vector>
#pragma once

class Pixel {
};

class Image {
public:
  Image();
  size_t width();
  size_t height();
  std::vector<Pixel> pixels();
};

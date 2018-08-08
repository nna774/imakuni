#include "image.h"

Pixel operator+(Pixel const& lhs, Pixel const& rhs) {
  Pixel p;
  auto add = [](Byte a, Byte b) { return static_cast<Byte>((static_cast<int>(a) + static_cast<signed char>(b) + 256) % 256); };
  p.r = add(lhs.r, rhs.r);
  p.g = add(lhs.g, rhs.g);
  p.b = add(lhs.b, rhs.b);
  return p;
}

Pixel operator-(Pixel const& lhs, Pixel const& rhs) {
  Pixel p;
  auto sub = [](Byte a, Byte b) { return static_cast<Byte>((static_cast<int>(a) - static_cast<signed char>(b) + 256) % 256); };
  p.r = sub(lhs.r, rhs.r);
  p.g = sub(lhs.g, rhs.g);
  p.b = sub(lhs.b, rhs.b);
  return p;
}

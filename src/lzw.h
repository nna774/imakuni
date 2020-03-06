#include <vector>

#include "byte.h"

#pragma once

namespace LZW {
  std::vector<Byte> compress(std::vector<Byte> const& src, size_t size);
  std::vector<Byte> decompress(std::vector<Byte> const& src, size_t size);
}

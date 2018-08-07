#include <vector>

#include "byte.h"

#pragma once

namespace Deflate {
  std::vector<Byte> compress(std::vector<Byte> const& src);
  std::vector<Byte> decompress(std::vector<Byte> const& src);
}

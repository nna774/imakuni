#include <vector>

#include "byte.h"

#pragma once

namespace Deflate {
  std::vector<Byte> decompress(std::vector<Byte> const& src);
}

#include <istream>
#include <memory>
#include "image.h"
#pragma once

namespace PNG {
  std::unique_ptr<Image> load(std::istream&);
}

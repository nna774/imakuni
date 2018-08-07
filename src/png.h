#include <istream>
#include <memory>
#include "image.h"
#pragma once

namespace PNG {
  std::unique_ptr<Image> load(std::istream&);
  std::unique_ptr<Image> exportPNG(std::unique_ptr<Image>&&, std::ostream&);
}

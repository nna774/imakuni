#include <istream>
#include <memory>
#include "image.h"
#pragma once

namespace GIF {
  std::unique_ptr<Image> load(std::istream&);
  std::unique_ptr<Image> exportGIF(std::unique_ptr<Image>&&, std::ostream&);
  void showInfo(std::istream&);
}

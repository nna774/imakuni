#include <istream>
#include <memory>
#include "image.h"
#pragma once

namespace JPG {
  std::unique_ptr<Image> load(std::istream&);
  std::unique_ptr<Image> exportJPG(std::unique_ptr<Image>&&, std::ostream&);
  void showInfo(std::istream&);
}

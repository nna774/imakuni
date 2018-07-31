#include <istream>
#include <memory>
#include "image.h"
#pragma once

namespace PNM {
  std::unique_ptr<Image> load(std::istream&);
  std::unique_ptr<Image> exportPNM(std::unique_ptr<Image>&&, std::ostream&);
}

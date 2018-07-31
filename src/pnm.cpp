#include "pnm.h"

namespace PNM {
  std::unique_ptr<Image> exportPNM(std::unique_ptr<Image>&& img, std::ostream& os) {
    size_t width = img->width();
    size_t height = img->height();
    std::vector<Pixel> const& ps = img->pixels();
    os << "P3" << std::endl;
    os << width << ' ' << height << std::endl;
    os << 255 << std::endl;
    for(auto e: ps) {
      os << static_cast<int>(e.r) << ' ' << static_cast<int>(e.g) << ' ' << static_cast<int>(e.b) << '\n';
    }
    std::flush(os);
    return std::move(img);
  }
}

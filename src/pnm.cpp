#include <string>
#include <iostream>

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

  std::unique_ptr<Image> load(std::istream& is) {
    std::string buf;
    std::getline(is, buf);
    if(buf == "P3") {
      // # からはじまるコメントが許容されているが、未対応である。
      int width, height;
      is >> width >> height;
      int max;
      is >> max;
      if (max != 255) { std::cerr << "max is not 255" << std::endl; }
      std::vector<Pixel> pixels(width * height);
      for(int i{0}; i < width * height; ++i) {
        int r, g, b;
        is >> r >> g >> b;
        pixels[i].r = r * 256 / (max + 1);
        pixels[i].g = g * 256 / (max + 1);
        pixels[i].b = b * 256 / (max + 1);
      }
      return std::make_unique<Image>(width, height, std::move(pixels));
    } else if (buf == "P1" || buf == "P2") {
      std::cerr << "unimpled yet" << std::endl;
      return nullptr;
    } else {
      std::cerr << "not pnm file" << std::endl;
      return nullptr;
    }
  }
}

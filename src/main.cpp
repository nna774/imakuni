#include <iostream>
#include <fstream>

#include "png.h"

namespace PNM {
  std::unique_ptr<Image> exportPNM(std::unique_ptr<Image>&& img, std::ostream& os) {
    size_t width = img->width();
    size_t height = img->height();
    std::vector<Pixel> const& ps = img->pixels();
    os << "P3" << std::endl;
    os << width << ' ' << height << std::endl;
    os << 255 << std::endl;
    for(auto e: ps) {
      os << static_cast<int>(e.r) << ' ' << static_cast<int>(e.g) << ' ' << static_cast<int>(e.b) << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  std::unique_ptr<Image> img;
  if(argc >= 2) {
    std::ifstream fs{argv[1], std::ifstream::binary};
    if (!fs.is_open()) {
      std::cerr << "failed to open" << std::endl;
      return -1;
    }
    img = PNG::load(fs);
  } else {
    img = PNG::load(std::cin);
  }

  PNM::exportPNM(std::move(img), std::cout);
  return 0;
}

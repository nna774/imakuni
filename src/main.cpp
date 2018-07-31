#include <iostream>
#include <fstream>

#include "png.h"
#include "pnm.h"

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

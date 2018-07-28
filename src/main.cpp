#include <iostream>
#include <fstream>

#include "png.h"

int main(int argc, char** argv) {
  if(argc >= 2) {
    std::ifstream fs{argv[1], std::ifstream::binary};
    if (!fs.is_open()) {
      std::cerr << "failed to open" << std::endl;
      return -1;
    }
    PNG::load(fs);
  } else {
    PNG::load(std::cin);
  }
  return 0;
}

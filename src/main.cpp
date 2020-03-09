#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <functional>

#include "png.h"
#include "pnm.h"
#include "gif.h"

template<class T, class... Args>
inline std::array<T, sizeof...(Args)> make_array(Args &&... args) {
  return std::array< T, sizeof...(Args) >{ std::forward<Args>(args)... };
}

using loadType = std::function<std::unique_ptr<Image>(std::istream&)>;
using exportType = std::function<std::unique_ptr<Image>(std::unique_ptr<Image>&&, std::ostream&)>;
using infoType = std::function<void(std::istream&)>;
auto availableExts = make_array<std::tuple<std::string, loadType, exportType, infoType>>(
  std::make_tuple("png", PNG::load, PNG::exportPNG, nullptr),
  std::make_tuple("pnm", PNM::load, PNM::exportPNM, nullptr),
  std::make_tuple("gif", GIF::load, GIF::exportGIF, GIF::showInfo)
);

auto hasSuffix = [](std::string const& str, std::string const& suffix) {
  return str.size() >= suffix.size() && str.find(suffix, str.size() - suffix.size()) != std::string::npos;
};

int main(int argc, char** argv) {
  std::unique_ptr<Image> img;
  if(argc < 2) {
    std::cerr << argv[0] << " infile" << std::endl;
    std::cerr << argv[0] << " infile outfile" << std::endl;
    return -1;
  }
  if(std::string{argv[1]} == "show") {
    std::string in{argv[2]};
    for(auto e: availableExts) {
      auto ext = std::get<0>(e);
      auto info = std::get<3>(e);
      if(hasSuffix(in, "." + ext)) {
        std::ifstream fs{in, std::ifstream::binary};
        if (!fs.is_open()) {
          std::cerr << "failed to open " << in << std::endl;
          return -1;
        }
        if(info != nullptr) {
          info(fs);
          return 0;
        }
      }
    }
    std::cerr << "unknown type" << std::endl;
    return -1;
  }

  if(std::string{argv[1]} == "convert") {
    std::string in{argv[2]}, out{argv[3]};
    bool okIn{false}, okOut{false};
    for(auto e: availableExts) {
      auto ext = std::get<0>(e);
      auto load = std::get<1>(e);
      if(in == ext + ":-") {
        img = load(std::cin);
      } else if (hasSuffix(in, "." + ext)) {
        std::ifstream fs{in, std::ifstream::binary};
        if (!fs.is_open()) {
          std::cerr << "failed to open " << in << std::endl;
          return -1;
        }
        img = load(fs);
      }
      if(img) {
        okIn = true;
        break;
      }
    }

    if(!okIn) {
      std::cerr << "input file " << in << " is not supported." << std::endl;
      return -1;
    }

    if(!img) {
      std::cerr << "something wrong while loading " << in << "." << std::endl;
      return -1;
    }

    for(auto e: availableExts) {
      auto ext = std::get<0>(e);
      auto export_ = std::get<2>(e);
      if(out == ext + ":-") {
        export_(std::move(img), std::cout);
        okOut = true;
      } else if (hasSuffix(out, "." + ext)) {
        std::ofstream fs{out, std::ofstream::binary};
        if (!fs.is_open()) {
          std::cerr << "failed to open " << out << std::endl;
          return -1;
        }
        export_(std::move(img), fs);
        okOut = true;
      }
    }
    if(!okOut) {
      std::cerr << "output file " << out << " is not supported." << std::endl;
    }
  }

  return 0;
}

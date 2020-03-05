#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <optional>
#include <variant>

#include "gif.h"
#include "byte.h"
#include "read.h"
#include "to_string.h"

using std::begin;
using std::end;

namespace GIF {
  enum class GifType {
    GIF87a,
    GIF89a,
  };

  std::string show(GifType t) {
    switch(t) {
      case GifType::GIF87a:
        return "GIF87a";
      case GifType::GIF89a:
        return "GIF89a";
      default:
        return "NOTGIF";
    }
  }

  std::optional<GifType> readType(std::istream& fs) {
    auto head = read<std::array<Byte, 6>>(fs);
    if(!(head[0] == 'G' && head[1] == 'I' && head[2] == 'F')) {
      return std::nullopt;
    }

    if(head[3] == '8' && head[4] == '7' && head[5] == 'a') {
      return GifType::GIF87a;
    }
    if(head[3] == '8' && head[4] == '9' && head[5] == 'a') {
      return GifType::GIF89a;
    }

    return std::nullopt;
  }

  struct Header {
    GifType type;
    int width, height;
    bool hasGct;
    int resolution;
    bool gctSorted;
    int gctSize;
    int bgColorIndex;
    double aspectRatio;
    std::vector<Pixel> gct;
  };

  Byte static const ImageSeparator{0x2C};
  Byte static const ExtensionIntroducer{0x21};
  Byte static const GifTerminator{0x3b};

  struct ImageDescripter {
    int leftPos;
    int topPos;
    int width;
    int height;
    bool hasLct;
    bool interlaced;
    bool lctSorted;
    int lctSize;
    std::vector<Pixel> lct;
    int lzwSize;
    std::vector<Byte> imageData;
  };
  std::string show(ImageDescripter desc) {
    std::stringstream ss;
    ss << "Image Descripter" << std::endl;
    ss << "  hasLct?: " << desc.hasLct;

    return ss.str();
  }

  Byte static const ApplicationExtensionLabel{0xff};
  Byte static const GraphicControlExtensionLabel{0xf9};

  struct ImageExtension {
    int functionCode;
    std::vector<Byte> data;
  };
  std::string show(ImageExtension ext) {
    std::stringstream ss;
    ss << "Image Extension code: 0x" << std::hex << ext.functionCode;
    if(ext.functionCode == 0xfe) {
      ss << "ext type: comment" << std::endl;
      for(auto e: ext.data) {
        ss << static_cast<char>(e);
      }
    }
    return ss.str();
  }
  struct ApplicationExtension {
    std::string identifier;
    std::array<Byte, 3> authenticationCode;
    std::vector<Byte> data;
  };
  std::string show(ApplicationExtension ext) {
    std::stringstream ss;
    ss << "Application Extension" << std::endl;
    ss << "  identifier: " << ext.identifier << std::endl;
    ss << "  auth code: ";
    for(auto e: ext.authenticationCode) {
      ss << static_cast<char>(e);
    }
    ss << std::endl << "  data: ";
    for(auto e: ext.data) {
      ss << static_cast<char>(e);
    }

    return ss.str();
  }
  struct GraphicControlExtension {
    int disposalMethod;
    bool expectUserInput;
    bool hasTransparentColor;
    int delayTime;
    int transparentColorIndex;
  };
  std::string show(GraphicControlExtension ext) {
    std::stringstream ss;
    ss << "GraphicControlExtension" << std::endl;
    ss << "  disposalMethod: " << ext.disposalMethod << ", expectUserInput?: " << ext.expectUserInput << ", hasTransparent?: " << ext.hasTransparentColor << ", delay: " << ext.delayTime << ", trans index: " << ext.transparentColorIndex;

    return ss.str();
  }

  class EndOfBlock {};
  std::string show(EndOfBlock) {
    return "end of block";
  }

  using Block = std::variant<
    ImageDescripter,
    ImageExtension,
    ApplicationExtension,
    GraphicControlExtension,
    EndOfBlock
  >;

  std::unique_ptr<Image> load(std::istream& fs) {
    auto t = readType(fs);
    if(!t) {
      std::cerr << "not gif file" << std::endl;
      return nullptr;
    }

    return nullptr;
  }

  std::unique_ptr<Image> exportGIF(std::unique_ptr<Image>&& img, std::ostream& os) {
    return std::move(img);
  }

  int readSize(std::istream& fs) {
    auto size = read<std::array<Byte, 2>>(fs);
    return static_cast<int>(size[0]) + (static_cast<int>(size[1]) << 8);
  }

  std::optional<Header> readHeader(std::istream& fs, GifType t) {
    Header header;
    header.type = t;
    header.width = readSize(fs);
    header.height = readSize(fs);
    auto flags = read<Byte>(fs);
    header.hasGct = flags & 0x80;
    header.resolution = ((flags & 0x70) >> 4) + 1;
    header.gctSorted = flags & 0x08;
    header.gctSize = std::pow(2, (flags & 0x07) + 1);
    auto index = read<Byte>(fs);
    header.bgColorIndex = header.hasGct ? index : 0;
    auto aspect = read<Byte>(fs);
    header.aspectRatio = aspect ? ((aspect + 15.0) / 64) : 0;
    if(header.hasGct) {
      header.gct.reserve(header.gctSize);
      for(int i{}; i < header.gctSize; ++i) {
        header.gct[i] = read<Pixel>(fs);
      }
    }
    return header;
  }

  Block readImageDiscripter(std::istream& fs) {
    ImageDescripter desc;
    desc.leftPos = readSize(fs);
    desc.topPos = readSize(fs);
    desc.width = readSize(fs);
    desc.height = readSize(fs);
    auto flags = read<Byte>(fs);
    desc.hasLct = flags & 0x80;
    desc.interlaced = flags & 0x40;
    desc.lctSorted = flags & 0x20;
    desc.lctSize = std::pow(2, (flags & 0x07) + 1);

    if(desc.hasLct) {
      desc.lct.reserve(desc.lctSize);
      for(int i{}; i < desc.lctSize; ++i) {
        desc.lct[i] = read<Pixel>(fs);
      }
    }

    desc.lzwSize = read<Byte>(fs);
    int blockSize;
    while(blockSize = read<Byte>(fs), blockSize) {
      for(int i{}; i < blockSize; ++i) {
        auto d = read<Byte>(fs);
        desc.imageData.push_back(d);
      }
    }

    return desc;
  }

  Block readApplicationExtension(std::istream& fs) {
    auto fixed = read<Byte>(fs);
    if(fixed != 11) {
      std::cout << "unexpected size" << std::endl;
    }
    ApplicationExtension ext;
    auto identifier = read<std::array<Byte, 8>>(fs);
    std::string buf;
    for(auto e: identifier) { buf += e; }
    ext.identifier = buf;

    ext.authenticationCode = read<std::array<Byte, 3>>(fs);

    int size;
    while(size = read<Byte>(fs), size) {
      auto d = read<Byte>(fs);
      ext.data.push_back(d);
    }

    return ext;
  }
  Block readGraphicControlExtension(std::istream& fs) {
    auto fixed = read<Byte>(fs);
    if(fixed != 4) {
      std::cout << "unexpected size" << std::endl;
    }

    GraphicControlExtension ext;
    auto flags = read<Byte>(fs);
    ext.disposalMethod = (flags & 0b11100) >> 2;
    ext.expectUserInput = flags & 0b10;
    ext.hasTransparentColor = flags & 1;
    ext.delayTime = readSize(fs);
    ext.transparentColorIndex = read<Byte>(fs);

    auto terminator = read<Byte>(fs);
    if(terminator != 0) {
      std::cout << "?" << std::endl;
    }

    return ext;
  }

  Block readImageExtension(std::istream& fs) {
    ImageExtension ext;
    ext.functionCode = read<Byte>(fs);
    if(ext.functionCode == GraphicControlExtensionLabel) {
      return readGraphicControlExtension(fs);
    } else if(ext.functionCode == ApplicationExtensionLabel) {
      return readApplicationExtension(fs);
    }
    Byte cnt;
    while(cnt = read<Byte>(fs), cnt) {
      for(int i{}; i < cnt; ++i) {
        auto d = read<Byte>(fs);
        ext.data.push_back(d);
      }
    }

    return ext;
  }

  std::optional<Block> readBlock(std::istream& fs) {
    auto sep = read<Byte>(fs);
    if(sep == ImageSeparator) {
      return readImageDiscripter(fs);
    } else if(sep == ExtensionIntroducer) {
      return readImageExtension(fs);
    } else if(sep == GifTerminator){
      return EndOfBlock{};
    } else {
      std::cout << "unknown block" << std::endl;
      return std::nullopt;
    }
  }

  std::vector<Block> readBlocks(std::istream& fs) {
    std::optional<Block> block;
    std::vector<Block> blocks;
    while(block = readBlock(fs), block && std::holds_alternative<EndOfBlock>(*block)) {
      blocks.push_back(*block);
    }
    return blocks;
  }

  void showInfo(std::istream& fs) {
    auto t = readType(fs);
    if(!t) {
      std::cout << "not gif file" << std::endl;
      return;
    }
    auto optHeader = readHeader(fs, *t);
    if(!optHeader) { return; }
    auto header = *optHeader;
    std::cout << "this is gif" << std::endl;
    std::cout << "gif type is " << show(*t) << std::endl;
    std::cout << "size is " << header.width << 'x' << header.height << std::endl;
    std::cout << "hasGct?: " << header.hasGct << ", resolution: " << header.resolution << ", sorted?: " << header.gctSorted << ", gctsize: " << header.gctSize << std::endl;
    std::cout << "bg index: " << header.bgColorIndex << ", aspect: " << header.aspectRatio << std::endl;

    auto blocks = readBlocks(fs);
    std::cout << blocks.size() << "blocks" << std::endl;
    for(auto e: blocks) {
      std::cout << std::visit([](auto& x) { return show(x); }, e) << std::endl;
    }

    return;
  }
}

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cctype>
#include <cassert>

#include "png.h"
#include "byte.h"
#include "deflate.h"

namespace PNG {
  class PNG : public Image {
  public:
    PNG(size_t width, size_t height) : _width{width}, _height{height} {}
    size_t width() { return _width; }
    size_t height() { return _height; }
    std::vector<Pixel> const& pixels() { }
  private:
    size_t const _width;
    size_t const _height;
  };

  class Chunk {
  public:
    Chunk(std::string const& type) : _type{type} {}
    std::string type() { return _type; }
    bool isCritical() { return std::isupper(_type[0]); }
    bool isPublic() { return std::isupper(_type[1]); }
    bool isSafe() { return std::isupper(_type[3]); }
    virtual ~Chunk() {}
  private:
    std::string const _type;
  };

  class IHDRChunk : public Chunk {
  public:
    IHDRChunk(size_t width, size_t height) : Chunk{"IHDR"}, _width{width}, _height{height} {}
    size_t width() { return _width; }
    size_t height() { return _height; }
  private:
    size_t const _width;
    size_t const _height;
  };

  class IDATChunk : public Chunk {
  public:
    IDATChunk(std::vector<Byte> const& data) : Chunk{"IDAT"}, _data{data} {}
    std::vector<Byte> const& data() { return _data; }
  private:
    std::vector<Byte> const _data;
  };

  void _read(std::istream& fs, char* p, size_t size) {
    fs.read(p, size);
  }

  void _read(std::istream& fs, Byte* p, size_t size) {
    fs.read(reinterpret_cast<char*>(p), size);
  }

#define read(fs, p) _read((fs), (p), sizeof(p))
#define readWithSize(fs, p, size) _read((fs), (p), (size))

  template <typename To, typename From>
  std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& p) {
    if (To* cast = dynamic_cast<To*>(p.get())) {
      std::unique_ptr<To> result(cast);
      p.release();
      return result;
    }
    return std::unique_ptr<To>(nullptr);
  }

  bool readHeader(std::istream& fs) {
    Byte sig[8];
    read(fs, sig);
    if(!(sig[0] == 0x89 &&
         sig[1] == 0x50 &&
         sig[2] == 0x4e &&
         sig[3] == 0x47 &&
         sig[4] == 0x0d &&
         sig[5] == 0x0a &&
         sig[6] == 0x1a &&
         sig[7] == 0x0a)) {
      return false;
    }
    return true;
  }

  size_t readSize(std::istream& fs) {
    Byte sizes[4];
    read(fs, sizes);
    return (sizes[0] << 24) + (sizes[1] << 16) + (sizes[2] << 8) + sizes[3];
  }

  std::unique_ptr<Chunk> readIHDR(std::istream& fs) {
    size_t width = readSize(fs);
    size_t height = readSize(fs);
    std::cout << width << ' ' << height;
    Byte other[9];
    read(fs, other);
    return std::unique_ptr<Chunk>{
      new IHDRChunk{
        width,
        height,
      }
    };
  }

  std::unique_ptr<Chunk> readIDAT(std::istream& fs, size_t size) {
    std::vector<Byte> data(size);
    readWithSize(fs, data.data(), size);
    char crc[4];
    read(fs, crc);
    return std::unique_ptr<Chunk>{new IDATChunk{data}};
  }

  std::unique_ptr<Chunk> readIEND(std::istream& fs) {
    char crc[4];
    read(fs, crc);
    return std::unique_ptr<Chunk>{new Chunk{"IEND"}};
  }

  std::unique_ptr<Chunk> readChunk(std::istream& fs) {
    size_t size = readSize(fs);
    char types[4];
    read(fs, types);
    std::string type{types};
    if(type == "IHDR") {
      return readIHDR(fs);
    } else if(type == "IDAT") {
      return readIDAT(fs, size);
    } else if(type == "IEND") {
      return readIEND(fs);
    }
    assert(!"never come");
  }

  std::vector<std::unique_ptr<Chunk>> readChunks(std::istream& fs) {
    std::vector<std::unique_ptr<Chunk>> v;
    std::unique_ptr<Chunk> c;
    std::string type;
    do {
      c = readChunk(fs);
      type = c->type();
      std::cout << type << std::endl;
      v.push_back(std::move(c));
    } while(type != "IEND");
    return v;
  }

  std::unique_ptr<Image> load(std::istream& fs) {
    if(!readHeader(fs)) {
      std::cerr << "not png file" << std::endl;
      return nullptr;
    }

    std::vector<std::unique_ptr<Chunk>> chunks = readChunks(fs);
    std::unique_ptr<IHDRChunk> ihdr = dynamic_unique_cast<IHDRChunk>(std::move(chunks[0]));
    PNG png{ihdr->width(), ihdr->height()};
  }
}

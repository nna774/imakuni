#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <sstream>

#include "png.h"
#include "byte.h"
#include "deflate.h"

using std::begin;
using std::end;

namespace PNG {
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
    IHDRChunk(
      size_t width,
      size_t height,
      int depth,
      int colorType,
      int compression,
      int filter,
      int interlace
      ) :
      Chunk{"IHDR"},
      _width{width},
      _height{height},
      _depth{depth},
      _colorType{colorType},
      _compression{compression},
      _filter{filter},
      _interlace{interlace}
    {}
    size_t width() { return _width; }
    size_t height() { return _height; }
    int depth() { return _depth; }
    int colorType() { return _colorType; }
    int compression() { return _compression; }
    int filter() { return _filter; }
    int interlace() { return _interlace; }
  private:
    size_t const _width;
    size_t const _height;
    int const _depth;
    int const _colorType;
    int const _compression;
    int const _filter;
    int const _interlace;
  };

  class IDATChunk : public Chunk {
  public:
    IDATChunk(std::vector<Byte> const& data) : Chunk{"IDAT"}, _data{data} {}
    IDATChunk(std::vector<Byte>&& data) : Chunk{"IDAT"}, _data{std::move(data)} {}
    std::vector<Byte> const& data() { return _data; }
  private:
    std::vector<Byte> const _data;
  };

  template<typename T>
  void read(std::istream& fs, T& p) {
    fs.read(reinterpret_cast<char*>(&p), sizeof(T));
  }
  template<>
  void read(std::istream& fs, std::vector<Byte>& arr) {
    fs.read(reinterpret_cast<char*>(arr.data()), arr.size());
  }
  template<size_t N>
  void read(std::istream& fs, std::array<Byte, N>& arr) {
    fs.read(reinterpret_cast<char*>(arr.data()), N);
  }

  template <typename To, typename From>
  std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& p) {
    if (To* cast = dynamic_cast<To*>(p.get())) {
      std::unique_ptr<To> result(cast);
      p.release();
      return result;
    }
    return std::unique_ptr<To>(nullptr);
  }

  template<size_t N>
  std::string to_str(std::array<Byte, N> const& arr) {
    std::stringstream buf;
    for(size_t i{0}; i < N; ++i) {
      buf << std::hex << static_cast<int>(arr[i]);
    }
    return buf.str();
  }

  constexpr std::array<uint32_t, 256> makeCrcTable() {
    std::array<uint32_t, 256> table{};
    for(int i{0}; i < 256; ++i) {
      uint32_t c{static_cast<uint32_t>(i)};
      for(int j{0}; j < 8; ++j){
        if(c & 1) {
          c = UINT32_C(0xedb88320) ^ (c >> 1);
        } else {
          c >>= 1;
        }
      }
      table[i] = c;
    }
    return table;
  }
  std::array<uint32_t, 256> constexpr const crcTable = makeCrcTable();

  std::array<Byte, 4> crc(std::vector<Byte> const& data) {
    uint32_t _crc = UINT32_C(0xffffffff);
    for(auto b: data) {
      _crc = crcTable[(_crc ^ b) & 0xff] ^ (_crc >> 8);
    }
    _crc = ~_crc;
    std::array<Byte, 4> c{};
    c[3] = _crc & 0xff;
    c[2] = (_crc & 0xff00) >> 8;
    c[1] = (_crc & 0xff0000) >> 16;
    c[0] = (_crc & 0xff000000) >> 24;
    return c;
  }

  std::array<Byte, 8> const pngSigneture = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

  bool readHeader(std::istream& fs) {
    std::array<Byte, 8> sig;
    read(fs, sig);
    for(int i{0}; i < 8; ++i) {
      if(sig[i] != pngSigneture[i]) { return false; }
    }
    return true;
  }

  size_t readSize(std::istream& fs) {
    std::array<Byte, 4> sizes;
    read(fs, sizes);
    return (sizes[0] << 24) + (sizes[1] << 16) + (sizes[2] << 8) + sizes[3];
  }

  std::unique_ptr<Chunk> readIHDR(std::istream& fs) {
    size_t width = readSize(fs);
    size_t height = readSize(fs);
    Byte others[5];
    read(fs, others);
    char crc[4];
    read(fs, crc);
    return std::unique_ptr<Chunk>{
      new IHDRChunk{
        width,
        height,
        others[0], // depth
        others[1], // color type
        others[2], // compression method
        others[3], // filter method
        others[4], // interlace method
      }
    };
  }

  std::unique_ptr<Chunk> readIDAT(std::istream& fs, size_t size) {
    std::vector<Byte> data(size);
    read(fs, data);
    char crc[4];
    read(fs, crc);
    return std::unique_ptr<Chunk>{new IDATChunk{data}};
  }

  std::unique_ptr<Chunk> readIEND(std::istream& fs) {
    std::array<Byte, 4> _crc;
    read(fs, _crc);
    std::array<Byte, 4> expected = crc({'I', 'E', 'N', 'D'});
    if(_crc != expected) {
      std::cerr << "crc mismatched at IEND chunk(expected " << to_str(expected) << ", but got " << to_str(_crc) << ")." << std::endl;
    }

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
    } else {
      std::cerr << "unknown chunk type: " << type << std::endl;
      std::cerr << "size: " << size << std::endl;
      fs.ignore(size + 4); // 4 is crc
      return std::unique_ptr<Chunk>{new Chunk{type}};
    }
  }

  std::vector<std::unique_ptr<Chunk>> readChunks(std::istream& fs) {
    std::vector<std::unique_ptr<Chunk>> v;
    std::unique_ptr<Chunk> c;
    std::string type;
    do {
      c = readChunk(fs);
      type = c->type();
      std::cerr << type << std::endl;
      v.push_back(std::move(c));
    } while(type != "IEND");
    return v;
  }

  std::vector<Byte> concatIDAT(std::vector<std::unique_ptr<Chunk>>& cs) {
    std::vector<Byte> v;
    for(auto& c : cs) {
      if(c->type() == "IDAT") {
        auto ic = dynamic_unique_cast<IDATChunk>(std::move(c));
        auto d = ic->data();
        v.insert(end(v), begin(d), end(d));
        c = std::make_unique<Chunk>("IDAT");
      }
    }
    return v;
  }

  std::vector<Pixel> render(std::unique_ptr<IHDRChunk> ihdr, std::vector<Byte>& data) {
    size_t const width = ihdr->width();
    size_t const height = ihdr->height();
    int const depth = ihdr->depth();
    int const colorType = ihdr->colorType();
    int const filter = ihdr->filter();
    std::vector<Pixel> pixels(width * height);
    for(size_t h{0}; h < height; ++h) {
      for(size_t w{0}; w < width; ++w) {
        int i = h * width + w;
        Pixel p;
        switch(colorType) {
        case 0: // grayscale
        {
          switch(filter) {
          case 0: // None
            if(depth == 1) {
              p.r = p.g = p.b = data[h * (width + 1) + w + 1] ? 255 : 0;
            } else {
              p.r = p.g = p.b = data[h * (width + 1) + w + 1];
            }
          }
          break;
        }
        case 2: // color
        {
          auto add = [](Byte a, Byte b) { return Byte((static_cast<int>(a) + static_cast<signed char>(b) + 256) % 256); };
          switch(data[h * (width * 3 + 1)]) { // filtertype
          case 0: // None
          {
            int base = h * (width * 3 + 1) + w * 3 + 1;
            p.r = data[base];
            p.g = data[base + 1];
            p.b = data[base + 2];
            break;
          }
          case 1:
          {
            int base = h * (width * 3 + 1) + w * 3 + 1;
            if(w == 0) {
              p.r = data[base];
              p.g = data[base + 1];
              p.b = data[base + 2];
            } else {
              Pixel b = pixels[i - 1];
              p.r = add(b.r, data[base]);
              p.g = add(b.g, data[base + 1]);
              p.b = add(b.b, data[base + 2]);
            }
            break;
          }
          case 2:
          {
            int base = h * (width * 3 + 1) + w * 3 + 1;
            if(h == 0) {
              p.r = data[base];
              p.g = data[base + 1];
              p.b = data[base + 2];
            } else {
              Pixel b = pixels[i - width];
              p.r = add(b.r, data[base]);
              p.g = add(b.g, data[base + 1]);
              p.b = add(b.b, data[base + 2]);
            }
            break;
          }
          case 3:
          {
            int base = h * (width * 3 + 1) + w * 3 + 1;
            if(w == 0 || h == 0) {
              p.r = data[base];
              p.g = data[base + 1];
              p.b = data[base + 2];
            } else {
              Pixel b1 = pixels[i - 1];
              Pixel b2 = pixels[i - width];

              p.r = add((static_cast<int>(b1.r) + b2.r) / 2, data[base]);
              p.g = add((static_cast<int>(b1.g) + b2.g) / 2, data[base + 1]);
              p.b = add((static_cast<int>(b1.b) + b2.b) / 2, data[base + 2]);
            }
            break;
          }
          case 4:
          {
            int base = h * (width * 3 + 1) + w * 3 + 1;
            if(w == 0 || h == 0) {
              p.r = data[base];
              p.g = data[base + 1];
              p.b = data[base + 2];
            } else {
              Pixel left = pixels[i - 1];
              Pixel up = pixels[i - width];
              Pixel naname = pixels[i - width - 1];
              auto paethPredictor = [](Byte a, Byte b, Byte c) {
                int pp = static_cast<int>(a) + b - c;
                int pa = std::abs(pp - a);
                int pb = std::abs(pp - b);
                int pc = std::abs(pp - c);

                if (pa <= pb && pa <= pc) {
                  return a;
                } else if (pb <= pc) {
                  return b;
                } else {
                  return c;
                }
              };

              p.r = add(paethPredictor(left.r, up.r, naname.r), data[base]);
              p.g = add(paethPredictor(left.g, up.g, naname.g), data[base + 1]);
              p.b = add(paethPredictor(left.b, up.b, naname.b), data[base + 2]);
            }
            break;
          }
          }
        }
        pixels[i] = p;
        }
      }
    }
    return pixels;
  }

  std::unique_ptr<Image> load(std::istream& fs) {
    if(!readHeader(fs)) {
      std::cerr << "not png file" << std::endl;
      return nullptr;
    }

    std::vector<std::unique_ptr<Chunk>> chunks = readChunks(fs);
    auto ihdr = dynamic_unique_cast<IHDRChunk>(std::move(chunks[0]));
    size_t width = ihdr->width();
    size_t height = ihdr->height();
   std::cerr
      << ihdr->depth() << ' '
      << ihdr->colorType() << ' '
      << ihdr->compression() << ' '
      << ihdr->filter() << ' '
      << ihdr->interlace() << std::endl;
    chunks[0] = std::make_unique<Chunk>("IHDR");
    std::vector<Byte> data = concatIDAT(chunks);
    data = Deflate::decompress(data);
// //    std::cout << "size: " << data.size() << std::endl;
    // for(auto e: data) {
    //   std::cout << int(e) << ' ';
    // }
    // std::cout << std::endl;
    std::vector<Pixel> pixels = render(std::move(ihdr), data);
    return std::make_unique<Image>(width, height, std::move(pixels));
  }
}

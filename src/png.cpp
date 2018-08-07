#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <sstream>
#include <algorithm>

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
      Byte depth,
      Byte colorType,
      Byte compression,
      Byte filter,
      Byte interlace
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
    Byte depth() { return _depth; }
    Byte colorType() { return _colorType; }
    Byte compression() { return _compression; }
    Byte filter() { return _filter; }
    Byte interlace() { return _interlace; }
  private:
    size_t const _width;
    size_t const _height;
    Byte const _depth;
    Byte const _colorType;
    Byte const _compression;
    Byte const _filter;
    Byte const _interlace;
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
  class vector_view {
  public:
    vector_view(std::vector<T>&& v) : _v{v}, _pos{begin(_v)} {}
    typename std::vector<T>::iterator& pos() {
      assert(_pos != end(_v));
      return _pos;
    }
    std::vector<T>& data() { return _v; }
  private:
    std::vector<T> _v;
    typename std::vector<T>::iterator _pos;
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

  template<typename T>
  void read(vector_view<Byte>& v, T& p) {
    Byte* pp = reinterpret_cast<Byte*>(&p);
    for(size_t i{0}; i < sizeof(T); ++i) {
      pp[i] = *(v.pos()++);
    }
  }
  template<>
  void read(vector_view<Byte>& v, std::vector<Byte>& arr) {
    std::copy_n(v.pos(), arr.size(), begin(arr));
    std::advance(v.pos(), arr.size());
  }
  template<typename T, size_t N>
  void read(vector_view<Byte>& v, std::array<T, N>& p) {
    std::copy_n(v.pos(), N, reinterpret_cast<char*>(begin(p)));
    std::advance(v.pos(), N);
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

  Pixel average(Pixel lhs, Pixel rhs) {
    auto ave = [](Byte a, Byte b) { return (static_cast<int>(a) + b) / 2; };
    Pixel p;
    p.r = ave(lhs.r, rhs.r);
    p.g = ave(lhs.g, rhs.g);
    p.b = ave(lhs.b, rhs.b);
    return p;
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

  template<typename T>
  size_t readSize(T& fs) {
    std::array<Byte, 4> sizes;
    read(fs, sizes);
    return (sizes[0] << 24) + (sizes[1] << 16) + (sizes[2] << 8) + sizes[3];
  }

  std::unique_ptr<Chunk> readIHDR(vector_view<Byte>& view) {
    size_t width = readSize(view);
    size_t height = readSize(view);
    std::array<Byte,5> others;
    read(view, others);
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

  std::unique_ptr<Chunk> readIDAT(vector_view<Byte>& view, size_t size) {
    std::vector<Byte> data(size);
    read(view, data);
    return std::unique_ptr<Chunk>{new IDATChunk{std::move(data)}};
  }

  std::unique_ptr<Chunk> readChunk(std::istream& fs) {
    size_t const size = readSize(fs);
    std::vector<Byte> buf(size + 4); // 4 is type
    read(fs, buf);
    vector_view<Byte> view{std::move(buf)};
    std::array<char, 4> types;
    read(view, types);
    std::string type{begin(types), end(types)};
    std::unique_ptr<Chunk> chunk;
    if(type == "IHDR") {
      chunk = readIHDR(view);
    } else if(type == "IDAT") {
      chunk = readIDAT(view, size);
    } else if(type == "IEND") {
      chunk = std::make_unique<Chunk>("IEND");
    } else {
      std::cerr << "unknown chunk type: " << type << std::endl;
      std::cerr << "size: " << size << std::endl;
      chunk = std::make_unique<Chunk>(type);
    }
    std::array<Byte, 4> _crc;
    read(fs, _crc);
    std::array<Byte, 4> expected = crc(view.data());
    if(_crc != expected) {
      std::cerr << "crc mismatched at IEND chunk(expected " << to_str(expected) << ", but got " << to_str(_crc) << ")." << std::endl;
    }
    return chunk;
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
        std::vector<Byte> const& d = ic->data();
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
          int base = h * (width * 3 + 1) + w * 3 + 1;
          p = {data[base], data[base + 1], data[base + 2]};
          switch(data[h * (width * 3 + 1)]) { // filtertype
          case 0: // None
          {
            break;
          }
          case 1:
          {
            if(w != 0) {
              Pixel b = pixels[i - 1];
              p = p + b;
            }
            break;
          }
          case 2:
          {
            if(h != 0) {
              Pixel b = pixels[i - width];
              p = p + b;
            }
            break;
          }
          case 3:
          {
            if(!(w == 0 || h == 0)) {
              Pixel b1 = pixels[i - 1];
              Pixel b2 = pixels[i - width];
              p = p + average(b1, b2);
            }
            break;
          }
          case 4:
          {
            if(!(w == 0 || h == 0)) {
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
              auto pp = [&paethPredictor](Pixel a, Pixel b, Pixel c) {
                Pixel p;
                p.r = paethPredictor(a.r, b.r, c.r);
                p.g = paethPredictor(a.g, b.g, c.g);
                p.b = paethPredictor(a.b, b.b, c.b);
                return p;
              };
              p = p + pp(left, up, naname);
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

  std::unique_ptr<Chunk> makeIHDR(size_t width, size_t height) {
    return std::unique_ptr<Chunk>{new IHDRChunk{width, height, 8, 2, 0, 0, 0}};
  }

  std::pair<int, std::vector<Pixel>> filter(std::vector<Pixel> const& pixels,
                                            std::vector<Pixel>::const_iterator s,
                                            std::vector<Pixel>::const_iterator g,
                                            std::vector<Pixel>::const_iterator pre_s) {
    size_t size = pixels.size();
    std::vector<std::vector<Pixel>> filters(5, std::vector<Pixel>(size));

    // none filter
    std::copy(s, g, begin(filters[0]));

    // sub filter
    {
      auto out = begin(filters[1]);
      Pixel pre{};
      for(auto it{s}; it != g; ++it) {
        auto p = *it;
        if(it == s) {
          pre = *(out++) = p;
        } else {
          auto diff = p - pre;
          *(out++) = diff;
          pre = p;
        }
      }
    }

    // up filter
    {
      auto out = begin(filters[2]);
      for(auto it{s}; it != g; ++it) {
        auto p = *it;
        if(pre_s == end(pixels)) {
          *(out++) = p;
        } else {
          auto diff = p - *(pre_s++);
          *(out++) = diff;
        }
      }
    }

    return std::make_pair(2, filters[2]);
  }

  std::unique_ptr<Chunk> makeIDAT(size_t width, size_t height, std::vector<Pixel> const& pixels) {
    std::vector<Byte> data((width * 3 + 1) * height);
    auto s = begin(pixels);
    auto g = begin(pixels);
    std::advance(g, width);
    decltype(s) pre_s = end(pixels);

    for(size_t i{0}; i < height; ++i) {
      size_t const base{(width * 3 + 1) * i};
      std::pair<int, std::vector<Pixel>> const v = filter(pixels, s, g, pre_s);
      std::vector<Pixel> const& ps = v.second;
      data[base] = v.first;
      for(size_t j{0}; j < width; ++j) {
        Pixel const p = ps[j];
        data[base + 1 + 3 * j] = p.r;
        data[base + 1 + 3 * j + 1] = p.g;
        data[base + 1 + 3 * j + 2] = p.b;
      }
      pre_s = s;
      std::advance(s, width);
      std::advance(g, width);
    }
    return std::unique_ptr<Chunk>{new IDATChunk{data}};
  }

  std::vector<std::unique_ptr<Chunk>> makeChunks(size_t width, size_t height, std::vector<Pixel> const& pixels) {
    std::vector<std::unique_ptr<Chunk>> v;
    v.push_back(makeIHDR(width, height));
    v.push_back(makeIDAT(width, height, pixels));
    v.push_back(std::make_unique<Chunk>("IEND"));
    return v;
  }

  void putSize(std::vector<Byte>& buf, size_t size) {
    buf.push_back((size >> 24) & 0xff);
    buf.push_back((size >> 16) & 0xff);
    buf.push_back((size >>  8) & 0xff);
    buf.push_back((size >>  0) & 0xff);
  }

  void putSize(std::ostream& os, size_t size) {
    os << static_cast<Byte>((size >> 24) & 0xff);
    os << static_cast<Byte>((size >> 16) & 0xff);
    os << static_cast<Byte>((size >>  8) & 0xff);
    os << static_cast<Byte>((size >>  0) & 0xff);
  }

  void putString(std::vector<Byte>& buf, std::string str) {
    for(char c: str) {
      buf.push_back(c);
    }
  }

  void flush(std::ostream& os, std::vector<Byte> const& buf) {
    for(Byte b: buf) {
      os << b;
    }
    for(Byte b: crc(buf)) {
      os << b;
    }
  }

  void putIHDRChunk(std::ostream& os, std::unique_ptr<IHDRChunk> c) {
    std::vector<Byte> buf;
    putSize(os, 13);

    putString(buf, "IHDR");
    putSize(buf, c->width());
    putSize(buf, c->height());
    buf.push_back(c->depth());
    buf.push_back(c->colorType());
    buf.push_back(c->compression());
    buf.push_back(c->filter());
    buf.push_back(c->interlace());
    flush(os, buf);
  }

  void putIDATChunk(std::ostream& os, std::unique_ptr<IDATChunk> c) {
    std::vector<Byte> buf;
    std::vector<Byte> compressed = Deflate::compress(c->data());
    putSize(os, compressed.size());

    putString(buf, "IDAT");
    std::copy(begin(compressed), end(compressed), std::back_inserter(buf));
    flush(os, buf);
  }

  void putIENDChunk(std::ostream& os) {
    putSize(os, 0);
    std::vector<Byte> buf;
    putString(buf, "IEND");
    flush(os, buf);
  }

  void putChunks(std::ostream& os, std::vector<std::unique_ptr<Chunk>>& chunks) {
    for(auto& c: chunks) {
      std::string const& type = c->type();
      if(type == "IHDR") {
        putIHDRChunk(os, dynamic_unique_cast<IHDRChunk>(std::move(c)));
      } else if (type == "IDAT") {
        putIDATChunk(os, dynamic_unique_cast<IDATChunk>(std::move(c)));
      } else if (type == "IEND") {
        putIENDChunk(os);
      } else {
        std::cerr << "unknown chunk type(" << type << ") skipping..." << std::endl;
      }
    }
  }

  std::unique_ptr<Image> exportPNG(std::unique_ptr<Image>&& img, std::ostream& os) {
    for(Byte b: pngSigneture) {
      os << b;
    }
    std::vector<std::unique_ptr<Chunk>> chunks = makeChunks(img->width(), img->height(), img->pixels());
    putChunks(os, chunks);
    return std::move(img);
  }
}

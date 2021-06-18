#include <iostream>
#include <optional>
#include <variant>
#include <vector>
#include <string>

#include "read.h"
#include "jpg.h"

namespace JPG {
  enum class SegmentType {
    SOI,
    EOI,
    APP0,
    APP1,
    SOS,
    RST,
    Unknown,
  };
  char const* to_s(SegmentType t) {
    switch (t) {
    case SegmentType::SOI:
      return "SOI";
    case SegmentType::EOI:
      return "EOI";
    default:
      return "unknown";
    }
  }
  void show(SegmentType t) {
    std::cout << to_s(t) << std::endl;
  }

  template<SegmentType t>
  struct EmptySegment {
    static const SegmentType type = t;
    static const size_t length = 0;
  };
  template<SegmentType t> void show(EmptySegment<t>) { show(t); }
  using SOISegment = EmptySegment<SegmentType::SOI>;
  using EOISegment = EmptySegment<SegmentType::EOI>;

  struct APP0Segment {
    static const SegmentType type = SegmentType::APP0;
    size_t length;
  };
  void show(APP0Segment) { std::cout << "app0" << std::endl; }
  struct APP1Segment {
    static const SegmentType type = SegmentType::APP1;
    size_t length;
  };
  void show(APP1Segment) { std::cout << "app1" << std::endl; }

  struct SOSSegment {
    static const SegmentType type = SegmentType::SOS;
    size_t length;
  };
  void show(SOSSegment) { std::cout << "sos" << std::endl; }

  struct RSTSegment {
    SegmentType type;
    size_t length;
  };
  void show(RSTSegment) { std::cout << "rst" << std::endl; }

  struct UnknownSegment {
    static const SegmentType type = SegmentType::Unknown;
    size_t length;
  };
  void show(UnknownSegment) { std::cout << "unknown" << std::endl; }

  using Segment = std::variant<
    SOISegment,
    EOISegment,
    APP0Segment,
    APP1Segment,
    SOSSegment,
    RSTSegment,
    UnknownSegment
  >;
  SegmentType type(Segment const& s) {
    return std::visit([](auto& e){ return e.type; }, s);
  }
  void show(Segment const& s) {
    return std::visit([](auto& e){ return show(e); }, s);
  }

  struct Jpg {
    std::vector<Segment> segments;
  };
  void show(Jpg const& jpg) {
    std::cout << "parse success!" << std::endl;
    for (auto const& e: jpg.segments) {
      show(e);
    }
  }

  using rawMarker = std::array<unsigned char, 2>;
  void show(rawMarker const& mk) { std::cout << std::hex << static_cast<int>(mk[0]) << ' ' << static_cast<int>(mk[1]) << std::dec << std::endl; }
  std::optional<SegmentType> readMarker(std::istream& fs) {
    auto mk = read<rawMarker>(fs);
    // show(mk);
    if (mk[0] != 0xFF) {
      std::cerr << "not marker!" << std::endl;
      return std::nullopt;
    }
    switch (mk[1]) {
    case 0xd8:
      return SegmentType::SOI;
    case 0xe0:
      return SegmentType::APP0;
    case 0xe1:
      return SegmentType::APP1;
    case 0xda:
      return SegmentType::SOS;
    case 0xd9:
     return SegmentType::EOI;
    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd6:
    case 0xd7:
      return SegmentType::RST;
    }
    return SegmentType::Unknown;
  }

  template<typename T>
  size_t readLength(T& fs) { // , bool endian
    auto len = read<std::array<unsigned char, 2>>(fs);
    return static_cast<size_t>(len[0]) * 256 + len[1];
  }

  std::optional<APP0Segment> readAPP0(std::istream& fs) {
    auto len = readLength(fs);
    // std::cout << "app0 len: " << len << std::endl;
    fs.ignore(len - 2);
    return APP0Segment{len};
  }

  template<typename T>
  size_t readOffset(T& it, bool bigendian) {
    auto len = read<std::array<unsigned char, 4>>(it);
    if (bigendian) {
      return static_cast<size_t>(len[0]) * (1 << 24)
           + static_cast<size_t>(len[1]) * (1 << 16)
           + static_cast<size_t>(len[2]) * (1 <<  8)
           + static_cast<size_t>(len[3]);
    } else {
      return 0; // todo
    }
  }

  enum class TagType : Byte {
    Unknown, // tag type valと一致させる(Byte = 1, Ascii = 2, ...)。
    Byte,
    Ascii,
    Short,
    Long,
    Rational,
    Undefined,
    Slong,
    Srational,
  };

  using TiffValue = std::variant<
    Byte
  >;

  struct TagField {
    uint16_t tag;
    TagType type;
    size_t count;
    size_t offset;
    size_t length() { return 0; } // countとtypeからbyte数を出す。
    TiffValue value() {}
  };

  template<typename T>
  std::optional<TagField> readTagField(T& it) {
    return std::nullopt;
  }

  void readTiff(std::istream& fs, size_t len) {
    std::cout << "readTiff" << std::endl;
    auto tiff = read<std::vector<Byte>>(fs, len);
    auto it = cbegin(tiff);
    auto bom = read<std::array<unsigned char, 2>>(it);
    show(bom);
    bool bigendian = bom[0] == 0x4d;
    auto ver = read<std::array<unsigned char, 2>>(it);
    show(ver);
    auto off = readOffset(it, bigendian);
    std::cout << "0th IFD offset: " << off << "(should be 8)" << std::endl;
    while (true) {
      for (auto tag_cnt = readLength(it); tag_cnt > 0; --tag_cnt) {
        auto field = readTagField(it);
      }
      break;
    }

    std::cout << "end of readTiff" << std::endl;
  }

  std::optional<APP1Segment> readAPP1(std::istream& fs) {
    auto len = readLength(fs);
    auto identifier = read<std::array<char, 6>>(fs);
    // std::cout << "identifier: " << begin(identifier) << std::endl;
    if (std::string{begin(identifier)} == "Exif") {
      readTiff(fs, len - 2 - 6);
    } else fs.ignore(len - 2 - 6);
    return APP1Segment{len};
  }

  std::optional<SOSSegment> readSOS(std::istream& fs) {
    auto len = readLength(fs);
    // std::cout << "SOS len: " << len << std::endl;
    fs.ignore(len - 2);
    std::byte b{};
    size_t cnt{};
    while (true) {
      while (b != std::byte{0xff}) {
        b = read<std::byte>(fs);
        ++cnt;
      }
      if (fs.peek() == 0) {
        b = read<std::byte>(fs);
        ++cnt;
        continue;
      }
      fs.unget();
      --cnt;
      break;
    }
    // std::cout << "SOS: " << cnt << " bytes skipped" << std::endl;
    return SOSSegment{len};
  }

  std::optional<RSTSegment> readRST(std::istream& fs) {
    std::byte b{};
    size_t cnt{};
    while (true) {
      while (b != std::byte{0xff}) {
        b = read<std::byte>(fs);
        ++cnt;
      }
      if (fs.peek() == 0) {
        b = read<std::byte>(fs);
        ++cnt;
        continue;
      }
      fs.unget();
      --cnt;
      break;
    }
    std::cout << "RST: " << cnt << " bytes skipped" << std::endl;
    return RSTSegment{};
  }

  std::optional<Segment> readSegment(std::istream& fs) {
    auto mk = readMarker(fs);
    if (!mk) return std::nullopt;

    switch (*mk) {
    case SegmentType::SOI:
      return SOISegment{};
    case SegmentType::APP0:
      return readAPP0(fs);
    case SegmentType::APP1:
      return readAPP1(fs);
    case SegmentType::SOS:
      return readSOS(fs);
    case SegmentType::RST:
      return readRST(fs);
    case SegmentType::EOI:
      return EOISegment{};
    default:
      auto len = readLength(fs);
      fs.ignore(len - 2);
      return UnknownSegment{len};
    }
  }

  std::optional<Jpg> readJpg(std::istream& fs) {
    Jpg jpg;

    std::optional<Segment> s;
    while(s = readSegment(fs), s && type(*s) != SegmentType::EOI){
      jpg.segments.push_back(*s);
    }
    if (s) jpg.segments.push_back(*s);
    if (type(*s) != SegmentType::EOI) {
      std::cerr << "unexpected segment(maybe broken image)" << std::endl;
    }
    return jpg;
  }

  void showInfo(std::istream& fs) {
    auto jpg = readJpg(fs);
    if(!jpg) {
      std::cout << "not a jpg" << std::endl;
      return;
    }
    show(*jpg);
  }
}

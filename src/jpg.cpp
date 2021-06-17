#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include "read.h"
#include "jpg.h"

namespace JPG {
  enum class SegmentType {
    SOI,
    EOI,
    APP0,
    SOS,
    RST,
    Unknown,
  };

  template<SegmentType t>
  struct EmptySegment {
    static const SegmentType type = t;
    static const size_t length = 0;
  };
  using SOISegment = EmptySegment<SegmentType::SOI>;
  using EOISegment = EmptySegment<SegmentType::EOI>;

  struct APP0Segment {
    static const SegmentType type = SegmentType::APP0;
    size_t length;
  };

  struct SOSSegment {
    static const SegmentType type = SegmentType::SOS;
    size_t length;
  };

  struct RSTSegment {
    SegmentType type;
    size_t length;
  };

  struct UnknownSegment {
    static const SegmentType type = SegmentType::Unknown;
    size_t length;
  };

  using Segment = std::variant<
    SOISegment,
    EOISegment,
    APP0Segment,
    SOSSegment,
    RSTSegment,
    UnknownSegment
  >;
  SegmentType type(Segment const& s) {
    return std::visit([](auto& e){ return e.type; }, s);
  }

  struct Jpg {
    std::vector<Segment> segments;
  };
  void show(Jpg const& jpg) {
    std::cout << "parse success!" << std::endl;
  }

  using rawMarker = std::array<unsigned char, 2>;
  void show(rawMarker const& mk) { std::cout << std::hex << static_cast<int>(mk[0]) << ' ' << static_cast<int>(mk[1]) << std::dec << std::endl; }
  std::optional<SegmentType> readMarker(std::istream& fs) {
    auto mk = read<rawMarker>(fs);
    show(mk);
    if (mk[0] != 0xFF) {
      std::cerr << "not marker!" << std::endl;
      return std::nullopt;
    }
    switch (mk[1]) {
    case 0xd8:
      return SegmentType::SOI;
    case 0xe0:
      return SegmentType::APP0;
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

  size_t readLength(std::istream& fs) {
    auto len = read<std::array<unsigned char, 2>>(fs);
    return static_cast<size_t>(len[0]) * 256 + len[1];
  }

  std::optional<APP0Segment> readAPP0(std::istream& fs) {
    auto len = readLength(fs);
    std::cout << "app0 len: " << len << std::endl;
    fs.ignore(len - 2);
    return APP0Segment{len};
  }

  std::optional<SOSSegment> readSOS(std::istream& fs) {
    auto len = readLength(fs);
    std::cout << "SOS len: " << len << std::endl;
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
    std::cout << "SOS: " << cnt << " bytes skipped" << std::endl;
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
    case SegmentType::SOS:
      return readSOS(fs);
    case SegmentType::RST:
      return readRST(fs);
    case SegmentType::EOI:
      return EOISegment{};
    }
    auto len = readLength(fs);
    fs.ignore(len - 2);
    return UnknownSegment{len};
  }

  std::optional<Jpg> readJpg(std::istream& fs) {
    Jpg jpg;

    std::optional<Segment> s;
    while(s = readSegment(fs), s && type(*s) != SegmentType::EOI){
      jpg.segments.push_back(*s);
    }
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

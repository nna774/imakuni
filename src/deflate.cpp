#include "deflate.h"

#include "miniz.c"

namespace Deflate {
  std::vector<Byte> decompress(std::vector<Byte> src) {
    std::vector<Byte> v;
    unsigned long size = src.size() * 100; //
    Byte *buf = new Byte[size];
    int cmp_status = uncompress(buf, &size, src.data(), src.size());
    if (cmp_status != Z_OK) {
      return v;
    }
    for(unsigned long i{0}; i < size; ++i) {
      v.push_back(buf[i]);
    }
    return v;
  }
}

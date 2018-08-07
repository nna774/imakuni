#include <algorithm>
#include <memory>

#include "deflate.h"

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "miniz.c"

using std::begin;

namespace Deflate {
  std::vector<Byte> compress(std::vector<Byte> const& src) {
    unsigned long size = mz_compressBound(src.size());
    std::unique_ptr<Byte> buf{new Byte[size]};
    int status = mz_compress(buf.get(), &size, src.data(), src.size());
    if (status != MZ_OK) {
      return {};
    }
    std::vector<Byte> v(size);
    std::copy_n(buf.get(), size, begin(v));
    return v;
  }

  std::vector<Byte> decompress(std::vector<Byte> const& src) {
    unsigned long size = src.size() * 100; //
    std::unique_ptr<Byte> buf{new Byte[size]};
    int status = mz_uncompress(buf.get(), &size, src.data(), src.size());
    if (status != MZ_OK) {
      return {};
    }
    std::vector<Byte> v(size);
    std::copy_n(buf.get(), size, begin(v));
    return v;
  }
}

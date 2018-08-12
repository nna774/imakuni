#include <string>
#include <array>
#include <sstream>

#pragma once

template<size_t N>
inline std::string to_str(std::array<Byte, N> const& arr) {
  std::stringstream buf;
  for(size_t i{0}; i < N; ++i) {
    buf << std::hex << static_cast<int>(arr[i]);
  }
  return buf.str();
}

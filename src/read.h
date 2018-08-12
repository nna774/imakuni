#include <istream>
#include <algorithm>

#include "byte.h"
#include "vector_view.h"

#pragma once

template<typename T>
inline void read(std::istream& fs, T& p) {
  fs.read(reinterpret_cast<char*>(&p), sizeof(T));
}
template<>
inline void read(std::istream& fs, std::vector<Byte>& arr) {
  fs.read(reinterpret_cast<char*>(arr.data()), arr.size());
}
template<size_t N>
inline void read(std::istream& fs, std::array<Byte, N>& arr) {
  fs.read(reinterpret_cast<char*>(arr.data()), N);
}

template<typename T>
inline void read(vector_view<Byte>& v, T& p) {
  Byte* pp = reinterpret_cast<Byte*>(&p);
  for(size_t i{0}; i < sizeof(T); ++i) {
    pp[i] = *(v.pos()++);
  }
}
template<>
inline void read(vector_view<Byte>& v, std::vector<Byte>& arr) {
  std::copy_n(v.pos(), arr.size(), begin(arr));
  std::advance(v.pos(), arr.size());
}
template<typename T, size_t N>
inline void read(vector_view<Byte>& v, std::array<T, N>& p) {
  std::copy_n(v.pos(), N, reinterpret_cast<char*>(begin(p)));
  std::advance(v.pos(), N);
}

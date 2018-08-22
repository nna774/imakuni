#include <istream>
#include <algorithm>
#include <vector>

#include "byte.h"

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
inline void read(std::vector<Byte>::iterator& v, T& p) {
  Byte* pp = reinterpret_cast<Byte*>(&p);
  for(size_t i{0}; i < sizeof(T); ++i) {
    pp[i] = *(v++);
  }
}
template<>
inline void read(std::vector<Byte>::iterator& v, std::vector<Byte>& arr) {
  std::copy_n(v, arr.size(), begin(arr));
  std::advance(v, arr.size());
}
template<typename T, size_t N>
inline void read(std::vector<Byte>::iterator& v, std::array<T, N>& p) {
  std::copy_n(v, N, reinterpret_cast<char*>(begin(p)));
  std::advance(v, N);
}

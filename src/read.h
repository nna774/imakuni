#include <istream>
#include <algorithm>
#include <vector>

#include "byte.h"

#pragma once

template<typename T>
inline void read(std::istream& fs, T& p) {
  fs.read(reinterpret_cast<char*>(&p), sizeof(T));
}
template<typename T>
inline void read(std::istream& fs, std::vector<T>& arr) {
  fs.read(reinterpret_cast<char*>(arr.data()), arr.size() * sizeof(T));
}
template<typename T, size_t N>
inline void read(std::istream& fs, std::array<T, N>& arr) {
  fs.read(reinterpret_cast<char*>(begin(arr)), N * sizeof(T));
}

template<typename T>
inline void read(std::vector<Byte>::iterator& v, T& p) {
  Byte* pp = reinterpret_cast<Byte*>(&p);
  for(size_t i{0}; i < sizeof(T); ++i) {
    pp[i] = *(v++);
  }
}
template<typename T>
inline void read(std::vector<Byte>::iterator& v, std::vector<T>& arr) {
  std::copy_n(v, arr.size() * sizeof(T), begin(arr));
  std::advance(v, arr.size() * sizeof(T));
}
template<typename T, size_t N>
inline void read(std::vector<Byte>::iterator& v, std::array<T, N>& p) {
  std::copy_n(v, N * sizeof(T), reinterpret_cast<char*>(begin(p)));
  std::advance(v, N * sizeof(T));
}

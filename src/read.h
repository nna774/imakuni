#include <istream>
#include <algorithm>
#include <vector>

#include "byte.h"

#pragma once

template<typename T>
struct read_ {
  T operator() (std::istream& fs) {
    T t;
    fs.read(reinterpret_cast<char*>(&t), sizeof(T));
    return t;
  }
  T operator() (std::vector<Byte>::const_iterator& it) {
    T t;
    Byte* pp = reinterpret_cast<Byte*>(&t);
    for(size_t i{0}; i < sizeof(T); ++i) {
      pp[i] = *(it++);
    }
    return t;
  }
};
template<typename T>
struct read_<std::vector<T>> {
  std::vector<T> operator() (std::istream& fs, size_t n) {
    std::vector<T> v(n);
    fs.read(reinterpret_cast<char*>(v.data()), n * sizeof(T));
    return v;
  }
  std::vector<T> operator() (std::vector<Byte>::const_iterator& it, size_t n) {
    std::vector<T> v(n);
    std::copy_n(it, n * sizeof(T), begin(v));
    std::advance(it, n * sizeof(T));
    return v;
  }
};
template<typename T, size_t N>
struct read_<std::array<T, N>> {
  std::array<T, N> operator() (std::istream& fs) {
    std::array<T, N> arr;
    fs.read(reinterpret_cast<char*>(begin(arr)), N * sizeof(T));
    return arr;
  }
  std::array<T, N> operator() (std::vector<Byte>::const_iterator& it) {
    std::array<T, N> arr;
    std::copy_n(it, N * sizeof(T), reinterpret_cast<char*>(begin(arr)));
    std::advance(it, N * sizeof(T));
    return arr;
  }
};

template <typename T>
read_<T> read{};

template<typename T>
size_t readSize(T& fs) {
  auto const sizes = read<std::array<Byte, 4>>(fs);
  return (sizes[0] << 24) + (sizes[1] << 16) + (sizes[2] << 8) + sizes[3];
}

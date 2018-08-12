#include <vector>
#include <cassert>

#pragma once

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

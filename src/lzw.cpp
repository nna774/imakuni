#include <algorithm>
#include <variant>
#include <cmath>

#include <iostream>


#include "lzw.h"

using std::begin;

namespace LZW {
  std::vector<Byte> compress(std::vector<Byte> const&) {
    std::vector<Byte> v;
    return v;
  }

  class ClearCode {};
  class EodCode {};
  bool operator==(ClearCode, ClearCode) { return true; }
  bool operator==(EodCode, EodCode) { return true; }
  using Output = std::variant<std::vector<Byte>, ClearCode, EodCode>;

  Output static const clearCode{ClearCode{}};
  Output static const eodCode{EodCode{}};

  void initDict(int clear, std::vector<Output>& dict) {
    dict.clear();
    for(int i{}; i < clear; ++i) {
      // clear <= 256のハズ……？
      dict.push_back(std::vector<Byte>{static_cast<Byte>(i)});
    }
    dict.push_back(clearCode);
    dict.push_back(eodCode);
  }

  struct Pos {
    int byte;
    int bit;
    void advance(size_t size) {
      int tmp = byte * 8 + bit + size;
      byte = tmp / 8;
      bit = tmp % 8;
    }
  };
  int eat(Pos& pos, size_t size, std::vector<Byte> const& src) { // size は高々12。
    int result = src[pos.byte] >> pos.bit;
    size_t eaten = 8 - pos.bit;
    size_t restSize = size - eaten;
    ++pos.byte;
    if(restSize > 8) {
      result += src[pos.byte] << eaten;
      ++pos.byte;
      restSize -= 8;
    }

    Byte mask = 0xff >> (8 - restSize);
    Byte masked = src[pos.byte] & mask;
    result += masked << eaten;
    pos.bit = restSize;

    return result;
  }

  std::vector<Byte> decompress(std::vector<Byte> const& src, size_t size) {
    int const clear = std::pow(2, size);
    std::vector<Output> dict;
    std::vector<Byte> res;
    auto res_tail = back_inserter(res);

    initDict(clear, dict);

    size_t currentSize = size + 1;
    int currentMax = clear * 2;
    std::optional<int> localCode{};
    Pos pos{};
    while(true) {
      if(static_cast<int>(dict.size()) >= currentMax) { // 辞書のサイズは高々2^12。
        ++currentSize;
        currentMax *= 2;
      }
      int n = eat(pos, currentSize, src);
      Output output;
      if(n >= static_cast<int>(dict.size())) { // 辞書のサイズは高々2^12。
        // 入力コードが辞書に無い時は、必ずlocalCodeはvalidである。
        dict.push_back(std::get<std::vector<Byte>>(dict[*localCode]));
        std::get<std::vector<Byte>>(dict.back()).push_back(std::get<std::vector<Byte>>(dict[*localCode])[0]);
      } else {
        output = dict[n];
        if(std::holds_alternative<ClearCode>(output)) {
          initDict(clear, dict);
          std::cout << "got clear code!" << std::endl;
          currentSize = size + 1;
          currentMax = clear * 2;
          localCode = std::nullopt;
          continue;
        } else if(std::holds_alternative<EodCode>(output)) {
          break;
        }
        if(localCode) {
          dict.push_back(dict[*localCode]);
          std::get<std::vector<Byte>>(dict.back()).push_back(std::get<std::vector<Byte>>(dict[*localCode])[0]);
        }
        localCode = n;
      }
      auto out = std::get<std::vector<Byte>>(output);
      copy(begin(out), end(out), res_tail);
    }

    return res;
  }
}

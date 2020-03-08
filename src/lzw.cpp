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
    if(pos.bit + size < 8) {
      int mask = static_cast<int>(std::pow(2, size) - 1) << (pos.bit + 1);
      int result = (src[pos.byte] & mask) >> (pos.bit + 1);
      pos.bit += size;
      return result;
    }
    // 次のwordにまたがる事が確定。
    int eaten = 8 - pos.bit;
    int result = src[pos.byte] >> pos.bit; // 前のword分。
    int restSize = size - eaten;
    if(restSize == 0) {
      pos.bit = 7;
      return result;
    }
    ++pos.byte;
    if(restSize >= 8) {
      // 次のwordはまるまる食べる。
      result += src[pos.byte] << eaten;
      ++pos.byte;
      restSize -= 8;
      eaten += 8;
    }
    // ここでrestSizeは8以下。
    int mask = std::pow(2, restSize) - 1;
    result += (src[pos.byte] & mask) << eaten;
    pos.bit = restSize;
    return result;
  }

  std::vector<Byte> decompress(std::vector<Byte> const& src, size_t size) {
    int const clear = std::pow(2, size);
    std::vector<Output> dict;
    std::vector<Byte> res;

    initDict(clear, dict);

    size_t currentSize = size + 1;
    int currentMax = clear * 2;
    std::optional<int> localCode{};
    Pos pos{};
    bool fullDict{};
    while(true) {
      if(!(static_cast<int>(dict.size()) < 4096)) {
        std::cout << "dict full! expect following clear code!" << std::endl;
        fullDict = true;
      } else if(static_cast<int>(dict.size()) >= currentMax) { // 辞書のサイズは高々2^12。
        ++currentSize;
        currentMax *= 2;
        /*
        std::cout << "sizeup!" << std::endl;
        std::cout << "size: " << currentSize << ", max: " << currentMax << std::endl;
        */
      }
      int n = eat(pos, currentSize, src);
      // std::cout << n << ", ";
      Output output;
      if(n >= static_cast<int>(dict.size()) && !fullDict) { // 辞書のサイズは高々2^12。
        if(n != static_cast<int>(dict.size())) {
          std::cout << "### too large! something went wrong? n: " << n << ", dict size: " << dict.size() << std::endl;
        }
        if(!localCode) { std::cout << "never come!" << std::endl; }
        // 入力コードが辞書に無い時は、必ずlocalCodeはvalidである。
        auto newOne = std::get<std::vector<Byte>>(dict[*localCode]);
        newOne.push_back(newOne[0]);
        dict.push_back(newOne);
        localCode = n;
        copy(begin(newOne), end(newOne), back_inserter(res));
        continue;
      }
      output = dict[n];
      if(std::holds_alternative<ClearCode>(output)) {
        initDict(clear, dict);
        fullDict = false;
        currentSize = size + 1;
        currentMax = clear * 2;
        localCode = std::nullopt;
        // std::cout << "got clear code! size: " << currentSize << ", max: " << currentMax << std::endl;
        continue;
      } else if(std::holds_alternative<EodCode>(output)) {
        /*
        std::cout << "got end code!" << std::endl;
        std::cout << "  dict size: " << dict.size() << std::endl;
        std::cout << "  pos: (" << pos.byte << ", " << pos.bit << ")" << std::endl;
        std::cout << "  src size; " << src.size() << std::endl;
        */
        break;
      }
      auto out = std::get<std::vector<Byte>>(output);
      if(localCode) {
        std::vector<Byte> newOne = std::get<std::vector<Byte>>(dict[*localCode]);
        newOne.push_back(out[0]);
        dict.push_back(newOne);
      }
      localCode = n;
      copy(begin(out), end(out), back_inserter(res));
    }

    return res;
  }
}

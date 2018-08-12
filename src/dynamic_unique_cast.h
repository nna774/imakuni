#include <memory>

#pragma once

template <typename To, typename From>
inline std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& p) {
  if (To* cast = dynamic_cast<To*>(p.get())) {
    std::unique_ptr<To> result(cast);
    p.release();
    return result;
  }
  return std::unique_ptr<To>(nullptr);
}

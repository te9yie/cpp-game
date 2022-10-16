#pragma once
#include <cstddef>
#include <cstdint>

namespace t9 {

// FNV-1a.
inline void fnv1a(std::uint32_t* hash, const void* data, std::size_t n) {
  auto p = static_cast<const std::uint8_t*>(data);
  while (n--) {
    *hash = (*hash ^ *p++) * 16777619;
  }
}
inline std::uint32_t fnv1a(const void* data, std::size_t n) {
  std::uint32_t hash = 2166136261;
  fnv1a(&hash, data, n);
  return hash;
}

}  // namespace t9
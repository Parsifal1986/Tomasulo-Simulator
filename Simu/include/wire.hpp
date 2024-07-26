#ifndef WIRE_HPP
#define WIRE_HPP

#include <cstdio>
#include <cstdint>

namespace parsifal_tools {

class Wire {
private:
  uint32_t wire;

public:
  uint32_t slice(int left, int right) {
    return (wire >> right) & ((1 << (left + 1)) - 1);
  }

  Wire &replace(int left, int right, uint32_t num) {
    int numBits = right - left + 1;
    uint32_t mask = ~(((1 << numBits) - 1) << left);
    wire &= mask;
    num <<= left;
    wire |= num;
    return *this;
  }

  uint32_t Toi() {
    return wire;
  }

  uint32_t replace_tmp(int left, int right, uint32_t num) {
    int numBits = right - left + 1;
    uint32_t mask = ~(((1 << numBits) - 1) << left);
    uint32_t tmp = wire;
    tmp &= mask;
    num <<= left;
    tmp |= num;
    return tmp;
  }

  uint32_t operator[](int a) { return wire & (1 << a); }

  Wire &operator=(const Wire &other) {
    if (this == &other) {
      return *this;
    }
    this->wire = other.wire;
    return *this;
  }

  Wire &operator=(const uint32_t data) {
    this->wire = data;
    return *this;
  }
};

} // namespace parsifal_tools

#endif // WIRE_HPP
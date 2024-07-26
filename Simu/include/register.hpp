#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <cstdint>

namespace parsifal_tools {

class Reg {
private:
  uint32_t reg;
  uint32_t tmp;

public:
  uint32_t slice(int left, int right) {
    return (reg >> right) & ((1 << (left + 1)) - 1);
  }

  Reg &replace(int left, int right, uint32_t num) {
    int numBits = right - left + 1;
    uint32_t mask = ~(((1 << numBits) - 1) << left);
    reg &= mask;
    num <<= left;
    reg |= num;
    return *this;
  }

  uint32_t replace_tmp(int left, int right, uint32_t num) {
    int numBits = right - left + 1;
    uint32_t mask = ~(((1 << numBits) - 1) << left);
    uint32_t tmp = reg;
    tmp &= mask;
    num <<= left;
    tmp |= num;
    return tmp;
  }

  uint32_t operator[](const int num) { return reg & (1 << num); }

  Reg &operator=(const Reg &other) {
    if (this == &other) {
      return *this;
    }
    this->reg = other.reg;
    return *this;
  }

  Reg &operator=(const uint32_t data) {
    this->reg = data;
    return *this;
  }

  void operator<=(const Reg &other) {
    tmp = other.reg;
  }

  void operator<=(const uint32_t data) {
    this->reg = data;
  }

  void Update() {
    reg = tmp;
  }
};

} // namespace parsifal_tools

#endif // REGISTER_HPP
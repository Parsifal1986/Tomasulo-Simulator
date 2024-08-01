#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <cstdint>

namespace parsifal_tools {

class Reg {
private:
  uint32_t reg = 0;
  uint32_t tmp = 0;

public:
  Reg() {

  }

  Reg(const uint32_t data) {
    reg = data;
    tmp = data;
  }

  uint32_t slice(int left, int right) {
    int numBits = left - right + 1;
    return (reg >> right) & ((1 << numBits) - 1);
  }
  
  int32_t slice_with_sign(int left, int right) {
    int numBits = left - right + 1;
    uint32_t mask = (1 << numBits) - 1;
    uint32_t num = (reg >> right) & mask;
    if (num & (1 << (numBits - 1))) {
      num |= ~mask;
    }
    return num;
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

  uint32_t operator[](const int num) { return (reg >> num) & 1; }

  Reg &operator=(const Reg &other) {
    if (this == &other) {
      return *this;
    }
    this->reg = other.reg;
    this->tmp = reg;
    return *this;
  }

  Reg &operator=(const uint32_t data) {
    this->reg = data;
    this->tmp = data;
    return *this;
  }

  void operator<=(const Reg &other) {
    tmp = other.reg;
  }

  void operator<=(const uint32_t data) {
    tmp = data;
  }

  bool operator==(const Reg &other) {
    return this->reg == other.reg;
  }
 
  void Update() {
    reg = tmp;
  }

  uint32_t Toi() const { return reg; }

  friend class RegisterFile;
};

} // namespace parsifal_tools

#endif // REGISTER_HPP
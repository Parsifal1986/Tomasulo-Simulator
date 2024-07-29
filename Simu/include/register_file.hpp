#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include "register.hpp"

namespace parsifal_tools {

class RegisterFile {
public:
  Reg reg[31];
  Reg program_counter;
  Reg state[31];

  void Update() {
    for (int i = 0; i < 32; i++) {
      if (i == 0) {
        reg[i] = 0;
        state[i] = 0;
      }
      reg[i].Update();
      state[i].Update();
    }
  }

  static const uint32_t empty = 0b10000;
};

}

#endif // REGISTER_FILE_HPP
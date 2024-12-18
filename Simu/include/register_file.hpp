#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include "register.hpp"
#include <ostream>

namespace parsifal_tools {

class RegisterFile {
public:
  Reg reg[32];
  Reg program_counter;
  Reg flush;
  Reg state[32];
  Reg jump;
  Reg head_tag;
  Reg terminal = 0;
  Reg need_jump = 0;

  void Update() {
    if (flush.Toi()) {
      for (int i = 0; i < 32; i++) {
        state[i] = 0;
      }
    }
    // if (reg[10].tmp == 255) {
    //   terminal = 1;
    //   return;
    // }
    for (int i = 0; i < 32; i++) {
      if (i == 0) {
        reg[i] = 0;
        state[i] = 0;
      }
      reg[i].Update();
      state[i].Update();
    }
    program_counter.Update();
    flush.Update();
    jump.Update();
    head_tag.Update();
    need_jump.Update();
    terminal.Update();
  }

  static const uint32_t empty = 0b10000;

  friend std::ostream &operator<<(std::ostream &os, const RegisterFile &rf);
};

inline std::ostream &operator<<(std::ostream &os, const RegisterFile &rf) {
  for (int i = 0; i < 32; i++) {
    os << "reg[" << i << "] = " << (int)rf.reg[i].Toi() << std::endl;
  }
  os << "program_counter = " << rf.program_counter.Toi() << std::endl;
  os << "flush = " << rf.flush.Toi() << std::endl;
  os << "jump = " << rf.jump.Toi() << std::endl;
  return os;
}

}

#endif // REGISTER_FILE_HPP
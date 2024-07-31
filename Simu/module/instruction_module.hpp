#ifndef INSTRUCTION_MODULE_HPP
#define INSTRUCTION_MODULE_HPP

#include "BP_module.hpp"
#include "ROB_module.hpp"
#include "register.hpp"
#include "all_tools.hpp"
#include <cstdint>
#include <sys/types.h>

namespace parsifal_modules {

struct InstructionInput {
  shared_ptr<Wire> ready;
  shared_ptr<Wire> addr;
  shared_ptr<Wire> instruction;
  shared_ptr<Wire> jmp_pc;

  InstructionInput &operator=(InstructionInput &other) {
    ready = other.ready;
    addr = other.addr;
    instruction = other.instruction;
    jmp_pc = other.jmp_pc;
    return *this;
  }
};

struct InstructionOutput {
  shared_ptr<Reg> addr;
  shared_ptr<Reg> ready;
  shared_ptr<Reg> program_counter;
  shared_ptr<Reg> instruction;

  InstructionOutput &operator=(InstructionOutput &other) {
    addr = other.addr;
    instruction = other.instruction;
    ready = other.ready;
    program_counter = other.program_counter;
    return *this;
  }

  void Update() {
    addr->Update();
    instruction->Update();
    program_counter->Update();
    ready->Update();
  }
};

class InstructionModule : public Module {
private:
  struct element{
    Reg instruction;
    Reg program_counter;
  };

  InstructionInput input;
  InstructionOutput output;
  Queue<element, 100> queue;
  Reg abandon = 0;

public:
  void Set(InstructionInput set_input, InstructionOutput set_output) {
    input = set_input;
    output = set_output;
  }

  void Flush() {
    if (register_file.flush.Toi()) {
      queue.Clear();
    }
  }

  void InputWork() {
    if (input.instruction->Toi() && !abandon.Toi()) {
      if (input.instruction->Toi() == 0xFD000000) {
        return;
      }
      queue.PushBack(element{Reg(input.instruction->Toi()), Reg(input.addr->Toi())});
      if (input.instruction->slice(6, 4) == 0b110) { // control
        if (input.instruction->slice(6, 0) == 0b1100011) {
          uint32_t offset = ((*input.instruction)[31] << 12) | ((*input.instruction)[7] << 11) | (input.instruction->slice(30, 25) << 5) | (input.instruction->slice(11, 8) << 1);
          register_file.program_counter = ((register_file.jump.Toi() & 0b10) ? input.addr->Toi() + offset : register_file.program_counter.Toi());
          if (register_file.jump.Toi() & 0b01) {
            abandon <= 1;
          }
        } else if (input.instruction->slice(6, 0) == 0b1101111) {
          uint32_t offset = ((*input.instruction)[31] << 20) | (input.instruction->slice(19, 12) << 12) | ((*input.instruction)[20] << 11) | (input.instruction->slice(30, 21) << 1);
          register_file.program_counter = input.addr->Toi() + offset;
          abandon <= 1;
        }
      }
    }
    if (abandon.Toi() == 1) {
      abandon <= 0;
    }
    if (queue.Size() < 98) {
      (*output.addr) <= register_file.program_counter;
      register_file.program_counter <= register_file.program_counter.Toi() + 4;
      (*output.ready) <= 1;
    } else {
      (*output.addr) <= 0;
    }
  }

  void Work() override {
    Flush();
    InputWork();
    QueueWork();
  }

  void QueueWork() {
    if (queue.Empty() || !input.ready) {
      (*output.instruction) <= 0;
      return;
    }
    (*output.instruction) <= queue.Front().instruction.Toi();
    (*output.program_counter) <= queue.Front().program_counter.Toi();
    queue.PopFront();
  }

  void Update() override {
    if (input.jmp_pc->Toi()) {
      register_file.program_counter <= input.jmp_pc->Toi();
    }
    abandon.Update();
    output.Update();
  }

};

}

#endif // INSTRUCTION_MODULE_HPP
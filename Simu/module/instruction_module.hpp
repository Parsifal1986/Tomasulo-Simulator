#ifndef INSTRUCTION_MODULE_HPP
#define INSTRUCTION_MODULE_HPP

#include "ROB_module.hpp"
#include "register.hpp"
#include "tools.hpp"
#include <sys/types.h>

namespace parsifal_modules {

struct InstructionInput {
  shared_ptr<Wire> ready;
  shared_ptr<Wire> addr;
  shared_ptr<Wire> instruction;

  InstructionInput &operator=(InstructionInput &other) {
    ready = other.ready;
    addr = other.addr;
    instruction = other.instruction;
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
  uint32_t queuesize;

public:
  void Set(InstructionInput set_input, InstructionOutput set_output) {
    input = set_input;
    output = set_output;
  }

  void Work() override {
    if (input.instruction->Toi()) {
      queue.PushBack(element{Reg(input.instruction->Toi()), Reg(input.addr->Toi())});
    }
    if (queuesize < 100) {
      (*output.addr) <= register_file.program_counter;
      register_file.program_counter <= register_file.program_counter.Toi() + 4;
      (*output.ready) <= 1;
    } else {
      (*output.addr) <= 0;
    }
    QueueWork();
  }

  void QueueWork() {
    if (queue.Empty() || !input.ready) {
      return;
    }
    (*output.instruction) <= queue.Front().instruction.Toi();
    (*output.program_counter) <= queue.Front().program_counter.Toi();
    queue.PopFront();
  }

  void Update() override {
    output.Update();
  }

};

}

#endif // INSTRUCTION_MODULE_HPP
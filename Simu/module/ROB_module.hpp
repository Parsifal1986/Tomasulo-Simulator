#ifndef ROB_MODULE_HPP
#define ROB_MODULE_HPP

#include "tools.hpp"

extern RegisterFile register_file;

namespace parsifal_modules {

struct RobInsInput {
  shared_ptr<Wire> program_counter;
  shared_ptr<Wire> instruction;

  RobInsInput &operator=(RobInsInput &other) {
    program_counter = other.program_counter;
    instruction = other.instruction;
    return *this;
  }
};

struct RobMemInput {

};

struct RobCdbInput {

};

struct RobBpInput {

};

struct RobBpOutput {

};

struct RobRsInput {
  shared_ptr<Reg> ls_ready;
  shared_ptr<Reg> alu_ready;

  RobRsInput &operator=(RobRsInput &other) {
    ls_ready = other.ls_ready;
    alu_ready = other.alu_ready;
    return *this;
  }
};

struct RobInsOutput {
  shared_ptr<Reg> ready;

  void Update() {
    ready->Update();
  }

  RobInsOutput &operator=(RobInsOutput &other) {
    ready = other.ready;
    return *this;
  }
};

struct RobRsOutput {
  shared_ptr<Reg> tag;
  shared_ptr<Reg> instruction;

  void Update() {
    tag->Update();
    instruction->Update();
  }

  RobRsOutput &operator=(RobRsOutput &other) {
    tag = other.tag;
    instruction = other.instruction;
    return *this;
  }
};

class RobModule : public Module {
private:
RobInsInput instruction_input;
RobRsInput rs_input;
RobInsOutput instruction_output;
RobRsOutput rs_output;
struct element {
  Reg busy;
  Reg instruction;
  Reg state;
  Reg destination;
  Reg value;
  Reg pc;
};
Queue<element, 16> queue;
int queuesize;
enum InstructionType {
  AR = 0b0110011,
  AI = 0b0010011,
  LD = 0b0000011,
  ST = 0b0100011,
  BR = 0b1100011,
  JP = 0b1101111,
  JPR = 0b1100111
};

public:
  void Set(RobInsInput set_ins_input, RobRsInput set_rs_input, RobInsOutput set_ins_output, RobRsOutput set_rs_output) {
    instruction_input = set_ins_input;
    instruction_output = set_ins_output;

    rs_input = set_rs_input;
    rs_output = set_rs_output;
  }

  void WorkQueue() {
    if (instruction_input.instruction->Toi()) {
      element a;
      a.busy = 1;
      a.instruction = instruction_input.instruction->Toi();
      a.state = 0;
      a.destination = instruction_input.instruction->slice(11, 7);
      a.value = 0;
      a.pc = instruction_input.program_counter->Toi();
    }
    
    if (queuesize >= 16) {
      (*instruction_output.ready) <= 0;
    } else {
      (*instruction_output.ready) <= 1;
      ++queuesize;
    }

    if (queue.Front().state.Toi() == 0b10) {
      if (queue.Front().instruction.slice(6, 0) == JPR) {
        // BP
      } else if (queue.Front().instruction.slice(6, 0) == BR) {
        if (queue.Front().value.Toi()) {
          // BP
        }
      }
      register_file.reg[queue.Front().destination.Toi()] <= queue.Front().value;
      queue.Front().state = 0b11;
      queue.Front().busy = 0;
      if (register_file.state[queue.Front().destination.Toi()].Toi() == queue.begin().num()) {
        register_file.state[queue.Front().destination.Toi()] <= RegisterFile::empty;
      }
      queue.PopFront();
      queuesize--;
    }
  }

  void WorkToRs() {
    Reg book;
    for (auto it = queue.begin(); it != queue.end(); ++it) {
      if (book.Toi()) {
        break;
      }
      if ((*it).busy.Toi() == 1 && (*it).state.Toi() == 0) {
        switch (instruction_input.instruction->slice(6, 0)) {
          case LD:
          case ST: {
            if (rs_input.ls_ready->Toi()) {
              (*rs_output.tag) <= it.num();
              (*rs_output.instruction) <= (*it).instruction;
              book = 1;
            }
            break;
          }
          case AI:
          case AR:
          case BR:
          case JPR: {
            if (rs_input.alu_ready->Toi()) {
              (*rs_output.tag) <= it.num();
              (*rs_output.instruction) <= (*it).instruction;
              book = 1;
            }
            break;
          }
          case JP: {
            (*it).state = 0b10;
            (*it).value = (*it).pc.Toi() + 4;
            break;
          }
          default: {
            
            break;
          }
        }  
      }
    }

  }

  void Work() override {
    WorkQueue();
    WorkToRs();
  }

  void Update() override {
    instruction_output.Update();
    rs_output.Update();
  }
};

}

#endif // ROB_MODULE_HPP
#ifndef  ALU_HPP
#define ALU_HPP

#include "tools.hpp"
#include <cstdint>
#include <iostream>
#include <sys/types.h>

namespace parsifal_modules {

struct AluInput {
  shared_ptr<Wire> a;
  shared_ptr<Wire> b;
  shared_ptr<Wire> operand;

  AluInput &operator=(const AluInput &other) {
    a = other.a;
    b = other.b;
    operand = other.operand;
    return *this;
  }
};

struct AluOutput {
  shared_ptr<Reg> result;
  shared_ptr<Reg> done;

  void Update() {
    done->Update();
    result->Update();
  }

  AluOutput &operator=(const AluOutput &other) {
    result = other.result;
    done = other.done;
    return *this;
  }
};

class AluModule : public Module {
private:
  enum AluOperand {
    ADD,
    SUB,
    AND,
    OR,
    XOR,
    SLL,
    SRL,
    SRA,
    SLT,
    SLTU,
    BEQ,
    BNE,
    BGE,
    BGEU,
    BLT,
    BLTU
  };
  AluInput input;
  AluOutput output;

public:
  void Set(AluInput alu_input, AluOutput alu_output) {
    input = alu_input;
    output = alu_output;
  }

  void Work() override {
    if ((*input.operand)[31] == 0) {
      switch (input.operand->Toi()) {
        case ADD: {
          (*output.result) <= input.a->Toi() + input.b->Toi();
          break;
        }
        case SUB: {
          (*output.result) <= input.a->Toi() - input.b->Toi();
          break;
        }
        case AND: {
          (*output.result) <= (input.a->Toi() & input.b->Toi());
          break;
        }
        case OR: {
          (*output.result) <= (input.a->Toi() | input.b->Toi());
          break;
        }
        case XOR: {
          (*output.result) <= (input.a->Toi() ^ input.b->Toi());
          break;
        }
        case SLL: {
          (*output.result) <= (input.a->Toi() << input.b->Toi());
          break;
        }
        case SRL: {
          (*output.result) <= (input.a->Toi() >> input.b->Toi());
          break;
        }
        case SRA: {
          (*output.result) <= (uint32_t)(((int)input.a->Toi()) >> input.b->Toi());
          break;
        }
        case SLT: {
          (*output.result) <= ((int)input.a->Toi() < (int)input.b->Toi());
          break;
        }
        case SLTU: {
          (*output.result) <= (input.a->Toi() < input.b->Toi());
          break;
        }
        case BEQ: {
          (*output.result) <= (input.a->Toi() == input.b->Toi());
          break;
        }
        case BNE: {
          (*output.result) <= (input.a->Toi() != input.b->Toi());
          break;
        }
        case BGE: {
          (*output.result) <= ((int)input.a->Toi() >= (int)input.b->Toi());
          break;
        }
        case BGEU: {
          (*output.result) <= (input.a->Toi() >= input.b->Toi());
          break;
        }
        case BLT: {
          (*output.result) <= ((int)input.a->Toi() < (int)input.b->Toi());
          break;
        }
        case BLTU: {
          (*output.result) <= (input.a->Toi() < input.b->Toi());
          break;
        }
        default: {
          (*output.result) <= 0xffff;
          std::cerr << "Wrong Opecode!" << std::endl;
          break;
        }
      }
      (*output.done) <= 1;
    } else {
      (*output.done) <= 0;
    }
  }

  void Update() override {
    output.Update();
  }
};

}

#endif // ALU_HPP
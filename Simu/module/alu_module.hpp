#ifndef  ALU_HPP
#define ALU_HPP

#include "all_tools.hpp"
#include <cstdint>
#include <iostream>
#include <sys/types.h>

namespace parsifal_modules {

struct AluInput {
  shared_ptr<Wire> a;
  shared_ptr<Wire> b;
  shared_ptr<Wire> operand;
  shared_ptr<Wire> tag;

  AluInput &operator=(const AluInput &other) {
    a = other.a;
    b = other.b;
    operand = other.operand;
    tag = other.tag;
    return *this;
  }
};

struct AluOutput {
  shared_ptr<CDB> cdb;

  void Update() {
    cdb->Update();
  }

  AluOutput &operator=(const AluOutput &other) {
    cdb = other.cdb;
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
    if (input.tag->Toi()) {
      switch (input.operand->Toi()) {
        case ADD: {
          output.cdb->alu_data <= input.a->Toi() + input.b->Toi();
          break;
        }
        case SUB: {
          output.cdb->alu_data <= input.a->Toi() - input.b->Toi();
          break;
        }
        case AND: {
          output.cdb->alu_data <= (input.a->Toi() & input.b->Toi());
          break;
        }
        case OR: {
          output.cdb->alu_data <= (input.a->Toi() | input.b->Toi());
          break;
        }
        case XOR: {
          output.cdb->alu_data <= (input.a->Toi() ^ input.b->Toi());
          break;
        }
        case SLL: {
          output.cdb->alu_data <= (input.a->Toi() << input.b->Toi());
          break;
        }
        case SRL: {
          output.cdb->alu_data <= (input.a->Toi() >> input.b->Toi());
          break;
        }
        case SRA: {
          output.cdb->alu_data <= (uint32_t)(((int)input.a->Toi()) >> input.b->Toi());
          break;
        }
        case SLT: {
          output.cdb->alu_data <= ((int)input.a->Toi() < (int)input.b->Toi());
          break;
        }
        case SLTU: {
          output.cdb->alu_data <= (input.a->Toi() < input.b->Toi());
          break;
        }
        case BEQ: {
          output.cdb->alu_data <= (input.a->Toi() == input.b->Toi());
          break;
        }
        case BNE: {
          output.cdb->alu_data <= (input.a->Toi() != input.b->Toi());
          break;
        }
        case BGE: {
          output.cdb->alu_data <= ((int)input.a->Toi() >= (int)input.b->Toi());
          break;
        }
        case BGEU: {
          output.cdb->alu_data <= (input.a->Toi() >= input.b->Toi());
          break;
        }
        case BLT: {
          output.cdb->alu_data <= ((int)input.a->Toi() < (int)input.b->Toi());
          break;
        }
        case BLTU: {
          output.cdb->alu_data <= (input.a->Toi() < input.b->Toi());
          break;
        }
        default: {
          output.cdb->alu_data <= 0xffff;
          std::cerr << "Wrong Opecode!" << std::endl;
          break;
        }
      }
      output.cdb->alu_done <= 1;
      output.cdb->alu_tag <= input.tag->Toi();
    } else {
      output.cdb->alu_done <= 0;
    }
  }

  void Update() override {
    output.Update();
  }
};

}

#endif // ALU_HPP
#ifndef RS_MODULE_HPP
#define RS_MODULE_HPP

#include "ROB_module.hpp"
#include "tools.hpp"
#include <cstdint>
#include <sys/types.h>

namespace parsifal_modules {

struct RsInput {
  shared_ptr<Wire> instruction;
  shared_ptr<Wire> tag;
  shared_ptr<CDB> cdb;
};

struct RsAluOutput {
  shared_ptr<Reg> operand;
  shared_ptr<Reg> a;
  shared_ptr<Reg> b;

  void Update() {
    a->Update();
    b->Update();
    operand->Update();
  }
};

class RsModule : public Module {
private:
  struct element {
    Reg busy;
    Reg op;
    Reg vj;
    Reg vk;
    Reg qj;
    Reg qk;
    Reg a;
    Reg dest;
  };
  element alu_array[16];
  uint32_t alu_array_size = 0;
  element ls_array[16];
  uint32_t ls_array_size = 0;
  RsInput rs_input;
  RsAluOutput rs_alu_output;
  enum InstructionType {
    AR = 0b0110011,
    AI = 0b0010011,
    LD = 0b0000011,
    ST = 0b0100011,
    BR = 0b1100011,
    JPR = 0b1100111
  };
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

public:
  void WorkArray() {
    uint32_t opcode = rs_input.instruction->slice(6, 0);
    uint32_t func3 = rs_input.instruction->slice(14, 12);
    if (rs_input.instruction->Toi()) {
      if (opcode == ST || opcode == ST) {

      } else {
        for (int i = 0; i < 16; i++) {
          if (alu_array[i].busy.Toi() == 0) {
            ++alu_array_size;
            alu_array[i].busy = 1;
            alu_array[i].dest = rs_input.tag->Toi();
            register_file.reg[alu_array[i].dest.Toi()] = i;
            switch (opcode) {
              case AR: {
                uint32_t func7 = rs_input.instruction->slice(31, 25);
                switch (func3) {
                  case 0b000: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = ADD;
                    } else if (func7 == 0b0100000) {
                      alu_array[i].op = SUB;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b111: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = AND;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b110: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = OR;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b100: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = XOR;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b001: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = SLL;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b101: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = SRL;
                    } else if (func7 == 0b0100000) {
                      alu_array[i].op = SRA;
                    }else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b010: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = SLT;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                  case 0b011: {
                    if (func7 == 0b0000000) {
                      alu_array[i].op = SLTU;
                    } else {
                      std::cerr << "Wrong func7(RS parser)" << std::endl;
                    }
                    break;
                  }
                }
                uint32_t rs1 = rs_input.instruction->slice(19, 15);
                uint32_t rs2 = rs_input.instruction->slice(24, 20);
                if (register_file.state[rs1].Toi()) {
                  alu_array[i].qj = register_file.state[rs1].Toi();
                } else {
                  alu_array[i].vj = register_file.reg[rs1];
                }
                if (register_file.state[rs2].Toi()) {
                  alu_array[i].qk = register_file.state[rs2].Toi();
                } else {
                  alu_array[i].qj = register_file.reg[rs2];
                }
                break;
              }
              case AI: {
                switch (func3) {
                  case 0b000: {
                    alu_array[i].op = ADD;
                    break;
                  }
                  case 0b111: {
                    alu_array[i].op = AND;
                    break;
                  }
                  case 0b110: {
                    alu_array[i].op = OR;
                    break;
                  }
                  case 0b100: {
                    alu_array[i].op = XOR;
                    break;
                  }
                  case 0b001: {
                    alu_array[i].op = SLL;
                    break;
                  }
                  case 0b101: {
                    alu_array[i].op = SRL;
                    break;
                  }
                  case 0b010: {
                    alu_array[i].op = SLT;
                    break;
                  }
                  case 0b011: {
                    alu_array[i].op = SLTU;
                    break;
                  }
                }
                uint32_t rs1 = rs_input.instruction->slice(19, 15);
                uint32_t imm;
                if (alu_array[i].op.Toi() == SRL || alu_array[i].op.Toi() == SLL) {
                  uint32_t func7 = rs_input.instruction->slice(31, 25);
                  if (func7) {
                    alu_array[i].op = SRA;
                  }
                  imm = rs_input.instruction->slice(24, 20);
                } else {
                  imm = rs_input.instruction->slice(31, 20);
                }
                if (register_file.state[rs1].Toi()) {
                  alu_array[i].qj = register_file.state[rs1].Toi();
                } else {
                  alu_array[i].vj = register_file.reg[rs1];
                }
                alu_array[i].qk = imm;
                break;
              }
              case BR: {
                switch (rs_input.instruction->slice(14, 12)) {
                  case 0b000: {
                    alu_array[i].op = BEQ;
                    break;
                  }
                  case 0b101: {
                    alu_array[i].op = BGE;
                    break;
                  }
                  case 0b111: {
                    alu_array[i].op = BGEU;
                    break;
                  }
                  case 0b100: {
                    alu_array[i].op = BLT;
                    break;
                  }
                  case 0b110: {
                    alu_array[i].op = BLTU;
                    break;
                  }
                  case 0b001: {
                    alu_array[i].op = BNE;
                    break;
                  }
                }
                uint32_t rs1 = rs_input.instruction->slice(19, 15);
                uint32_t rs2 = rs_input.instruction->slice(24, 20);
                if (register_file.state[rs1].Toi()) {
                  alu_array[i].qj = register_file.state[rs1].Toi();
                } else {
                  alu_array[i].vj = register_file.reg[rs1];
                }
                if (register_file.state[rs2].Toi()) {
                  alu_array[i].qk = register_file.state[rs2].Toi();
                } else {
                  alu_array[i].qj = register_file.reg[rs2];
                }
                break;
              }
              case JPR: {
                alu_array[i].op = ADD;
                uint32_t rs1 = rs_input.instruction->slice(19, 15);
                uint32_t imm;
                imm = rs_input.instruction->slice(31, 20);
                if (register_file.state[rs1].Toi()) {
                  alu_array[i].qj = register_file.state[rs1].Toi();
                } else {
                  alu_array[i].vj = register_file.reg[rs1];
                }
                alu_array[i].qk = imm;
                break;
              }
            }
          }
        }
      }
    }
  }

  void Work() override {
    WorkArray();
  }

  void Update() override {
    rs_alu_output.Update();
  }

};

}

#endif // RS_MODULE_HP
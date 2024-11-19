#ifndef ROB_MODULE_HPP
#define ROB_MODULE_HPP

#include "LSB_module.hpp"
#include "RS_module.hpp"
#include "all_tools.hpp"
#include "register.hpp"
#include <cstdint>
#include <sys/types.h>

extern RegisterFile register_file;

namespace parsifal_modules {

struct RobInsInput {
  shared_ptr<Wire> program_counter;
  shared_ptr<Wire> instruction;

  RobInsInput &operator=(const RobInsInput &other) {
    program_counter = other.program_counter;
    instruction = other.instruction;
    return *this;
  }
};

struct RobCdbInput {
  shared_ptr<CDB> cdb;

  void Update() {
    cdb->Update();
  }

  RobCdbInput &operator=(const RobCdbInput &other) {
    cdb = other.cdb;
    return *this;
  }
};

struct RobBpInput {
  shared_ptr<Reg> tag;

  RobBpInput &operator=(const RobBpInput &other) {
    tag = other.tag;
    return *this;
  }

  void Update() {
    tag->Update();
  }
};

struct RobBpOutput {
  shared_ptr<Reg> tag;
  shared_ptr<Reg> jp;

  void Update() {
    tag->Update();
    jp->Update();
  }

  RobBpOutput &operator=(const RobBpOutput &other) {
    tag = other.tag;
    jp = other.jp;
    return *this;
  }
};

struct RobRsInput {
  shared_ptr<Reg> alu_ready;

  RobRsInput &operator=(const RobRsInput &other) {
    alu_ready = other.alu_ready;
    return *this;
  }
};

struct RobLsbInput {
  shared_ptr<Reg> ls_ready;

  RobLsbInput &operator=(const RobLsbInput &other) {
    ls_ready = other.ls_ready;
    return *this;
  }
};

struct RobInsOutput {
  shared_ptr<Reg> ready;
  shared_ptr<Reg> jmp_pc;

  void Update() {
    ready->Update();
    jmp_pc->Update();
  }

  RobInsOutput &operator=(const RobInsOutput &other) {
    ready = other.ready;
    jmp_pc = other.jmp_pc;
    return *this;
  }
};

struct RobRsOutput {
  shared_ptr<Reg> tag;
  shared_ptr<RsElement> instruction;

  void Update() {
    tag->Update();
    instruction->Update();
  }

  RobRsOutput &operator=(const RobRsOutput &other) {
    tag = other.tag;
    instruction = other.instruction;
    return *this;
  }
};

struct RobLsbOutput {
  shared_ptr<LsbElement> instruction;

  void Update() {
    instruction->Update();
  }

  RobLsbOutput &operator=(const RobLsbOutput &other) {
    instruction = other.instruction;
    return *this;
  }
};

class RobModule : public Module {
private:
RobInsInput instruction_input;
RobRsInput rs_input;
RobLsbInput lsb_input;
RobInsOutput instruction_output;
RobRsOutput rs_output;
RobLsbOutput lsb_output;
RobCdbInput cdb_input;
RobBpInput bp_input;
RobBpOutput bp_output;

u_int32_t wait = 0;
u_int32_t stop = 0;

struct element {
  Reg busy;
  Reg instruction;
  Reg state;
  Reg destination;
  Reg value;
  Reg pc;
  Reg flush = 0;
};
Queue<element, 64> queue;
enum InstructionType {
  AR = 0b0110011,
  AI = 0b0010011,
  LD = 0b0000011,
  ST = 0b0100011,
  BR = 0b1100011,
  JP = 0b1101111,
  JPR = 0b1100111,
  LUI = 0b0110111,
  AUIPC = 0b0010111
};
enum Operand {
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
  BLTU,
  LDI,
  STI
};

public:
  void Set(RobInsInput set_ins_input, RobRsInput set_rs_input, RobLsbInput set_lsb_input, RobBpInput set_bp_input, RobCdbInput set_cdb_input, RobInsOutput set_ins_output, RobRsOutput set_rs_output, RobLsbOutput set_lsb_output, RobBpOutput set_bp_output) {
    instruction_input = set_ins_input;
    instruction_output = set_ins_output;
    bp_input = set_bp_input;
    lsb_input = set_lsb_input;

    cdb_input = set_cdb_input;

    rs_input = set_rs_input;
    rs_output = set_rs_output;
    bp_output = set_bp_output;
    lsb_output = set_lsb_output;
  }

  void WorkBp() {
    if (bp_input.tag->Toi()) {
      queue.At(bp_input.tag->Toi() - 1).flush = 1;
    }
  }

  void WorkFlush() {
    if (register_file.flush.Toi()) {
      queue.Clear(); 
      (*bp_output.tag) <= 0;
      (lsb_output.instruction->tag <= 0);
      (*rs_output.tag) <= 0;
      stop = 0;
    }
  }

  void WorkEnQueue() {
    if (instruction_input.instruction->Toi() && !stop) {
      element a;
      a.busy = 1;
      a.instruction = instruction_input.instruction->Toi();
      a.state = 0;
      a.destination = instruction_input.instruction->slice(11, 7);
      a.value = 0;
      a.pc = instruction_input.program_counter->Toi();
      queue.PushBack(a);
    }
    
    if (queue.Size() < 62) {
      (*instruction_output.ready) <= 1;
    } else {
      (*instruction_output.ready) <= 0;
    }
  }

  void WorkDeQueue() {
    register_file.flush <= 0;
    register_file.head_tag <= queue.begin().num() + 1;
    register_file.need_jump <= 0;
    if (queue.Size() && queue.Front().state.Toi() == 0b10) {
      if (queue.Front().instruction.Toi() == 0x0ff00513) {
        register_file.terminal <= 1;
        return;
      }
      if (queue.Front().instruction.slice(6, 4) == 0b110) {
        if (queue.Front().instruction.slice(6, 0) == JPR || queue.Front().instruction.slice(6, 0) == JP) {
          register_file.reg[queue.Front().destination.Toi()] <= queue.Front().value;
          if (register_file.state[queue.Front().destination.Toi()].Toi() - 1 == queue.begin().num()) {
            register_file.state[queue.Front().destination.Toi()] = 0;
          }
        }
        if (queue.Front().instruction.slice(6, 0) == JPR) {
          stop = 0;
          (*instruction_output.jmp_pc) <= queue.Front().value;
          register_file.need_jump <= 1;
          register_file.flush <= 1;
        } else if (queue.Front().instruction.slice(6, 0) == BR) {
          if (!wait) {
            wait = 1;
            return;
          } else {
            wait = 0;
          }
          if (queue.Front().flush.Toi()) {
            register_file.flush <= 1;
            if (queue.Front().value.Toi()) {
              uint32_t offset = ((queue.Front().instruction[31] ? 0xfffff000 : 0) | (queue.Front().instruction[7] << 11) | (queue.Front().instruction.slice(30, 25) << 5) | (queue.Front().instruction.slice(11, 8) << 1));
              (*instruction_output.jmp_pc) <= queue.Front().pc.Toi() + offset;
              register_file.need_jump <= 1;
            } else {
              (*instruction_output.jmp_pc) <= queue.Front().pc.Toi() + 4;
              register_file.need_jump <= 1;
            }
          }
        }
      } else if (queue.Front().instruction.slice(6, 0) == 0b0100011) {
      } else {
        register_file.reg[queue.Front().destination.Toi()] <= queue.Front().value;
        if (register_file.state[queue.Front().destination.Toi()].Toi() - 1 == queue.begin().num()) {
          register_file.state[queue.Front().destination.Toi()] = 0;
        }
      }
      // std::cout << "Commit: " << queue.Front().instruction.Toi() << " , PC" << std::hex << queue.Front().pc.Toi() << std::endl << register_file << std::endl;
      queue.Front().state = 0b11;
      queue.Front().busy = 0;
      queue.PopFront();
    }
  }

  void Decoder(Reg instruction, Reg tag) {
    uint32_t opcode = instruction.slice(6, 0);
    uint32_t func3 = instruction.slice(14, 12);
    if (opcode == ST || opcode == LD) {
      LsbElement e;
      e.tag = tag;
      switch (opcode) {
        case LD: {
          e.oprand = instruction.slice(14, 12);
          if (register_file.state[instruction.slice(19, 15)].Toi()) {
            uint32_t des = register_file.state[instruction.slice(19, 15)].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.addr = queue.At(des).value;
            } else {
              e.rd = register_file.state[instruction.slice(19, 15)].Toi();
            }
          } else {
            e.addr = register_file.reg[instruction.slice(19, 15)];
          }
          e.imm = instruction.slice_with_sign(31, 20);
          break;
        }
        case ST: {
          e.oprand = (1 << 31) + instruction.slice(14, 12);
          if (register_file.state[instruction.slice(19, 15)].Toi()) {
            uint32_t des = register_file.state[instruction.slice(19, 15)].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.addr = queue.At(des).value;
            } else {
              e.rd = register_file.state[instruction.slice(19, 15)].Toi();
            }
          } else {
            e.addr = register_file.reg[instruction.slice(19, 15)];
          }
          if (register_file.state[instruction.slice(24, 20)].Toi()) {
            uint32_t des = register_file.state[instruction.slice(24, 20)].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.data = queue.At(des).value;
            } else {
              e.rs1 = register_file.state[instruction.slice(24, 20)].Toi();
            }
          } else {
            e.data = register_file.reg[instruction.slice(24, 20)].Toi();
          }
          e.imm = instruction[31] ? 0xfffff000 | (instruction.slice(31, 25) << 5) | instruction.slice(11, 7) : (instruction.slice(31, 25) << 5) | instruction.slice(11, 7);
          break;
        }
      }
      (*lsb_output.instruction) <= e;
    } else {
      RsElement e;
      e.busy = 1;
      switch (opcode) {
        case AR: {
          uint32_t func7 = instruction.slice(31, 25);
          switch (func3) {
          case 0b000: {
            if (func7 == 0b0000000) {
              e.op = ADD;
            } else if (func7 == 0b0100000) {
              e.op = SUB;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b111: {
            if (func7 == 0b0000000) {
              e.op = AND;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b110: {
            if (func7 == 0b0000000) {
              e.op = OR;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b100: {
            if (func7 == 0b0000000) {
              e.op = XOR;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b001: {
            if (func7 == 0b0000000) {
              e.op = SLL;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b101: {
            if (func7 == 0b0000000) {
              e.op = SRL;
            } else if (func7 == 0b0100000) {
              e.op = SRA;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b010: {
            if (func7 == 0b0000000) {
              e.op = SLT;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          case 0b011: {
            if (func7 == 0b0000000) {
              e.op = SLTU;
            } else {
              std::cerr << "Wrong func7(RS parser)" << std::endl;
            }
            break;
          }
          }
          uint32_t rs1 = instruction.slice(19, 15);
          uint32_t rs2 = instruction.slice(24, 20);
          if (register_file.state[rs1].Toi()) {
            uint32_t des = register_file.state[rs1].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vj = queue.At(des).value;
            } else {
              e.qj = register_file.state[rs1].Toi();
            }
          } else {
            e.vj = register_file.reg[rs1];
          }
          if (register_file.state[rs2].Toi()) {
            uint32_t des = register_file.state[rs2].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vk = queue.At(des).value;
            } else {
              e.qk = register_file.state[rs2].Toi();
            }
          } else {
            e.vk = register_file.reg[rs2];
          }
          break;
        }
        case AI: {
          switch (func3) {
            case 0b000: {
              e.op = ADD;
              break;
            }
            case 0b111: {
              e.op = AND;
              break;
            }
            case 0b110: {
              e.op = OR;
              break;
            }
            case 0b100: {
              e.op = XOR;
              break;
            }
            case 0b001: {
              e.op = SLL;
              break;
            }
            case 0b101: {
              e.op = SRL;
              break;
            }
            case 0b010: {
              e.op = SLT;
              break;
            }
            case 0b011: {
              e.op = SLTU;
              break;
            }
          }
          uint32_t rs1 = instruction.slice(19, 15);
          uint32_t imm;
          if (e.op.Toi() == SRL || e.op.Toi() == SLL) {
            uint32_t func7 = instruction.slice(31, 25);
            if (func7) {
              e.op = SRA;
            }
            imm = instruction.slice(24, 20);
          } else {
            imm = instruction.slice_with_sign(31, 20);
          }
          if (register_file.state[rs1].Toi()) {
            uint32_t des = register_file.state[rs1].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vj = queue.At(des).value;
            } else {
              e.qj = register_file.state[rs1].Toi();
            }
          } else {
            e.vj = register_file.reg[rs1];
          }
          e.vk = imm;
          break;
        }
        case BR: {
          switch (instruction.slice(14, 12)) {
          case 0b000: {
            e.op = BEQ;
            break;
          }
          case 0b101: {
            e.op = BGE;
            break;
          }
          case 0b111: {
            e.op = BGEU;
            break;
          }
          case 0b100: {
            e.op = BLT;
            break;
          }
          case 0b110: {
            e.op = BLTU;
            break;
          }
          case 0b001: {
            e.op = BNE;
            break;
          }
          }
          uint32_t rs1 = instruction.slice(19, 15);
          uint32_t rs2 = instruction.slice(24, 20);
          if (register_file.state[rs1].Toi()) {
            uint32_t des = register_file.state[rs1].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vj = queue.At(des).value;
            } else {
              e.qj = register_file.state[rs1].Toi();
            }
          } else {
            e.vj = register_file.reg[rs1];
          }
          if (register_file.state[rs2].Toi()) {
            uint32_t des = register_file.state[rs2].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vk = queue.At(des).value;
            } else {
              e.qk = register_file.state[rs2].Toi();
            }
          } else {
            e.vk = register_file.reg[rs2];
          }
          break;
        }
        case JPR: {
          e.op = ADD;
          uint32_t rs1 = instruction.slice(19, 15);
          uint32_t imm;
          imm = instruction.slice(31, 20);
          if (register_file.state[rs1].Toi()) {
            uint32_t des = register_file.state[rs1].Toi() - 1;
            if (queue.At(des).state == 0b10) {
              e.vj = queue.At(des).value;
            } else {
              e.qj = register_file.state[rs1].Toi();
            }
          } else {
            e.vj = register_file.reg[rs1];
          }
          e.vk = imm;
          break;
        }
      }
      e.dest = tag.Toi();
      (*rs_output.instruction) <= e;
    }
  }

  void WorkDecode() {
    Reg book;
    (*rs_output.tag) <= 0;
    lsb_output.instruction->tag <= 0;
    (*bp_output.tag) <= 0;
    for (auto it = queue.begin(); it != queue.end(); ++it) {
      if (book.Toi()) {
        break;
      }
      if ((*it).busy.Toi() == 1 && (*it).state.Toi() == 0) {
        switch ((*it).instruction.slice(6, 0)) {
          case LD:
          case ST: {
            if (lsb_input.ls_ready->Toi()) {
              Decoder((*it).instruction, it.num() + 1);
              (*it).state = 0b01;
              book = 1;
            }
            break;
          }
          case AI:
          case AR:
          case BR:
          case JPR: {
            if (rs_input.alu_ready->Toi()) {
              (*rs_output.tag) <= it.num() + 1;
              Decoder((*it).instruction, it.num() + 1);
              (*it).state = 0b01;
              book = 1;
            }
            if (instruction_input.instruction->slice(6, 0) == JPR) {
              stop = 1;
            }
            if (instruction_input.instruction->slice(6, 0) == BR) {
              (*bp_output.tag) <= it.num() + 1;
              (*bp_output.jp) <= register_file.jump;
            }
            break;
          }
          case JP: {
            (*it).state = 0b10;
            (*it).value = (*it).pc.Toi() + 4;
            break;
          }
          case LUI: {
            (*it).state = 0b10;
            (*it).value = (*it).instruction.slice_with_sign(31, 12) << 12;
            break;
          }
          case AUIPC: {
            (*it).state = 0b10;
            (*it).value = (*it).pc.Toi() + ((*it).instruction.slice_with_sign(31, 12) << 12);
            break;
          }
          default: {
            
            break;
          }
        }
        if ((*it).instruction.slice(6, 0) != 0b0100011 && (*it).instruction.slice(6, 0) != 0b1100011) {
          register_file.state[(*it).destination.Toi()] = it.num() + 1;
        }
      }
    }

  }

  void WorkRsBack() {
    if (cdb_input.cdb->alu_done.Toi()) {
      queue.At(cdb_input.cdb->alu_tag.Toi() - 1).value = cdb_input.cdb->alu_data.Toi();
      queue.At(cdb_input.cdb->alu_tag.Toi() - 1).state = 0b10;
    }
    if (cdb_input.cdb->ls_done.Toi()) {
      queue.At(cdb_input.cdb->ls_tag.Toi() - 1).value = cdb_input.cdb->ls_data.Toi();
      queue.At(cdb_input.cdb->ls_tag.Toi() - 1).state = 0b10;
    }
  }
  
  void Work() override {
    WorkBp();
    WorkRsBack();
    WorkEnQueue();
    WorkFlush();
    WorkDecode();
    WorkDeQueue();
  }

  void Update() override {
    if (stop) {
      (*instruction_output.ready) <= !stop;
    }
    instruction_output.Update();
    rs_output.Update();
    bp_output.Update();
    lsb_output.Update();
  }
};

}

#endif // ROB_MODULE_HPP
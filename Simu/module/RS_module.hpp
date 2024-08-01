#ifndef RS_MODULE_HPP
#define RS_MODULE_HPP

#include "BP_module.hpp"
#include "all_tools.hpp"
#include "register.hpp"
#include <cstdint>
#include <sys/types.h>

namespace parsifal_modules {

struct RsElement {
  Reg busy;
  Reg op;
  Reg vj;
  Reg vk;
  Reg qj;
  Reg qk;
  Reg a;
  Reg dest;

  RsElement &operator=(const RsElement &other) {
    busy = other.busy;
    op = other.op;
    vj = other.vj;
    vk = other.vk;
    qj = other.qj;
    qk = other.qk;
    a = other.a;
    dest = other.dest;
    return *this;
  }

  RsElement &operator<=(const RsElement &other) {
    busy <= other.busy;
    op <= other.op;
    vj <= other.vj;
    vk <= other.vk;
    qj <= other.qj;
    qk <= other.qk;
    a <= other.a;
    dest <= other.dest;
    return *this;
  }

  void Update() {
    busy.Update();
    op.Update();
    vj.Update();
    vk.Update();
    qj.Update();
    qk.Update();
    a.Update();
    dest.Update();
  }
};

struct RsInput {
  shared_ptr<RsElement> instruction;
  shared_ptr<Wire> tag;
  shared_ptr<CDB> cdb;

  RsInput &operator=(RsInput &other) {
    instruction = other.instruction;
    tag = other.tag;
    cdb = other.cdb;
    return *this;
  }
};

struct RsAluOutput {
  shared_ptr<Reg> operand;
  shared_ptr<Reg> a;
  shared_ptr<Reg> b;
  shared_ptr<Reg> tag;

  void Update() {
    a->Update();
    b->Update();
    operand->Update();
    tag->Update();
  }

  RsAluOutput &operator=(RsAluOutput &other) {
    operand = other.operand;
    a = other.a;
    b = other.b;
    tag = other.tag;
    return *this;
  }
};

struct RsRobOutput {
  shared_ptr<Reg> alu_ready;
  // shared_ptr<Reg> tag;
  // shared_ptr<Reg> data;

  void Update() {
    alu_ready->Update();
    // tag->Update();
    // data->Update();
  }

  RsRobOutput &operator=(RsRobOutput &other) {
    alu_ready = other.alu_ready;
    // tag = other.tag;
    // data = other.data;
    return *this;
  }
};

class RsModule : public Module {
private:
  static const int ARRAY_SIZE = 16;
  RsElement alu_array[ARRAY_SIZE];
  uint32_t alu_array_size = 0;
  RsInput rs_input;
  RsAluOutput rs_alu_output;
  RsRobOutput rs_rob_output;
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
    BLTU
  };

public:
  void Set(RsInput set_input, RsAluOutput set_alu_output, RsRobOutput set_rob_output) {
    rs_input = set_input;
    rs_alu_output = set_alu_output;
    rs_rob_output = set_rob_output;
  }

  void WorkInstruction() {
    if (rs_input.tag->Toi()) {
      for (int i = 0; i < ARRAY_SIZE; i++) {
        if (alu_array[i].busy == 0) {
          alu_array[i] = (*rs_input.instruction);
          alu_array_size++;
          break;
        }
      }
    }
  }

  void WorkArray() {
    bool flag = false;
    (*rs_alu_output.tag) <= 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
      if (alu_array[i].busy.Toi() == 1) {
        if (!alu_array[i].qj.Toi() && !alu_array[i].qk.Toi()) {
          (*rs_alu_output.a) <= alu_array[i].vj.Toi();
          (*rs_alu_output.b) <= alu_array[i].vk.Toi();
          (*rs_alu_output.operand) <= alu_array[i].op.Toi();
          (*rs_alu_output.tag) <= alu_array[i].dest.Toi();
          flag = true;
          alu_array[i].busy = 2;
        }
      }
      if (flag) {
        break;
      }
    }
    if (!flag) {
      (*rs_alu_output.tag) <= 0;
    }
    if (alu_array_size < 14) {
      (*rs_rob_output.alu_ready) <= 1;
    } else {
      (*rs_rob_output.alu_ready) <= 0;
    }
  }

  void WorkCdb() {
    if (rs_input.cdb->alu_done.Toi()) {
      for (int i = 0; i <= ARRAY_SIZE; i++) {
        if (alu_array[i].busy.Toi()) {
          if (alu_array[i].dest.Toi() == rs_input.cdb->alu_tag.Toi()) {
            alu_array[i].a = rs_input.cdb->alu_data;
            alu_array[i].busy = 0;
            alu_array_size--;
          }
          if (alu_array[i].qj == rs_input.cdb->alu_tag.Toi()) {
            alu_array[i].vj = rs_input.cdb->alu_data;
            alu_array[i].qj = 0;
          }
          if (alu_array[i].qk == rs_input.cdb->alu_tag.Toi()) {
            alu_array[i].vk = rs_input.cdb->alu_data;
            alu_array[i].qk = 0;
          }
        }
      }
      // (*rs_rob_output.tag) <= rs_input.cdb->alu_tag.Toi();
      // (*rs_rob_output.data) <= rs_input.cdb->alu_data.Toi();
    }
    if (rs_input.cdb->ls_done.Toi()) {
      for (int i = 0; i <= ARRAY_SIZE; i++) {
        if (alu_array[i].busy.Toi()) {
          if (alu_array[i].dest.Toi() == rs_input.cdb->ls_tag.Toi()) {
            alu_array[i].a = rs_input.cdb->ls_data;
            alu_array[i].busy = 0;
            alu_array_size--;
          }
          if (alu_array[i].qj == rs_input.cdb->ls_tag.Toi()) {
            alu_array[i].vj = rs_input.cdb->ls_data;
            alu_array[i].qj = 0;
          }
          if (alu_array[i].qk == rs_input.cdb->ls_tag.Toi()) {
            alu_array[i].vk = rs_input.cdb->ls_data;
            alu_array[i].qk = 0;
          }
        }
      }
    }
  }

  void WorkFlush() {
    if (register_file.flush.Toi()) {
      for (int i = 0; i < ARRAY_SIZE; i++) {
        alu_array[i].busy = 0;
        alu_array_size = 0;
      }
      (*rs_alu_output.tag) <= 0;
    }
  }

  void Work() override {
    WorkFlush();
    WorkInstruction();
    WorkArray();
    WorkCdb();
  }

  void Update() override {
    rs_alu_output.Update();
    rs_rob_output.Update();
  }

};

}

#endif // RS_MODULE_HP
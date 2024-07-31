#ifndef LSB_MODULE_HPP
#define LSB_MODULE_HPP

#include "BP_module.hpp"
#include "all_tools.hpp"

namespace parsifal_modules {

struct LsbElement {
  Reg oprand;
  Reg data;
  Reg addr;
  Reg rd;
  Reg rs1;
  Reg imm;
  Reg tag;
  Reg complete;

  LsbElement &operator<=(const LsbElement &other) {
    this->oprand <= other.oprand;
    this->data <= other.data;
    this->addr <= other.addr;
    this->rd <= other.rd;
    this->rs1 <= other.rs1;
    this->imm <= other.imm;
    this->tag <= other.tag;
    this->complete <= other.complete;
    return *this;
  }

  void Update() {
    this->oprand.Update();
    this->data.Update();
    this->addr.Update();
    this->rd.Update();
    this->rs1.Update();
    this->imm.Update();
    this->tag.Update();
    this->complete.Update();
  }
};

struct LsbInput {
  shared_ptr<LsbElement> instruction;
  shared_ptr<Wire> ready;
  shared_ptr<Wire> mem_data;

  LsbInput &operator=(const LsbInput &other) {
    instruction = other.instruction;
    ready = other.ready;
    mem_data = other.mem_data;
    return *this;
  }
};

struct LsbCdbOutput {
  shared_ptr<CDB> cdb;

  void Update() {
    cdb->Update();
  }

  LsbCdbOutput &operator=(const LsbCdbOutput &other) {
    cdb = other.cdb;
    return *this;
  }
};

struct LsbMemOutput {
  shared_ptr<Wire> operand;
  shared_ptr<Wire> addr;
  shared_ptr<Wire> data;

  void Update() {
    operand->Update();
    addr->Update();
    data->Update();
  }

  LsbMemOutput &operator=(const LsbMemOutput &other) {
    operand = other.operand;
    addr = other.addr;
    data = other.data;
    return *this;
  }
};

struct LsbRobOutput {
  shared_ptr<Reg> ls_ready;

  void Update() {
    ls_ready->Update();
  }

  LsbRobOutput &operator=(const LsbRobOutput &other) {
    ls_ready = other.ls_ready;
    return *this;
  }
};

class LsbModule : public Module {
private:
  LsbInput input;
  LsbCdbOutput cdb_output;
  LsbMemOutput mem_output;
  LsbRobOutput rob_output;
  Queue<LsbElement, 64> queue;

public:
  void WorkFlush() {
    if (register_file.flush.Toi()) {
      queue.Clear();
    }
  }

  void Set(LsbInput set_input, LsbCdbOutput set_output, LsbMemOutput set_mem_output, LsbRobOutput set_rob_output) {
    input = set_input;
    cdb_output = set_output;
    mem_output = set_mem_output;
    rob_output = set_rob_output;
  }

  void WorkInput() {
    if (input.instruction->tag.Toi()) {
      queue.PushBack((*input.instruction));
    }

    if (input.ready->Toi() & 0b10) {
      for (auto it = queue.begin(); it != queue.end(); ++it) {
        if ((*it).complete.Toi() == 1) {
          (*it).data = input.mem_data->Toi();
          (*it).complete = 2;
          break;
        }
      }
    }

    if (queue.Size() < 62) {
      (*rob_output.ls_ready) <= 1;
    } else {
      (*rob_output.ls_ready) <= 0;
    }
  }

  void WorkDependence() {
    if (cdb_output.cdb->alu_done.Toi()) {
      for (auto it = queue.begin(); it != queue.end(); ++it) {
        if (!(*it).complete.Toi()) {
          if ((*it).rd == cdb_output.cdb->alu_tag.Toi()) {
            (*it).addr = cdb_output.cdb->alu_data.Toi();
            (*it).rd = 0;
          }
          if ((*it).rs1 == cdb_output.cdb->alu_tag.Toi()) {
            (*it).addr = cdb_output.cdb->alu_data.Toi();
            (*it).rs1 = 0;
          }
        }
      } 
    }
    if (cdb_output.cdb->ls_done.Toi()) {
      for (auto it = queue.begin(); it != queue.end(); ++it) {
        if (!(*it).complete.Toi()) {
          if ((*it).rd == cdb_output.cdb->ls_tag.Toi()) {
            (*it).addr = cdb_output.cdb->ls_data.Toi();
            (*it).rd = 0;
          }
          if ((*it).rs1 == cdb_output.cdb->ls_tag.Toi()) {
            (*it).addr = cdb_output.cdb->ls_data.Toi();
            (*it).rs1 = 0;
          }
        }
      }
    }
  }

  void WorkMem() {
    for (auto it = queue.begin(); it != queue.end(); ++it) {
      if ((*it).rd.Toi() == 0 && (*it).rs1.Toi() == 0) {
        if ((*it).oprand[31] == 0) {
          if (input.ready->Toi()) {
            (*mem_output.operand) <= (*it).oprand;
            (*mem_output.addr) <= (*it).addr.Toi() + (*it).imm.Toi();
            (*mem_output.data) <= (*it).data;
            (*it).complete = 1;
            break;
          }
        } else {
          break;
        }
      }
    }
  }

  void WorkOutput() {
    (*mem_output.operand) <= 0;
    if ((*queue.begin()).complete.Toi() == 2) {
      cdb_output.cdb->ls_tag <= (*queue.begin()).tag;
      cdb_output.cdb->ls_data <= (*queue.begin()).data;
      cdb_output.cdb->ls_done <= 1;
      queue.PopFront();
    }
    if (queue.Front().oprand[31] && queue.Front().complete.Toi() == 0 && register_file.head_tag.Toi() == queue.Front().tag.Toi()) {
      if (input.ready->Toi()) {
        (*mem_output.operand) <= (queue.Front().oprand.Toi() | (1 << 20));
        (*mem_output.addr) <= queue.Front().addr.Toi() + queue.Front().imm.Toi();
        (*mem_output.data) <= queue.Front().data;
        queue.Front().complete = 1;
      }
    }
  }

  void Work() override {
    WorkInput();
    WorkDependence();
    WorkMem();
    WorkOutput();
  }

  void Update() override {
    cdb_output.Update();
    mem_output.Update();
    rob_output.Update();
  }
};

}

#endif // LSB_MODULE_HPP
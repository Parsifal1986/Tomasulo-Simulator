#ifndef BP_MODULE_HPP
#define BP_MODULE_HPP

#include "all_tools.hpp"

extern RegisterFile register_file;

namespace parsifal_modules {

struct BpInput {
  shared_ptr<Wire> jp;
  shared_ptr<Wire> tag;
  shared_ptr<CDB> cdb;

  BpInput &operator=(const BpInput &other) {
    jp = other.jp;
    tag = other.tag;
    cdb = other.cdb;
    return *this;
  }

  void Update() {
    jp->Update();
    tag->Update();
    cdb->Update();
  }
};

struct BpOutput {
  shared_ptr<Reg> tag;
  shared_ptr<Reg> flush;

  BpOutput &operator=(const BpOutput &other) {
    tag = other.tag;
    flush = other.flush;
    return *this;
  }
  
  void Update() {
    tag->Update();
    flush->Update();
  }
};

struct BpElement {
  Reg jp;
  Reg tag;

  BpElement &operator<=(const BpElement &other) {
    jp <= other.jp;
    tag <= other.tag;
    return *this;
  }

  void Update() {
    jp.Update();
    tag.Update();
  }
};

class BpModule : public Module {
private:
  BpInput input;
  BpOutput output;
  Queue<BpElement, 16> queue;
  Reg num = 0;

public:
  void Set(BpInput set_bp_input, BpOutput set_bp_output) {
    input = set_bp_input;
    output = set_bp_output;
  }

  void Work() override {
    if (input.tag->Toi()) {
      queue.PushBack(BpElement{Reg(input.jp->Toi()), Reg(input.tag->Toi())});
    }
    if (queue.Size() && input.cdb->alu_done.Toi()) {
      if (input.cdb->alu_tag == queue.Front().tag) {
        if ((input.cdb->alu_data == queue.Front().jp)) {
          if (num.Toi() < 0b11) {
            num = num.Toi() + 1;
          }
        } else {
          if (num.Toi() > 0b00) {
            num = num.Toi() - 1;
          }
        }
        (*output.tag) <= ((input.cdb->alu_data == queue.Front().jp) ? queue.Front().tag : 0);
        queue.PopFront();
      }
    }
  }

  void Update() override {
    register_file.jump <= (num.Toi() & 0b10);
    input.Update();
    output.Update();
  }
};

}


#endif // BP_MODULE_HPP
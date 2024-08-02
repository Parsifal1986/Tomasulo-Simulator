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
  Reg busy;

  BpElement &operator<=(const BpElement &other) {
    jp <= other.jp;
    tag <= other.tag;
    busy <= other.busy;
    return *this;
  }

  void Update() {
    busy.Update();
    jp.Update();
    tag.Update();
  }
};

class BpModule : public Module {
static const int ARRAY_SIZE = 64;
private:
  BpInput input;
  BpOutput output;
  BpElement array[ARRAY_SIZE];
  int array_size = 0;
  Reg num = 0;

public:
  int accurate = 0;
  int wrong = 0;
  void Set(BpInput set_bp_input, BpOutput set_bp_output) {
    input = set_bp_input;
    output = set_bp_output;
  }

  void Work() override {
    (*output.tag) <= 0;
    if (input.tag->Toi()) {
      for (int i = 0; i < ARRAY_SIZE; i++) {
        if (!array[i].busy.Toi()) {
          array[i] = BpElement { Reg(input.jp->Toi()), Reg(input.tag->Toi()), 1};
          array_size++;
          break;
        }
      }
    }
    if (array_size && input.cdb->alu_done.Toi()) {
      for (int i = 0; i < ARRAY_SIZE; i++) {
        if (input.cdb->alu_tag == array[i].tag && array[i].busy.Toi()) {
          if ((input.cdb->alu_data.Toi())) {
            if (num.Toi() < 0b11) {
              num = num.Toi() + 1;
            }
          } else {
            if (num.Toi() > 0b00) {
              num = num.Toi() - 1;
            }
          }
          accurate += (input.cdb->alu_data.Toi() == array[i].jp.Toi());
          wrong += (input.cdb->alu_data.Toi() != array[i].jp.Toi());
          (*output.tag) <= ((input.cdb->alu_data == array[i].jp) ? 0 : array[i].tag);
          array[i].busy = 0;
          array_size--;
        }
      }
    }
    if (register_file.flush.Toi()) {
      for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i].busy = 0;
        array_size = 0;
      }
    }
    register_file.jump <= num[1];
  }

  void Update() override {
    input.Update();
    output.Update();
  }
};

}


#endif // BP_MODULE_HPP
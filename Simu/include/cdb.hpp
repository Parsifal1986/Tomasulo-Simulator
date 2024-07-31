#ifndef CDB_HPP
#define CDB_HPP

#include "register.hpp"

namespace parsifal_tools {

struct CDB {
  Reg alu_tag;
  Reg alu_data;
  Reg alu_done;

  Reg ls_tag;
  Reg ls_data;
  Reg ls_done;

  void Update() {
    alu_tag.Update();
    alu_data.Update();
    alu_done.Update();

    ls_tag.Update();
    ls_data.Update();
    ls_done.Update();
  }

  CDB &operator=(const CDB &other) {
    alu_tag = other.alu_tag;
    alu_data = other.alu_data;
    alu_done = other.alu_done;

    ls_tag = other.ls_tag;
    ls_done = other.ls_done;
    ls_data = other.ls_data;
    return *this;
  }
};

}

#endif // CDB_HPP
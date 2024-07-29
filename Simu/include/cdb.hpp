#ifndef CDB_HPP
#define CDB_HPP

#include "register.hpp"

namespace parsifal_tools {

struct CDB {
  Reg tag;
  Reg data;
  Reg done;

  void Update() {
    tag.Update();
    data.Update();
    done.Update();
  }
};

}

#endif // CDB_HPP
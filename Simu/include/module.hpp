#ifndef MODULE_HPP
#define MODULE_HPP

#include <cstdio>
#include <sys/types.h>
#include <wire.hpp>
#include <register.hpp>

namespace parsifal_tools {

class Module {
public:
  virtual void Work() = 0;

  virtual void Update() {

  };
};

} // namespace parsifal_tools

#endif // MODULE_HPP
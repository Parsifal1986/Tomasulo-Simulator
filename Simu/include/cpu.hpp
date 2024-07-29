#ifndef CPU_HPP
#define CPU_HPP

#include "module.hpp"
#include <iostream>
#include <memory>
#include <vector>

namespace parsifal_tools {

class Cpu {
private:
  int clk;
  std::vector<std::shared_ptr<Module>> modules;

public:
  Cpu() : clk(0) {}

  Cpu(std::shared_ptr<Module> module) : clk(0) { AddModule(module); }

  ~Cpu() = default;

  void Initialize() { clk = 0; }

  void AddModule(std::shared_ptr<Module> module) {
    if (clk) {
      std::cout << "CPU Has Already Started Working!" << std::endl;
    }
    modules.push_back(module);
  }

  void cycle() {
    clk++;

    for (auto it : modules) {
      it->Work();
    }

    for (auto it : modules) {
      it->Update();
    }
  }
};

} // namespace parsifal_tools

#endif // CPU_HPP
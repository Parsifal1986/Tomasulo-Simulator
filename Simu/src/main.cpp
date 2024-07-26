#include "tools.hpp"
#include "all_modules.hpp"
#include <memory>

int main() {
  Cpu cpu;
  
  const char* path = "array_test1.data";

  auto memory = std::make_shared<MemoryModule>();

  memory->ReadIn(path);

  cpu.AddModule(memory);
  
  return 0;
}
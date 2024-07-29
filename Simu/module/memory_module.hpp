#ifndef MEMORY_MODULE_HPP
#define MEMORY_MODULE_HPP

#include "ROB_module.hpp"
#include "tools.hpp"
#include <cstdint>
#include <fstream>
#include <sstream>

namespace parsifal_modules {

const int SIZE = 1<<22;

struct MemoryInput {
  shared_ptr<Wire> operand;
  shared_ptr<Wire> addr;
  shared_ptr<Wire> data;

  MemoryInput &operator=(MemoryInput &other) {
    operand = other.operand;
    addr = other.addr;
    data = other.data;
    return *this;
  }
};

struct MemoryOutput {
  shared_ptr<Reg> output;
  shared_ptr<Reg> ready;

  MemoryOutput &operator=(MemoryOutput &other) {
    output = other.output;
    ready = other.ready;
    return *this;
  }

  void Update() {
    output->Update();
    ready->Update();
  }
};

class MemoryModule : public Module {
private:
  enum MemoryOpcode : uint32_t {
    BYTE = 0,
    HALFWORD = 1,
    WORD = 2,
    INSTRUCTION = 3
  };
  uint8_t memory[SIZE];
  MemoryInput instruction_input;
  MemoryInput ls_input;
  MemoryOutput instruction_output;
  MemoryOutput ls_output;
  
  struct OutputBuffer {
    uint32_t buffer[2];
    int wait_time[2] = {-1, -1};

    void operator<=(uint32_t data) {
      for (int i = 0; i < 2; i++) {
        if (wait_time[i] == -1) {
          buffer[i] = data;
          wait_time[i] = 2;
          break;
        }
      }
    }
  } output_buffer;

public:
  void Set(MemoryInput set_ins_input, MemoryInput set_ls_input, MemoryOutput set_ins_output, MemoryOutput set_ls_output) {
    instruction_input = set_ins_input;
    instruction_output = set_ins_output;

    ls_input = set_ls_input;
    ls_output = set_ls_output;
  }

  void ReadIn(const char* path) {
    std::ifstream inputfile(path);

    if (!inputfile) {
      std::cerr << "Can't open the file!" << std::endl;
      return;
    }

    std::string line;
    uint32_t pos;
    bool flag;

    while (std::getline(inputfile, line)) {
      if (line.empty()) {
        continue;
      }
      if (line[0] == '@') {
        std::istringstream iss(line.substr(1));
        iss >> std::hex >> pos;
        if (!flag) {
          register_file.program_counter = pos;
          flag = 1;
        }
        continue;
      }

      std::istringstream iss(line);

      while (iss) {
        for (int i = 3; i >= 0; --i) {
          uint32_t byte;
          iss >> std::hex >> byte;
          memory[pos + i] = static_cast<uint8_t>(byte);
        }
        pos += 4;
      }
    }
  }

  void WorkInstruction () {
    if (!(*instruction_input.operand)[0]) {
      (*instruction_output.output) <= 0;
      return;
    }
    (*instruction_output.output) <= (uint32_t)((memory[instruction_input.addr->Toi()] << 24) + (memory[instruction_input.addr->Toi() + 1] << 16) + (memory[instruction_input.addr->Toi() + 2] << 8) + (memory[instruction_input.addr->Toi() + 3]));
    (*instruction_output.ready) <= instruction_input.addr->Toi();
  }

  void WorkLS() {
    if (!(*ls_input.operand)[0]) {
      return;
    }
    if ((*ls_input.operand)[31] == 0) {
      switch (ls_input.operand->slice(30, 29)) {
        case BYTE : {
          output_buffer <= (uint32_t)memory[ls_input.addr->Toi()];
          break;
        }
        case HALFWORD : {
          output_buffer <= (uint32_t)((memory[ls_input.addr->Toi()] << 8) + memory[ls_input.addr->Toi() + 1]);
          break;
        }
        case WORD : {
          output_buffer <= (uint32_t)((memory[ls_input.addr->Toi()] << 24) + (memory[ls_input.addr->Toi() + 1] << 16) + (memory[ls_input.addr->Toi() + 2] << 8) + (memory[ls_input.addr->Toi() + 3]));
          break;
        }
      }
    } else if ((*ls_input.operand)[31] == 1) {
      switch (ls_input.operand->slice(30, 29)) {
        case BYTE : {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->Toi());
          output_buffer <= 1;
          break;
        }
        case HALFWORD: {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->slice(15, 8));
          memory[ls_input.addr->Toi() + 1] = (uint8_t)(ls_input.data->slice(7, 0));
          output_buffer <= 1;
          break;
        }
        case WORD: {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->slice(31, 24));
          memory[ls_input.addr->Toi() + 1] = (uint8_t)(ls_input.data->slice(23, 16));
          memory[ls_input.addr->Toi() + 2] = (uint8_t)(ls_input.data->slice(15, 8));
          memory[ls_input.addr->Toi() + 3] = (uint8_t)(ls_input.data->slice(7, 0));
          output_buffer <= 1;
          break;
        }
      }
    } else {
      std::cerr << "Wrong operand code!" << std::endl;
    }
  }

  void Work() override {
    WorkInstruction();
    WorkLS();
  }

  void UpdateLS() {
      bool flag = 0;
      for (int i = 0; i < 2; i++) {
        output_buffer.wait_time[i] = std::max(output_buffer.wait_time[i] - 1, -1);
        if (output_buffer.wait_time[i] == 0) {
          (*ls_output.output) = output_buffer.buffer[i];
          flag = 1;
        }
      }
      (*ls_output.ready) = flag;
  }

  void Update() override {
    instruction_output.Update();
    UpdateLS();
  }
};

}

#endif //MEMORY_MODULE_HPP
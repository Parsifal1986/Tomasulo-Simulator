#ifndef MEMORY_MODULE_HPP
#define MEMORY_MODULE_HPP

#include "BP_module.hpp"
#include "ROB_module.hpp"
#include "all_tools.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

namespace parsifal_modules {

const size_t SIZE = 1<<20;

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
    BYTE = 0b000,
    BYTEU = 0b100,
    HALFWORD = 0b001,
    HALFWORDU = 0b101,
    WORD = 0b010
  };
  uint8_t memory[SIZE];
  MemoryInput instruction_input;
  MemoryInput ls_input;
  MemoryOutput instruction_output;
  MemoryOutput ls_output;
  
  struct OutputBuffer {
    uint32_t buffer;
    int wait_time = -1;

    void operator<=(uint32_t data) {
      if (wait_time == -1) {
        buffer = data;
        wait_time = 2;
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
    // std::ifstream inputfile(path);

    // if (!inputfile) {
    //   std::cerr << "Can't open the file!" << std::endl;
    //   return;
    // }

    std::string line;
    uint32_t pos;
    bool flag;

    while (std::getline(/*inputfile*/std::cin, line)) {
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
        bool flag = true;
        for (int i = 0; i < 4; ++i) {
          uint32_t byte;
          if (!(iss >> std::hex >> byte)) {
            flag = false;
            break;
          }
          memory[pos + i] = static_cast<uint8_t>(byte);
        }
        if (!flag) {
          break;
        }
        pos += 4;
      }
    }
  }

  void WorkInstruction () {
    if (!instruction_input.operand->Toi()) {
      (*instruction_output.output) <= 0;
      return;
    }
    (*instruction_output.output) <= (uint32_t)((memory[instruction_input.addr->Toi()]) + (memory[instruction_input.addr->Toi() + 1] << 8) + (memory[instruction_input.addr->Toi() + 2] << 16) + (memory[instruction_input.addr->Toi() + 3] << 24));
    (*instruction_output.ready) <= instruction_input.addr->Toi();
  }

  void WorkLS() {
    if (!(*ls_input.operand)[20]) {
      return;
    }
    if ((*ls_input.operand)[31] == 0) {
      switch (ls_input.operand->slice(2, 0)) {
        case BYTE : {
          output_buffer <= static_cast<uint32_t>(memory[ls_input.addr->Toi()]);
          break;
        }
        case BYTEU : {
          output_buffer <= static_cast<uint32_t>((uint8_t)(memory[ls_input.addr->Toi()]));
          break;
        }
        case HALFWORD : {
          output_buffer <= static_cast<uint32_t>(memory[ls_input.addr->Toi()] + (memory[ls_input.addr->Toi() + 1] << 8));
          break;
        }
        case HALFWORDU: {
          output_buffer <= static_cast<uint32_t>((uint16_t)(memory[ls_input.addr->Toi()]) + (uint16_t)(memory[ls_input.addr->Toi() + 1] << 8));
          break;
        }
        case WORD : {
          output_buffer <= (uint32_t)(memory[ls_input.addr->Toi()] + (memory[ls_input.addr->Toi() + 1] << 8) + (memory[ls_input.addr->Toi() + 2] << 16) + (memory[ls_input.addr->Toi() + 3]  << 24));
          break;
        }
      }
    } else if ((*ls_input.operand)[31] == 1) {
      switch (ls_input.operand->slice(2, 0)) {
        case BYTE : {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->slice(7, 0));
          output_buffer <= 1;
          break;
        }
        case HALFWORD: {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->slice(7, 0));
          memory[ls_input.addr->Toi() + 1] = (uint8_t)(ls_input.data->slice(15, 8));
          output_buffer <= 1;
          break;
        }
        case WORD: {
          memory[ls_input.addr->Toi()] = (uint8_t)(ls_input.data->slice(7, 0));
          memory[ls_input.addr->Toi() + 1] = (uint8_t)(ls_input.data->slice(15, 8));
          memory[ls_input.addr->Toi() + 2] = (uint8_t)(ls_input.data->slice(23, 16));
          memory[ls_input.addr->Toi() + 3] = (uint8_t)(ls_input.data->slice(31, 24));
          output_buffer <= 1;
          break;
        }
      }
    } else {
      std::cerr << "Wrong operand code!" << std::endl;
    }
  }

  void WorkFlush() {
    if (register_file.flush.Toi()) {
      (*instruction_output.output) <= 0;
      (output_buffer.buffer) = 0;
      (output_buffer.wait_time) = -1;
    }
  }

  void Work() override {
    WorkInstruction();
    WorkLS();
    WorkFlush();
  }

  void UpdateLS() {
    output_buffer.wait_time = std::max(output_buffer.wait_time - 1, -1);
    if (output_buffer.wait_time <= 0 && (!ls_output.ready->Toi())) {
      (*ls_output.ready) = 1;
    } else {
      (*ls_output.ready) = 0;
    }
    if (output_buffer.wait_time == 0) {
      (*ls_output.output) = output_buffer.buffer;
      (*ls_output.ready) = 0b11;
    }
  }

  void Update() override {
    instruction_output.Update();
    // ls_output.Update();
    UpdateLS();
  }
};

}

#endif //MEMORY_MODULE_HPP
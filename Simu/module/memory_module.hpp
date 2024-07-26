#ifndef MEMORY_MODULE_HPP
#define MEMORY_MODULE_HPP

#include "tools.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace parsifal_modules {

const int SIZE = 1<<22;

struct MemoryInput {
  std::shared_ptr<Wire> operand;
  std::shared_ptr<Wire> addr;
  std::shared_ptr<Wire> data;
};

struct MemoryOutput {
  std::shared_ptr<Reg> output;
  std::shared_ptr<Reg> done;
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
  MemoryInput input;
  MemoryOutput output;
  
  struct OutputBuffer {
    uint32_t buffer[3];
    int wait_time[3] = {-1, -1, -1};

    void operator<=(uint32_t data) {
      for (int i = 0; i < 3; i++) {
        if (wait_time[i] == -1) {
          buffer[i] = data;
          wait_time[i] = 3;
          break;
        }
      }
    }
  } output_buffer;

public:
  void Set(MemoryInput set_input, MemoryOutput set_output) {
    input.addr = set_input.addr;
    input.operand = set_input.operand;
    input.data = set_input.data;
    output.output = set_output.output;
  }

  void ReadIn(const char* path) {
    std::ifstream inputfile(path);

    if (!inputfile) {
      std::cerr << "Can't open the file!" << std::endl;
      return;
    }

    std::string line;
    uint32_t pos;

    while (std::getline(inputfile, line)) {
      if (line.empty()) {
        continue;
      }
      if (line[0] == '@') {
        std::istringstream iss(line.substr(1));
        iss >> std::hex >> pos;
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

  void Work() override {
    if ((*input.operand)[31] == 0) {
      switch (input.operand->slice(30, 29)) {
        case BYTE : {
          output_buffer <= (uint32_t)memory[input.addr->Toi()];
          break;
        }
        case HALFWORD : {
          output_buffer <= (uint32_t)((memory[input.addr->Toi()] << 8) + memory[input.addr->Toi() + 1]);
          break;
        }
        case WORD : {
          output_buffer <= (uint32_t)((memory[input.addr->Toi()] << 24) + (memory[input.addr->Toi() + 1] << 16) + (memory[input.addr->Toi() + 2] << 8) + (memory[input.addr->Toi() + 3]));
          break;
        }
        case INSTRUCTION : {
          (*output.output) = (uint32_t)((memory[input.addr->Toi()] << 24) + (memory[input.addr->Toi() + 1] << 16) + (memory[input.addr->Toi() + 2] << 8) + (memory[input.addr->Toi() + 3]));
        }
      }
    } else if ((*input.operand)[30] == 1) {
      switch (input.operand->slice(30, 29)) {
        case BYTE : {
          memory[input.addr->Toi()] = (uint8_t)(input.data->Toi());
          output_buffer <= 1;
          break;
        }
        case HALFWORD: {
          memory[input.addr->Toi()] = (uint8_t)(input.data->slice(15, 8));
          memory[input.addr->Toi() + 1] = (uint8_t)(input.data->slice(7, 0));
          output_buffer <= 1;
          break;
        }
        case WORD: {
          memory[input.addr->Toi()] = (uint8_t)(input.data->slice(31, 24));
          memory[input.addr->Toi() + 1] = (uint8_t)(input.data->slice(23, 16));
          memory[input.addr->Toi() + 2] = (uint8_t)(input.data->slice(15, 8));
          memory[input.addr->Toi() + 3] = (uint8_t)(input.data->slice(7, 0));
          output_buffer <= 1;
          break;
        }
      }
    } else {
      std::cerr << "Wrong operand code!" << std::endl;
    }
  }

  void Update() override {
    for (int i = 0; i < 3; i++) {
      output_buffer.wait_time[i] = std::max(output_buffer.wait_time[i] - 1, -1);
      if (output_buffer.wait_time[i] == 0) {
        (*output.output) = output_buffer.buffer[i];
        (*output.done) = 1;
      }
    }
  }
};

}

#endif //MEMORY_MODULE_HPP
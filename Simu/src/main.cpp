#include "ROB_module.hpp"
#include "instruction_module.hpp"
#include "memory_module.hpp"
#include "tools.hpp"
#include "all_modules.hpp"
#include <memory>

RegisterFile register_file;

int main() {
  Cpu cpu;

  cpu.Initialize();
  
  const char* path = "array_test1.data";

  auto memory = std::make_shared<MemoryModule>();
  auto alu = std::make_shared<AluModule>();
  auto rob = std::make_shared<RobModule>();
  auto iu = std::make_shared<InstructionModule>();
  auto rs = std::make_shared<RsModule>();

  shared_ptr<Wire> alu_a = std::make_shared<Wire>();
  shared_ptr<Wire> alu_b = std::make_shared<Wire>();
  shared_ptr<Wire> alu_operand = std::make_shared<Wire>();

  shared_ptr<Wire> memory_ins_operand = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_addr = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_data = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_output = std::make_shared<Reg>();
  shared_ptr<Wire> memory_ins_pc = std::make_shared<Reg>();

  shared_ptr<Wire> memory_data_operand = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_addr = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_data = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_output = std::make_shared<Reg>();
  shared_ptr<Wire> memory_data_done = std::make_shared<Reg>();

  shared_ptr<Wire> ins_rob_instruction = std::make_shared<Reg>();
  shared_ptr<Wire> ins_rob_program_counter = std::make_shared<Reg>();

  shared_ptr<Wire> rob_ins_pc = std::make_shared<Reg>();
  shared_ptr<Wire> rob_ins_instruction = std::make_shared<Reg>();
  shared_ptr<Wire> rob_ready = std::make_shared<Reg>();
  shared_ptr<Wire> rob_ls_ready = std::make_shared<Reg>();
  shared_ptr<Wire> rob_alu_ready = std::make_shared<Reg>();
  shared_ptr<Wire> rob_rs_tag = std::make_shared<Reg>();
  shared_ptr<Wire> rob_rs_instruction = std::make_shared<Reg>();

  alu->Set(
    parsifal_modules::AluInput{alu_a, alu_b, alu_operand}, parsifal_modules::AluOutput{}
  );
  
  memory->Set(
    parsifal_modules::MemoryInput{memory_ins_operand, memory_ins_addr, memory_ins_data}, parsifal_modules::MemoryInput{memory_data_operand, memory_data_addr, memory_data_data},
    parsifal_modules::MemoryOutput{memory_ins_output, memory_ins_pc},
    parsifal_modules::MemoryOutput{memory_data_output, memory_data_done}
  );

  iu->Set(
    parsifal_modules::InstructionInput{rob_ready, memory_ins_pc ,memory_ins_output},
    parsifal_modules::InstructionOutput{memory_ins_addr, memory_ins_operand, rob_ins_pc, rob_ins_instruction});

  rob->Set(
    parsifal_modules::RobInsInput{rob_ins_pc, rob_ins_instruction},
    parsifal_modules::RobRsInput{rob_ls_ready, rob_alu_ready},
    parsifal_modules::RobInsOutput{rob_ready},
    parsifal_modules::RobRsOutput{rob_rs_tag, rob_rs_instruction}
    );

  memory->ReadIn(path);

  cpu.AddModule(memory);
  cpu.AddModule(iu);
  cpu.AddModule(rob);
  cpu.AddModule(rs);
  cpu.AddModule(alu);

  while (true) {
    cpu.cycle();
  }
  
  return 0;
}
#include "BP_module.hpp"
#include "LSB_module.hpp"
#include "ROB_module.hpp"
#include "RS_module.hpp"
#include "all_tools.hpp"
#include "all_modules.hpp"
#include <cstdio>
#include <memory>

RegisterFile register_file;

int main() {
  Cpu cpu;

  cpu.Initialize();

  const char *path = "tak.data";

  auto memory = std::make_shared<MemoryModule>();
  auto alu = std::make_shared<AluModule>();
  auto rob = std::make_shared<RobModule>();
  auto iu = std::make_shared<InstructionModule>();
  auto rs = std::make_shared<RsModule>();
  auto lsb = std::make_shared<LsbModule>();
  auto bp = std::make_shared<BpModule>();

  shared_ptr<Wire> alu_a = std::make_shared<Wire>();
  shared_ptr<Wire> alu_b = std::make_shared<Wire>();
  shared_ptr<Wire> alu_operand = std::make_shared<Wire>();
  shared_ptr<Wire> alu_tag = std::make_shared<Wire>();

  shared_ptr<Wire> memory_ins_operand = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_addr = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_data = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_output = std::make_shared<Wire>();
  shared_ptr<Wire> memory_ins_pc = std::make_shared<Wire>();

  shared_ptr<Wire> memory_data_operand = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_addr = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_data = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_output = std::make_shared<Wire>();
  shared_ptr<Wire> memory_data_done = std::make_shared<Wire>();

  shared_ptr<Wire> ins_rob_instruction = std::make_shared<Wire>();
  shared_ptr<Wire> ins_rob_program_counter = std::make_shared<Wire>();
  shared_ptr<Wire> ins_rob_pc = std::make_shared<Wire>();

  shared_ptr<Wire> rob_ins_instruction = std::make_shared<Wire>();
  shared_ptr<Wire> rob_ready = std::make_shared<Wire>();
  shared_ptr<Wire> rob_ls_ready = std::make_shared<Wire>();
  shared_ptr<Wire> rob_alu_ready = std::make_shared<Wire>();
  shared_ptr<Wire> rob_rs_tag = std::make_shared<Wire>();
  shared_ptr<Wire> rob_bp_tag = std::make_shared<Wire>();
  shared_ptr<Wire> rob_bp_jp = std::make_shared<Wire>();
  shared_ptr<Wire> rob_ins_pc = std::make_shared<Wire>();
  shared_ptr<parsifal_modules::RsElement> rob_rs_instruction = std::make_shared<parsifal_modules::RsElement>();
  shared_ptr<parsifal_modules::LsbElement> rob_lsb_instruction = std::make_shared<parsifal_modules::LsbElement>();

  // shared_ptr<Wire> rs_rob_tag = std::make_shared<Wire>();
  // shared_ptr<Wire> rs_rob_data = std::make_shared<Wire>();

  shared_ptr<Wire> bp_tag = std::make_shared<Wire>();
  shared_ptr<Wire> bp_flush = std::make_shared<Wire>();

  shared_ptr<CDB> cdb = std::make_shared<CDB>();

  alu->Set(
    parsifal_modules::AluInput{alu_a, alu_b, alu_operand, alu_tag},
    parsifal_modules::AluOutput{cdb}
  );
  
  memory->Set(
    parsifal_modules::MemoryInput{memory_ins_operand, memory_ins_addr, memory_ins_data},
    parsifal_modules::MemoryInput{memory_data_operand, memory_data_addr, memory_data_data},
    parsifal_modules::MemoryOutput{memory_ins_output, memory_ins_pc},
    parsifal_modules::MemoryOutput{memory_data_output, memory_data_done}
  );

  iu->Set(
    parsifal_modules::InstructionInput{rob_ready, memory_ins_pc ,memory_ins_output, rob_ins_pc},
    parsifal_modules::InstructionOutput{memory_ins_addr, memory_ins_operand, ins_rob_pc, rob_ins_instruction}
  );

  rob->Set(
    parsifal_modules::RobInsInput{ins_rob_pc, rob_ins_instruction},
    parsifal_modules::RobRsInput{rob_alu_ready},
    parsifal_modules::RobLsbInput{rob_ls_ready},
    parsifal_modules::RobBpInput{bp_tag},
    parsifal_modules::RobCdbInput{cdb},
    parsifal_modules::RobInsOutput{rob_ready, rob_ins_pc},
    parsifal_modules::RobRsOutput{rob_rs_tag, rob_rs_instruction},
    parsifal_modules::RobLsbOutput{rob_lsb_instruction},
    parsifal_modules::RobBpOutput{rob_bp_tag, rob_bp_jp}
  );

  lsb->Set(
    parsifal_modules::LsbInput{rob_lsb_instruction, memory_data_done, memory_data_output},
    parsifal_modules::LsbCdbOutput{cdb},
    parsifal_modules::LsbMemOutput{memory_data_operand, memory_data_addr, memory_data_data},
    parsifal_modules::LsbRobOutput{rob_ls_ready}
  );

  rs->Set(
    parsifal_modules::RsInput{rob_rs_instruction, rob_rs_tag, cdb},
    parsifal_modules::RsAluOutput{alu_operand, alu_a, alu_b, alu_tag},
    parsifal_modules::RsRobOutput{rob_alu_ready}
  );

  bp->Set(
    parsifal_modules::BpInput{rob_bp_jp, rob_bp_tag, cdb},
    parsifal_modules::BpOutput{bp_tag, bp_flush}
  );

  memory->ReadIn(path);

  cpu.AddModule(memory);
  cpu.AddModule(iu);
  cpu.AddModule(rob);
  cpu.AddModule(rs);
  cpu.AddModule(alu);
  cpu.AddModule(lsb);
  cpu.AddModule(bp);

  // freopen("1.out", "w", stdout);
  while (true) {
    // if (cpu.GetClk() == 384) {
    //   std::cout << "Here";
    // }
    cpu.cycle();
    cdb->Update();
    register_file.Update();

    if (register_file.terminal.Toi()) {
      break;
    }

    // std::cout << "round" << cpu.GetClk() << std::endl;
    // std::cout << register_file << std::endl;
  }

  std::cout << register_file.reg[10].slice(7, 0);

  // fclose(stdout);

  return 0;
}
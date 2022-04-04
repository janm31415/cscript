///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "vmcodetests.h"
#include <stdint.h>
#include "vm/vmcode.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include "test_assert.h"

VM_BEGIN


struct vmcode_fixture
  {
  vmcode code;
  uint8_t buffer[255];
  };

namespace
  {
  std::string _uchar_to_hex(unsigned char i)
    {
    std::string hex;
    int h1 = (i >> 4) & 0x0f;
    if (h1 < 10)
      hex += '0' + char(h1);
    else
      hex += 'A' + char(h1) - 10;
    int h2 = (i) & 0x0f;
    if (h2 < 10)
      hex += '0' + char(h2);
    else
      hex += 'A' + char(h2) - 10;
    return hex;
    }

  void _check_buffer(uint8_t* buffer, uint64_t size, std::vector<uint8_t> expected)
    {
    bool error = false;
    TEST_EQ(expected.size(), size);
    if (size != expected.size())
      error = true;
    if (size == expected.size())
      {
      for (size_t i = 0; i < size; ++i)
        {
        TEST_EQ(expected[i], buffer[i]);
        if (expected[i] != buffer[i])
          error = true;
        }
      }
    if (error)
      {
      printf("expect : ");
      for (size_t i = 0; i < expected.size(); ++i)
        {
        printf("%s ", _uchar_to_hex(expected[i]).c_str());
        }
      printf("\nactual : ");
      for (size_t i = 0; i < size; ++i)
        {
        printf("%s ", _uchar_to_hex(buffer[i]).c_str());
        }
      printf("\n");
      }
    }


  void vmcode_mov_rax_number()
    {
    vmcode code; uint8_t buffer[255];
    double d = 5.0;
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, *reinterpret_cast<uint64_t*>(&d));
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x40 });
    }

  void vmcode_mov_rax_number_2()
    {
    vmcode code; uint8_t buffer[255];
    int64_t d = 5;
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, d);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC7, 0xC0, 0x05, 0x00, 0x00, 0x00 });
    }

  void vmcode_mov_rbx_number()
    {
    vmcode code; uint8_t buffer[255];
    double d = 5.0;
    code.add(vmcode::MOV, vmcode::RBX, vmcode::NUMBER, *reinterpret_cast<uint64_t*>(&d));
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x40 });
    }

  void vmcode_mov_r15_number()
    {
    vmcode code; uint8_t buffer[255];
    double d = 5.0;
    code.add(vmcode::MOV, vmcode::R15, vmcode::NUMBER, *reinterpret_cast<uint64_t*>(&d));
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x40 });
    }

  void vmcode_mov_rax_rcx()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RCX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xC8 });
    }

  void vmcode_mov_rcx_rax()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RCX, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xC1 });
    }

  void vmcode_mov_rax_memrsp_8()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RSP, -8);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x44, 0x24, 0xF8 });
    }

  void vmcode_mov_rax_memrsp_4096()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RSP, -4096);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x84, 0x24, 0x00, 0xF0, 0xFF, 0xFF });
    }

  void vmcode_mov_memrsp_8_rax()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RSP, -8, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x44, 0x24, 0xF8 });
    }

  void vmcode_mov_memrsp_4096_rax()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RSP, -4096, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x84, 0x24, 0x00, 0xF0, 0xFF, 0xFF });
    }

  void vmcode_mov_memrsp_8_rcx()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RSP, -8, vmcode::RCX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x4C, 0x24, 0xF8 });
    }

  void vmcode_mov_r14_mem_r13()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::R14, vmcode::MEM_R13);
    code.add(vmcode::MOV, vmcode::R13, vmcode::MEM_R14);
    code.add(vmcode::MOV, vmcode::R13, vmcode::MEM_R13);
    code.add(vmcode::MOV, vmcode::R14, vmcode::MEM_R14);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x75, 0x00 });  //<== BUG: TO FIX
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x2E });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x6D, 0x00 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x36 });
    }

  void vmcode_movq_xmm0_rax()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOVQ, vmcode::XMM0, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x6E, 0xC0 });
    }

  void vmcode_movq_rax_xmm0()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOVQ, vmcode::RAX, vmcode::XMM0);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x7E, 0xC0 });
    }

  void vmcode_mov_rsp_rdx()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RSP, vmcode::RDX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xD4 });
    }

  void vmcode_mov_rbp_r8()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RBP, vmcode::R8);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4C, 0x89, 0xC5 });
    }

  void vmcode_and_rsp_number()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::AND, vmcode::RSP, vmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xE4, 0xF0 });
    }

  void vmcode_and_rsp_number_32()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::AND, vmcode::RSP, vmcode::NUMBER, 0xABCDEF);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xE4, 0xEF, 0xCD, 0xAB, 0x00 });
    }

  void vmcode_and_rbp_number()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::AND, vmcode::RBP, vmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xE5, 0xF0 });
    }

  void vmcode_and_rbp_number_32()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::AND, vmcode::RBP, vmcode::NUMBER, 0xABCDEF);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xE5, 0xEF, 0xCD, 0xAB, 0x00 });
    }




  void vmcode_or_rsp_number()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::OR, vmcode::RSP, vmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xCC, 0xF0 });
    }

  void vmcode_or_rsp_number_32()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::OR, vmcode::RSP, vmcode::NUMBER, 0xABCDEF);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xCC, 0xEF, 0xCD, 0xAB, 0x00 });
    }

  void vmcode_or_rbp_number()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::OR, vmcode::RBP, vmcode::NUMBER, 0xFFFFFFFFFFFFFFF0);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xCD, 0xF0 });
    }

  void vmcode_or_rbp_number_32()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::OR, vmcode::RBP, vmcode::NUMBER, 0xABCDEF);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xCD, 0xEF, 0xCD, 0xAB, 0x00 });
    }

  void vmcode_add_rbp_number_15()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 15);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC5, 0x0F });
    }

  void vmcode_add_rbp_number_4096()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 4096);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xC5, 0x00, 0x10, 0x00, 0x00 });
    }

  void vmcode_sub_rbp_number_15()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::SUB, vmcode::RBP, vmcode::NUMBER, 15);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xED, 0x0F });
    }

  void vmcode_sub_rbp_number_4096()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::SUB, vmcode::RBP, vmcode::NUMBER, 4096);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xED, 0x00, 0x10, 0x00, 0x00 });
    }

  void vmcode_xor_rax_rax()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::XOR, vmcode::RAX, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x31, 0xC0 });
    }

  void vmcode_xor_r14_r14()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::XOR, vmcode::R14, vmcode::R14);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x31, 0xF6 });
    }

  void vmcode_pop_rbp()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::POP, vmcode::RBP);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x5D });
    }

  void vmcode_push_rbp()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::PUSH, vmcode::RBP);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x55 });
    }

  void vmcode_ret()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::RET);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xC3 });
    }

  void vmcode_intro_1()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RBX, vmcode::R9);
    code.add(vmcode::ADD, vmcode::RBX, vmcode::NUMBER, 7);
    code.add(vmcode::AND, vmcode::RBX, vmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    code.add(vmcode::XOR, vmcode::R14, vmcode::R14);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 9843185);
    code.add(vmcode::MOV, vmcode::MEM_RBX, vmcode::RAX);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4C, 0x89, 0xCB });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC3, 0x07 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xE3, 0xF8 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x31, 0xF6 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC7, 0xC0, 0xF1, 0x31, 0x96, 0x00 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x03 });
    }


  void vmcode_store_registers()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_R10, 8, vmcode::RBX);
    code.add(vmcode::MOV, vmcode::MEM_R10, 32, vmcode::RDI);
    code.add(vmcode::MOV, vmcode::MEM_R10, 40, vmcode::RSI);
    code.add(vmcode::MOV, vmcode::MEM_R10, 48, vmcode::RSP);
    code.add(vmcode::MOV, vmcode::MEM_R10, 56, vmcode::RBP);
    code.add(vmcode::MOV, vmcode::MEM_R10, 96, vmcode::R12);
    code.add(vmcode::MOV, vmcode::MEM_R10, 104, vmcode::R13);
    code.add(vmcode::MOV, vmcode::MEM_R10, 112, vmcode::R14);
    code.add(vmcode::MOV, vmcode::MEM_R10, 120, vmcode::R15);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x5A, 0x08 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x7A, 0x20 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x72, 0x28 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x62, 0x30 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x6A, 0x38 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x89, 0x62, 0x60 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x89, 0x6A, 0x68 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x89, 0x72, 0x70 });
    size = code.get_instructions_list().front()[8].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x89, 0x7A, 0x78 });
    }


  void vmcode_load_registers()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RBX, vmcode::MEM_R10, 8);
    code.add(vmcode::MOV, vmcode::RDI, vmcode::MEM_R10, 32);
    code.add(vmcode::MOV, vmcode::RSI, vmcode::MEM_R10, 40);
    code.add(vmcode::MOV, vmcode::RSP, vmcode::MEM_R10, 48);
    code.add(vmcode::MOV, vmcode::RBP, vmcode::MEM_R10, 56);
    code.add(vmcode::MOV, vmcode::R12, vmcode::MEM_R10, 96);
    code.add(vmcode::MOV, vmcode::R13, vmcode::MEM_R10, 104);
    code.add(vmcode::MOV, vmcode::R14, vmcode::MEM_R10, 112);
    code.add(vmcode::MOV, vmcode::R15, vmcode::MEM_R10, 120);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x8B, 0x5A, 0x08 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x8B, 0x7A, 0x20 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x8B, 0x72, 0x28 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x8B, 0x62, 0x30 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x8B, 0x6A, 0x38 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x62, 0x60 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x6A, 0x68 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x72, 0x70 });
    size = code.get_instructions_list().front()[8].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x8B, 0x7A, 0x78 });
    }

  void vmcode_prepare_external_function_call()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::SUB, vmcode::RSP, vmcode::NUMBER, 48);
    code.add(vmcode::PUSH, vmcode::R10);
    code.add(vmcode::PUSH, vmcode::RBP);
    code.add(vmcode::MOV, vmcode::RBP, vmcode::RSP);
    code.add(vmcode::SUB, vmcode::RSP, vmcode::NUMBER, 32 + 8);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xEC, 0x30 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0x52 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x55 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xE5 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xEC, 0x28 });
    }

  void finalize_external_function_call()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::ADD, vmcode::RSP, vmcode::NUMBER, 32 + 8);
    code.add(vmcode::POP, vmcode::RBP);
    code.add(vmcode::POP, vmcode::R10);
    code.add(vmcode::ADD, vmcode::RSP, vmcode::NUMBER, 48);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC4, 0x28 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x5D });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0x5A });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC4, 0x30 });
    }

  void code_get_stack_item()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RSP, -1064);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x84, 0x24, 0xD8, 0xFB, 0xFF, 0xFF });
    }

  void save_rax_on_stack()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RSP, -1064, vmcode::RAX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x84, 0x24, 0xD8, 0xFB, 0xFF, 0xFF });
    }

  void convert_short_string_to_string()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RBP, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RBP);
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 8);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x45, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xE8 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC5, 0x08 });
    }

  void vmcode_call()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CALL, vmcode::RBX);
    code.add(vmcode::CALL, vmcode::MEM_RBX);
    code.add(vmcode::CALL, vmcode::NUMBER, 64);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xFF, 0xD3 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xFF, 0x13 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xE8, 0x40, 0x00, 0x00, 0x00 });
    }

  void vmcode_add_7_and_3()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 7);
    code.add(vmcode::MOV, vmcode::RCX, vmcode::NUMBER, 3);
    code.add(vmcode::ADD, vmcode::RAX, vmcode::RCX);
    code.add(vmcode::RET);
    uint64_t size = 0;
    const auto& commands = code.get_instructions_list().front();
    for (const auto& c : commands)
      size += c.fill_opcode(buffer);

#ifdef _WIN32
    unsigned char* my_func = (unsigned char*)malloc(size);
#else
    unsigned char* my_func = (unsigned char*)mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    unsigned char* ptr = my_func;
    for (const auto& c : commands)
      {
      ptr += c.fill_opcode(ptr);
      }
    unsigned long i;
#ifdef _WIN32
    auto success = VirtualProtect((void *)(my_func), size, 0x40/*PAGE_EXECUTE_READWRITE*/, (unsigned long *)&i);
    TEST_ASSERT(success);
#endif
    typedef uint64_t(*fun_ptr)(void);

    uint64_t res = ((fun_ptr)(my_func))();

    TEST_EQ(res, uint64_t(10));

#ifdef _WIN32
    free(my_func);
#else
    munmap((void*)(my_func), size);
#endif
    }

  void vmcode_add_two_integers()
    {
    vmcode code; uint8_t buffer[255];
#ifdef _WIN32
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RCX); // windows calling convention
    code.add(vmcode::ADD, vmcode::RAX, vmcode::RDX);
#else
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RSI); // linux calling convention
    code.add(vmcode::ADD, vmcode::RAX, vmcode::RDI);
#endif
    code.add(vmcode::RET);
    uint64_t size = 0;
    const auto& commands = code.get_instructions_list().front();
    for (const auto& c : commands)
      size += c.fill_opcode(buffer);

#ifdef _WIN32
    unsigned char* my_func = (unsigned char*)malloc(size);
#else
    unsigned char* my_func = (unsigned char*)mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

    unsigned char* ptr = my_func;
    for (const auto& c : commands)
      {
      ptr += c.fill_opcode(ptr);
      }

    unsigned long i;
#ifdef _WIN32
    auto success = VirtualProtect((void *)(my_func), size, 0x40/*PAGE_EXECUTE_READWRITE*/, (unsigned long *)&i);
    TEST_ASSERT(success);
#endif
    typedef uint64_t(*fun_ptr)(uint64_t a, uint64_t b);

    uint64_t res = ((fun_ptr)(my_func))(3, 7);
    TEST_EQ(res, uint64_t(10));
    res = ((fun_ptr)(my_func))(100, 211);
    TEST_EQ(res, uint64_t(311));

#ifdef _WIN32
    free(my_func);
#else
    munmap((void*)(my_func), size);
#endif
    }

  void vmcode_cmp()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CMP, vmcode::RAX, vmcode::NUMBER, 1024);
    code.add(vmcode::CMP, vmcode::RBX, vmcode::NUMBER, 1024);
    code.add(vmcode::CMP, vmcode::RAX, vmcode::NUMBER, 8);
    code.add(vmcode::CMP, vmcode::RBX, vmcode::NUMBER, 8);
    code.add(vmcode::CMP, vmcode::RAX, vmcode::RCX);
    code.add(vmcode::CMP, vmcode::RBX, vmcode::RCX);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x3D, 0x00, 0x04, 0x00, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x81, 0xFB, 0x00, 0x04, 0x00, 0x00 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xF8, 0x08 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xFB, 0x08 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x39, 0xC8 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x39, 0xCB });
    }


  void vmcode_dec_inc()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::INC, vmcode::RAX);
    code.add(vmcode::INC, vmcode::RBX);
    code.add(vmcode::DEC, vmcode::RCX);
    code.add(vmcode::DEC, vmcode::RDX);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xFF, 0xC0 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xFF, 0xC3 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xFF, 0xC9 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xFF, 0xCA });
    }

  void vmcode_mul()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MUL, vmcode::RCX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xF7, 0xE1 });
    }

  void vmcode_div()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::DIV, vmcode::RCX);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xF7, 0xF1 });
    }

  void vmcode_nop()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::NOP);
    auto first_instruction = code.get_instructions_list().front().front();
    auto size = first_instruction.fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x90 });
    }

  void vmcode_cvt_sd_si()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CVTSI2SD, vmcode::XMM0, vmcode::RAX);
    code.add(vmcode::CVTSI2SD, vmcode::XMM1, vmcode::RAX);
    code.add(vmcode::CVTTSD2SI, vmcode::RAX, vmcode::XMM0);
    code.add(vmcode::CVTTSD2SI, vmcode::RCX, vmcode::XMM1);   

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2A, 0xC0 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2A, 0xC8 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2C, 0xC0 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2C, 0xC9 });
    }

  void vmcode_cmp_bytes()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RCX, vmcode::AL);
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RAX, vmcode::AL);
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RSP, vmcode::AL);

    code.add(vmcode::CMP, vmcode::BYTE_MEM_RCX, 8, vmcode::AL);
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RAX, 8, vmcode::AL);
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RSP, -8, vmcode::AL);

    code.add(vmcode::CMP, vmcode::BYTE_MEM_RCX, vmcode::NUMBER, 0);
    code.add(vmcode::CMP, vmcode::BYTE_MEM_RAX, vmcode::NUMBER, 0);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x01 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x00 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x04, 0x24 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x41, 0x08 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x40, 0x08 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x38, 0x44, 0x24, 0xF8 });

    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x80, 0x39, 0x00 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x80, 0x38, 0x00 });
    }


  void vmcode_mov_bytes()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::AL, vmcode::BYTE_MEM_RDX);
    code.add(vmcode::MOV, vmcode::BYTE_MEM_RDX, vmcode::AL);
    code.add(vmcode::MOV, vmcode::DL, vmcode::BYTE_MEM_RBP);
    code.add(vmcode::MOV, vmcode::BYTE_MEM_RBP, vmcode::DL);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x8A, 0x02 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x88, 0x02 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x8A, 0x55, 0x00 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x88, 0x55, 0x00 });
    }


  void vmcode_sse_basic_ops()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::ADDSD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::SUBSD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::DIVSD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::MULSD, vmcode::XMM0, vmcode::XMM1);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x0F, 0x58, 0xC1 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x0F, 0x5C, 0xC1 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x0F, 0x5E, 0xC1 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x0F, 0x59, 0xC1 });
    }

  void vmcode_sse_cmp_ops()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CMPEQPD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::CMPLTPD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::CMPLEPD, vmcode::XMM0, vmcode::XMM1);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0xC2, 0xC1, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0xC2, 0xC1, 0x01 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0xC2, 0xC1, 0x02 });
    }

  void vmcode_sse_other_ops()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOVMSKPD, vmcode::RAX, vmcode::XMM0);
    code.add(vmcode::SQRTPD, vmcode::XMM1, vmcode::XMM0);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0x50, 0xC0 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0x51, 0xC8 });
    }

  void vmcode_fpu()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::FLD, vmcode::MEM_RSP, -8);
    code.add(vmcode::FLD, vmcode::MEM_RAX, 0);

    code.add(vmcode::FADDP);
    code.add(vmcode::FADD, vmcode::ST0, vmcode::ST5);
    code.add(vmcode::FADD, vmcode::ST5, vmcode::ST0);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xDD, 0x44, 0x24, 0xF8 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xDD, 0x00 });

    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xDE, 0xC1 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xD8, 0xC5 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xDC, 0xC5 });
    }

  void vmcode_string_literals()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 0x6F77206F6C6C6548);
    code.add(vmcode::MOV, vmcode::MEM_RBP, vmcode::RAX);
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 0x0000000000000008);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 0x0000000021646C72);
    code.add(vmcode::MOV, vmcode::MEM_RBP, vmcode::RAX);
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 0x0000000000000008);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RBP);
    code.add(vmcode::SUB, vmcode::RAX, vmcode::NUMBER, 0x0000000000000010);
    code.add(vmcode::MOV, vmcode::MEM_RBX, 8, vmcode::RAX);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xB8, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x45, 0x00 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC5, 0x08 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC7, 0xC0, 0x72, 0x6C, 0x64, 0x21 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x45, 0x00 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC5, 0x08 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xE8 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xE8, 0x10 });
    size = code.get_instructions_list().front()[8].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x43, 0x08 });
    }


  void vmcode_print_statement()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RBX, 8);
    code.add(vmcode::MOV, vmcode::RCX, vmcode::RAX);
    code.add(vmcode::SUB, vmcode::RSP, vmcode::NUMBER, 0);
    code.add(vmcode::PUSH, vmcode::R10);
    code.add(vmcode::PUSH, vmcode::RBP);
    code.add(vmcode::MOV, vmcode::RBP, vmcode::RSP);
    code.add(vmcode::SUB, vmcode::RSP, vmcode::NUMBER, 0x0000000000000028);

    code.add(vmcode::ADD, vmcode::R14, vmcode::RAX);
    code.add(vmcode::XOR, vmcode::R14, vmcode::R14);

    code.add(vmcode::ADD, vmcode::RSP, vmcode::NUMBER, 0);
    code.add(vmcode::POP, vmcode::RBP);
    code.add(vmcode::POP, vmcode::R10);

    uint64_t size;

    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x43, 0x08 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xC1 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xEC, 0x00 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0x52 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x55 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xE5 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xEC, 0x28 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x01, 0xC6 });
    size = code.get_instructions_list().front()[8].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x4D, 0x31, 0xF6 });
    size = code.get_instructions_list().front()[9].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC4, 0x00 });
    size = code.get_instructions_list().front()[10].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x5D });
    size = code.get_instructions_list().front()[11].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0x5A });
    }


  void vmcode_string_literals_tst()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 0x0000000000747374);
    code.add(vmcode::MOV, vmcode::MEM_RBP, 0, vmcode::RAX);
    code.add(vmcode::ADD, vmcode::RBP, vmcode::NUMBER, 0x0000000000000008);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::RBP);
    code.add(vmcode::SUB, vmcode::RAX, vmcode::NUMBER, 0x0000000000000008);
    code.add(vmcode::MOV, vmcode::MEM_RBX, 8, vmcode::RAX);
    code.add(vmcode::RET);

    code.add(vmcode::MOV, vmcode::R10, vmcode::RCX);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC7, 0xC0, 0x74, 0x73, 0x74, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x45, 0x00 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xC5, 0x08 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0xE8 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xE8, 0x08 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x43, 0x08 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xC3 });

    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0xCA });
    }


  void vmcode_eq()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RSP, -8, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RSP, -8);
    code.add(vmcode::MOVQ, vmcode::XMM0, vmcode::RAX);
    code.add(vmcode::MOVQ, vmcode::XMM1, vmcode::RAX);
    code.add(vmcode::CMPEQPD, vmcode::XMM0, vmcode::XMM1);
    code.add(vmcode::MOVMSKPD, vmcode::RAX, vmcode::XMM0);
    code.add(vmcode::CMP, vmcode::RAX, vmcode::NUMBER, 0x0000000000000003);
    code.add(vmcode::MOVMSKPD, vmcode::RAX, vmcode::XMM1);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x44, 0x24, 0xF8 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x44, 0x24, 0xF8 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x6E, 0xC0 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x6E, 0xC8 });
    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0xC2, 0xC1, 0x00 });
    size = code.get_instructions_list().front()[5].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0x50, 0xC0 });
    size = code.get_instructions_list().front()[6].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x83, 0xF8, 0x03 });
    size = code.get_instructions_list().front()[7].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0x50, 0xC1 });

    }

  void vmcode_int()
    {
    vmcode code; uint8_t buffer[255];

    code.add(vmcode::MOVQ, vmcode::XMM0, vmcode::RAX);
    code.add(vmcode::CVTTSD2SI, vmcode::RAX, vmcode::XMM0);
    code.add(vmcode::CVTSI2SD, vmcode::XMM0, vmcode::RAX);
    code.add(vmcode::MOVQ, vmcode::RAX, vmcode::XMM0);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x6E, 0xC0 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2C, 0xC0 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x48, 0x0F, 0x2A, 0xC0 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x48, 0x0F, 0x7E, 0xC0 });
    }

  void vmcode_checks()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::RAX, vmcode::MEM_RBX, 8);
    code.add(vmcode::MOV, vmcode::MEM_R12, vmcode::RAX);
    code.add(vmcode::NEG, vmcode::RAX);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x8B, 0x43, 0x08 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0x89, 0x04, 0x24 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xF7, 0xD8 });
    }

  void vmcode_byte_boundary()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOV, vmcode::MEM_RBX, 128, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::MEM_RBX, -128, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::MEM_RSP, -128, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::MEM_RSP, 128, vmcode::RAX);
    code.add(vmcode::MOV, vmcode::RAX, vmcode::NUMBER, 0x00000000ffffffff);

    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x43, 0x80 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x44, 0x24, 0x80 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x89, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00 });

    size = code.get_instructions_list().front()[4].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 });
    }

 
  void vmcode_and_8bit()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::AND, vmcode::AL, vmcode::NUMBER, 3);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x24, 0x03 });
    }

  void vmcode_cmp_8bit()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CMP, vmcode::AL, vmcode::NUMBER, 3);
    code.add(vmcode::CMP, vmcode::CL, vmcode::NUMBER, 3);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x3c, 0x03 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x80, 0xF9, 0x03 });
    }

  void vmcode_shl_shr_sal_sar()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::SHL, vmcode::RAX, vmcode::NUMBER, 3);
    code.add(vmcode::SAL, vmcode::RAX, vmcode::NUMBER, 3);
    code.add(vmcode::SHR, vmcode::RAX, vmcode::NUMBER, 3);
    code.add(vmcode::SAR, vmcode::RAX, vmcode::NUMBER, 3);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC1, 0xE0, 0x03 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC1, 0xE0, 0x03 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC1, 0xE8, 0x03 });
    size = code.get_instructions_list().front()[3].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xC1, 0xF8, 0x03 });
    }

  void vmcode_sete()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::SETE, vmcode::AL);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x0F, 0x94, 0xC0 });
    }

  void vmcode_movsd()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOVSD, vmcode::XMM0, vmcode::XMM1);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0xF2, 0x0F, 0x11, 0xC8 });
    }

  void vmcode_movzx()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::MOVZX, vmcode::RAX, vmcode::AL);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x0F, 0xB6, 0xC0 });
    }

  void vmcode_idiv()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::IDIV, vmcode::R11);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x49, 0xF7, 0xFB });
    }

  void vmcode_cqo()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::CQO);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x99 });
    }

  void vmcode_jmp()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::JMP, vmcode::R11);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0xFF, 0xE3 });
    }

  void vmcode_or_r15mem()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::OR, vmcode::BYTE_MEM_R15, vmcode::AL);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x41, 0x08, 0x07 });
    }

  void test_test()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::TEST, vmcode::RAX, vmcode::NUMBER, 1);
    code.add(vmcode::TEST, vmcode::RCX, vmcode::NUMBER, 1);
    code.add(vmcode::TEST, vmcode::RCX, vmcode::RDX);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xA9, 0x01, 0x00, 0x00, 0x00 });
    size = code.get_instructions_list().front()[1].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0xF7, 0xC1, 0x01, 0x00, 0x00, 0x00 });
    size = code.get_instructions_list().front()[2].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x48, 0x85, 0xD1 });
    }

  void test_ucomisd()
    {
    vmcode code; uint8_t buffer[255];
    code.add(vmcode::UCOMISD, vmcode::XMM0, vmcode::XMM1);
    uint64_t size;
    size = code.get_instructions_list().front()[0].fill_opcode(buffer);
    _check_buffer(buffer, size, { 0x66, 0x0F, 0x2E, 0xC1 });
    }  

  }

VM_END


void run_all_vmcode_tests()
  {
  using namespace VM;  
  vmcode_mov_rax_number();
  vmcode_mov_rax_number_2();
  vmcode_mov_rbx_number();
  vmcode_mov_r15_number();
  vmcode_mov_rax_rcx();
  vmcode_mov_rcx_rax();
  vmcode_mov_rax_memrsp_8();
  vmcode_mov_rax_memrsp_4096();
  vmcode_mov_memrsp_8_rax();
  vmcode_mov_memrsp_4096_rax();
  vmcode_mov_memrsp_8_rcx();
  vmcode_mov_r14_mem_r13();
  vmcode_movq_xmm0_rax();
  vmcode_movq_rax_xmm0();
  vmcode_mov_rsp_rdx();
  vmcode_mov_rbp_r8();
  vmcode_and_rsp_number();
  vmcode_and_rsp_number_32();
  vmcode_and_rbp_number();
  vmcode_and_rbp_number_32();
  vmcode_or_rsp_number();
  vmcode_or_rsp_number_32();
  vmcode_or_rbp_number();
  vmcode_or_rbp_number_32();
  vmcode_add_rbp_number_15();
  vmcode_add_rbp_number_4096();
  vmcode_sub_rbp_number_15();
  vmcode_sub_rbp_number_4096();
  vmcode_xor_rax_rax();
  vmcode_xor_r14_r14();
  vmcode_pop_rbp();
  vmcode_push_rbp();
  vmcode_ret();
  vmcode_intro_1();
  vmcode_store_registers();
  vmcode_load_registers();
  vmcode_prepare_external_function_call();
  finalize_external_function_call();
  code_get_stack_item();
  save_rax_on_stack();
  convert_short_string_to_string();
  vmcode_call();
#ifndef _SKIWI_FOR_ARM
  vmcode_add_7_and_3();
  vmcode_add_two_integers();
#endif
  vmcode_cmp();
  vmcode_dec_inc();
  vmcode_mul();
  vmcode_div();
  vmcode_nop();
  vmcode_cvt_sd_si();
  vmcode_cmp_bytes();
  vmcode_mov_bytes();
  vmcode_sse_basic_ops();
  vmcode_sse_cmp_ops();
  vmcode_sse_other_ops();
  vmcode_fpu();
  vmcode_string_literals();
  vmcode_print_statement();
  vmcode_string_literals_tst();
  vmcode_eq();
  vmcode_int();
  vmcode_checks();
  vmcode_byte_boundary();  
  vmcode_and_8bit();
  vmcode_cmp_8bit();
  vmcode_shl_shr_sal_sar();
  vmcode_sete();
  vmcode_movsd();
  vmcode_movzx();
  vmcode_idiv();
  vmcode_cqo();
  vmcode_jmp();
  vmcode_or_r15mem();
  test_test();
  test_ucomisd();
  }

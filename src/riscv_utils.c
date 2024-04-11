#include "riscv_utils.h"

inline uint32_t rd_get(uint32_t instruction)
{
	return (instruction & 0xF80) >> 7;
}

inline uint32_t funct3_get(uint32_t instruction)
{
	return (instruction & 0x7000) >> 12;
}

inline uint32_t rs1_get(uint32_t instruction)
{
	return (instruction & 0xf8000) >> 15;
}

inline uint32_t rs2_get(uint32_t instruction)
{
	return (instruction & 0x1f00000) >> 20;
}

inline uint32_t funct7_get(uint32_t instruction)
{
	return (instruction & 0xfe000000) >> 25;
}

inline uint32_t bigImm_get(uint32_t instruction)
{
	return (instruction & 0xfff00000) >> 20;
}

inline uint32_t opcode_get(uint32_t instruction)
{
	return instruction & 0x7f;
}

inline bool is_control(uint32_t inst)
{
	int opcode = opcode_get(inst);
	return opcode == 103 || opcode == 111 || opcode == 99;
}
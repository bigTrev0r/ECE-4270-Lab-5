#include <stdio.h>

#include "riscv_utils.h"

inline int32_t _bit2_cast(uint32_t imm)
{
	int8_t lo = (int8_t)imm & 0xff;
	int8_t hi = (int8_t)(imm & 0xf00 >> 8);
	int8_t mask = (int32_t)(lo + (hi << 8));
	return mask;
}

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

inline uint32_t get_b_imm(uint32_t instruction)
{
	uint32_t imm4_1 = (instruction >> 7) & 0x1e;
	uint32_t imm10_5 = (instruction >> 25) & 0x3f;
	uint32_t imm11 = (instruction >> 7) & 1;
	uint32_t imm12 = (instruction >> 25) & 0x40;
	uint32_t imm = (imm12 << 11) | (imm11 << 10) | (imm10_5 << 4) | imm4_1;
	return(imm);
}

inline uint32_t get_j_imm(uint32_t instruction)
{
	uint32_t imm19_12 = (instruction>>12) & 0xff;
	uint32_t imm11 = (instruction >> 20) & 1;
	uint32_t imm10_1 = (instruction >> 21) & 0x3ff;
	uint32_t imm20 = (instruction >> 31) & 1;
	return (imm20<<19) | (imm19_12 << 11) | (imm11 << 10) | imm10_1;
}

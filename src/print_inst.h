#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "riscv_utils.h"

char* inst_to_string(uint32_t);
char* R_print(uint32_t rd, uint32_t f3, uint32_t rs1,uint32_t rs2,uint32_t f7);
char* ILoad_print(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm);
char* Iimm_print(uint32_t opcode, uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm);
char* S_print(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm11);
char* B_print(uint32_t funct3, uint32_t rs1, uint32_t rs2, uint32_t imm);
char* J_print(uint32_t rd,uint32_t imm);

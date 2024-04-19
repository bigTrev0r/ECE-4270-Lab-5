#include <stdint.h>
#include <stdbool.h>

int32_t _bit2_cast(uint32_t);
uint32_t rd_get(uint32_t);
uint32_t funct3_get(uint32_t);
uint32_t rs1_get(uint32_t);
uint32_t rs2_get(uint32_t);
uint32_t funct7_get(uint32_t);
uint32_t bigImm_get(uint32_t);
uint32_t opcode_get(uint32_t);
uint32_t get_b_imm(uint32_t);
uint32_t get_j_imm(uint32_t);
bool is_control(uint32_t);

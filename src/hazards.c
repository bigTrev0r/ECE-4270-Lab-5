#include <stdbool.h>
#include <stdint.h>
#include "riscv_utils.h"

// return true if instruction is type R or I
bool regWrite(uint32_t inst){
    uint32_t opcode = opcode_get(inst);
    return opcode == 0x33 || opcode == 0x03 || opcode == 0x13;
}

//these functions act as the forwarding unit.

//Checks for hazards with ALU operand A.
//return values: 0=no hazard; 1=MEM hazard, 2=EX hazard
uint8_t forward_unit_A(uint32_t rs1, uint32_t rs2, uint32_t ex_mem_rd, uint32_t mem_wb_rd){
    // check if the current instruction uses a register that is written to by the instructions in the EX or MEM stages in the current clock cycle.
	if (ex_mem_rd && (ex_mem_rd == rs1)) {return 2;} //hazard case 1a; EX hazard
	if (mem_wb_rd && (mem_wb_rd == rs1)) {return 1;} //hazard case 2a; MEM hazard
    return 0; //no hazard in ALU operand A
}

//Checks for hazards with ALU operand B
//return values: 0=no hazard; 1=MEM hazard, 2=EX hazard
uint8_t forward_unit_B(uint32_t rs1, uint32_t rs2, uint32_t ex_mem_rd, uint32_t mem_wb_rd){
    if (ex_mem_rd && (ex_mem_rd== rs2)) {return 2;} //hazard case 1b; EX hazard
    if (mem_wb_rd && (mem_wb_rd == rs2)) {return 1;} //hazard case 2b; MEM hazard
    return 0; //no hazard
}
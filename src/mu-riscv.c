#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "mu-riscv.h"
#include "riscv_utils.h"
#include "print_inst.h"
#include "hazards.c"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-RISCV Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("forwarding\t-- toggle forwarding\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {
	//printf("Cycle count: %d\n", CYCLE_COUNT);;
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
	//if(CURRENT_STATE.PC > (PROGRAM_SIZE * 4) + MEM_TEXT_BEGIN) RUN_FLAG = false;  //this line would end the program before the final instruction finished
}

/***************************************************************/
/* Simulate RISCV for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {

	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */
/***************************************************************/
void rdump() {
	int i;
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x   [%d]\n", i, CURRENT_STATE.REGS[i], CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */
/***************************************************************/
void handle_command() {
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-RISCV SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll();
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-RISCV! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value;
			NEXT_STATE.HI = hi_reg_value;
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program();
			break;
		case 'F':
		case 'f':
			ENABLE_FORWARDING ^= true;
			ENABLE_FORWARDING ? printf("Forwarding ON\n") : printf("Forwarding OFF\n");
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;

	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}

	/*load program*/
	load_program();

	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

#define R_ARGS uint32_t rs1, uint32_t rs2
#define I_ARGS uint32_t rs1, int32_t imm
#define S_ARGS uint32_t rs1, int32_t imm
#define B_ARGS uint32_t rs1, uint32_t rs2
#define J_ARGS uint32_t temp_pc

//***************** R TYPE INSTRUCTIONS **********************
static inline uint32_t ADD(R_ARGS){return rs1 + rs2;}
static inline uint32_t SUB(R_ARGS){return rs1 - rs2;}
static inline uint32_t XOR(R_ARGS){return rs1 ^ rs2;}
static inline uint32_t  OR(R_ARGS){return rs1 | rs2;}
static inline uint32_t AND(R_ARGS){return rs1 & rs2;}
static inline uint32_t SLL(R_ARGS){return rs1 << rs2;}
static inline uint32_t SRL(R_ARGS){return rs1 >> rs2;}
static inline uint32_t SRA(R_ARGS){return rs1 >> rs2;} //TODO: this is actually supposed to extend with the msb, I'll leave it unimplemened for now, but plan to do this later -Trevor
static inline uint32_t SLT(R_ARGS){return (rs1 < rs2);}
static inline uint32_t SLU(R_ARGS){return (rs1 < rs2);}//TODO: zero extends, leaving for now similar to last one. I figure these little things can be one of the last things we do - Trevor

//**************** I IMMEDIATE INSTRUCTIONS *****************
static inline uint32_t ADDI(I_ARGS){return rs1 + imm;}
static inline uint32_t XORI(I_ARGS){return rs1 ^ imm;}
static inline uint32_t 	ORI(I_ARGS){return rs1 | imm;}
static inline uint32_t ANDI(I_ARGS){return rs1 & imm;}
static inline uint32_t SLLI(I_ARGS){return rs1 << imm;}
static inline uint32_t SRLI(I_ARGS){return rs1 >> imm;}
static inline uint32_t SRAI(I_ARGS){return rs1 >> imm;}//TODO: msb extends, also note to self to be careful with the upper immediate bits, might have to mess with those in the function, we'll see. it would make it so much less clean though TwT ...
static inline uint32_t SLTI(I_ARGS){return rs1 < imm;}
static inline uint32_t SLTIU(I_ARGS){return rs1 < imm;}//TODO: zero extends


//**************** LOAD INSTRUCTIONS ************************
static inline uint32_t LOAD_GENERAL(I_ARGS){return rs1 + imm;}

//**************** STORE INSTRUCTIONS ***********************
static inline uint32_t STORE_GENERAL(S_ARGS){return rs1 + imm;}

//*************** BRANCH INSTRUCTIONS ***********************
static inline uint32_t BEQ(B_ARGS) {return (rs1 == rs2);}
static inline uint32_t BNE(B_ARGS) {return rs1 != rs2;}
static inline uint32_t BLT(B_ARGS) {return (int32_t)rs1 < (int32_t)rs2;}
static inline uint32_t BGE(B_ARGS) {return (int32_t)rs1 >= (int32_t)rs2;}
static inline uint32_t BLTU(B_ARGS) {return rs1 < rs2;}
static inline uint32_t BGEU(B_ARGS) {return rs1 >= rs2;}

//*************** JUMP INSTRUCTIONS *************************
static inline uint32_t JAL(uint32_t pc) {return pc+EX_MEM.imm;}
static inline uint32_t JALR(I_ARGS) {return rs1 + imm;}

//*************** INSTRUCTION TABLES ************************
static uint32_t (*R_MAP[10])(R_ARGS) = {ADD,SUB,SLT,SLU,XOR,SRL,SRA,OR,AND};
static uint32_t (*IIMM_MAP[9])(I_ARGS) = {ADDI,SLLI,SLTI,SLTIU,XORI,SRLI,SRAI,ORI,ANDI};
static uint32_t (*BRANCH_MAP[6])(B_ARGS) = {BEQ,BNE,BLT,BGE,BLTU,BGEU};

//*************** HANDLERS **********************************
static uint32_t r_handler(uint32_t funct3, uint32_t funct7){return R_MAP[funct3 + (funct3>5) + (funct7 >> 6)](EX_MEM.A,EX_MEM.B);}
static uint32_t iImm_handler(uint32_t funct3)
{
	uint8_t offset = ( (funct3 == 5) * (EX_MEM.imm >> 5)  ); //this is only here to handle srai
	//return IIMM_MAP[funct3 + (funct3 > 5) + offset](EX_MEM.A,EX_MEM.imm << 5 | EX_MEM.B); // ?
	return IIMM_MAP[funct3 + (funct3 > 5) + offset](EX_MEM.A,EX_MEM.imm);
}
static uint32_t iL_handler(){return LOAD_GENERAL(EX_MEM.A,EX_MEM.imm);}
static uint32_t s_handler(){return STORE_GENERAL(EX_MEM.A,EX_MEM.imm);}

static uint32_t b_handler(uint32_t funct3)
{
	uint32_t offset = (funct3 > 4) * 3;
	uint32_t branch = BRANCH_MAP[funct3 - offset](EX_MEM.A,EX_MEM.B);
	return CURRENT_STATE.PC + (EX_MEM.imm * branch);
}

/************************************************************/
/* maintain the pipeline                                                                                           */
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */

	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */
/************************************************************/
void WB()
{
	int lmd = MEM_WB.LMD;
	int alu = MEM_WB.ALUOutput;
	int inst = MEM_WB.IR;
	if(inst){ // do nothing if there is no instruction
		int opcode = opcode_get(inst);
		int rd = rd_get(inst); //destination register

		switch(opcode){
			case 51:{ //register-register instruction
				NEXT_STATE.REGS[rd] = alu;
				break;
			}
			case 19:{ //register-immediate instruction
				NEXT_STATE.REGS[rd] = alu;
				break;
			}
			case 3:{ //load instruction
				NEXT_STATE.REGS[rd] = lmd;
				break;
			}
		}

		INSTRUCTION_COUNT++;
		if(INSTRUCTION_COUNT >= PROGRAM_SIZE) RUN_FLAG = FALSE;
	}
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */
/************************************************************/
void MEM()
{
	MEM_WB = EX_MEM;

	//look in IR register to determine if instruction is load or store
	switch(opcode_get(EX_MEM.IR)){
		case 3:{ //Load
			//load: store mem[ALU output] in MEM_WB.LMD register
			MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);
			break;
		}
		case 0x23:{ //Store
			mem_write_32(EX_MEM.ALUOutput,  EX_MEM.B);
			break;
		}
	}
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */
/************************************************************/
void EX()
{
	uint32_t opcode = opcode_get(ID_EX.IR),funct3 = funct3_get(ID_EX.IR), funct7 = funct7_get(ID_EX.IR);
	uint32_t rs1 = ID_EX.rs1, rs2 = ID_EX.rs2;
	uint32_t A,B;

	//detect hazards
	// get forwarding control values from forwarding unit
	CURRENT_STATE.forwardA = forward_unit_A(rs1, rs2, EX_MEM.rd, MEM_WB.rd);
	CURRENT_STATE.forwardB = forward_unit_B(rs1, rs2, EX_MEM.rd, MEM_WB.rd);

	switch(CURRENT_STATE.forwardA){
		case 1:
			if (debug) printf("MEM hazard in operand A\n");
			break;
		case 2:
			if (debug) printf("EX hazard in operand A\n");
			break;
	}
	switch(CURRENT_STATE.forwardB){
		case 1:
			if (debug) printf("MEM hazard in operand B\n");
			break;
		case 2:
			if (debug) printf("EX hazard in operand B\n");
			break;
	}

	if(ENABLE_FORWARDING){	
		//retrieve values for A and B from multiplexors depending on if forwarding is necessary
		A = forward_mux_A(CURRENT_STATE.forwardA);
		B = forward_mux_B(CURRENT_STATE.forwardB);

		if (debug) printf("A: %d   B: %d \n", A, B);
	} else {
		A = CURRENT_STATE.REGS[rs1]; 
		B = CURRENT_STATE.REGS[rs2];
	}

	ID_EX.A = A;
	ID_EX.B = B;
	EX_MEM = ID_EX; //it's important to forward before overwriting the EX_MEM register from the previous clock cycle


	switch(opcode)
	{
		case(0x03):
			EX_MEM.ALUOutput = iL_handler();
			break;
		case(0x13):
			EX_MEM.ALUOutput = iImm_handler(funct3);
			break;
		case(0x23):
			EX_MEM.ALUOutput = s_handler();
			break;
		case(0x33): //register-register
			EX_MEM.ALUOutput = r_handler(funct3, funct7);
			break;
		case(0x63):
			EX_MEM.ALUOutput = b_handler(funct3);
			break;
		case(0x67):
			EX_MEM.ALUOutput = iImm_handler(funct3);
			break;
		case(0x6f):
			EX_MEM.ALUOutput = JAL(CURRENT_STATE.PC);
			break;
		default:
			break;
	}

	// since there are no branch operations yet, we always increment the PC by 4.
	// when we implement branches, we will use logic in the EX stage to determine NEXT_STATE.PC's value.
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;

}

uint32_t forward_mux_A(uint8_t control){
	switch(control) {
		case 0:
			return CURRENT_STATE.REGS[ID_EX.rs1];
		case 2:
			if (debug) printf("Retrieving A from ALUOutput.\n");
			return EX_MEM.ALUOutput;
		case 1:
			if (debug) printf("Retrieving A from LMD.\n");
			return MEM_WB.LMD;
	}
	return INT32_MAX;
}

uint32_t forward_mux_B(uint8_t control){
	switch(control) {
		case 0:
			return CURRENT_STATE.REGS[ID_EX.rs2];
		case 2:
			if (debug) printf("Retrieving B from ALUOutput.\n");
			return EX_MEM.ALUOutput;
		case 1:
			if (debug) printf("Retrieving B from LMD.\n");
			return MEM_WB.LMD;
	}
	return INT32_MAX;
	
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */
/************************************************************/

void ID()
{
	ID_EX = IF_ID;
	
	uint32_t temp_inst = IF_ID.IR;
	uint32_t rs1 = rs1_get(temp_inst);
	uint32_t rs2 = rs2_get(temp_inst);
	
	ID_EX.rs1 = rs1;
	ID_EX.rs2 = rs2;
	ID_EX.RegWrite = regWrite(temp_inst);
	if(ID_EX.RegWrite) ID_EX.rd = rd_get(temp_inst);
	else ID_EX.rd = 0;
	ID_EX.imm = bigImm_get(temp_inst);
	
	uint32_t temp_reg;

	if( !ENABLE_FORWARDING && 
			((temp_reg = rd_get(EX_MEM.IR)) || (temp_reg = rd_get(MEM_WB.IR))) && (temp_reg == rs1 || temp_reg == rs2))
	{
		ID_EX = ZERO;
		NEXT_STATE.PC -= 4;
		CURRENT_STATE.PC -= 4;
	}


}



/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */
/************************************************************/
void IF()
{
	IF_ID.PC = CURRENT_STATE.PC;	
	IF_ID.IR = mem_read_32(IF_ID.PC);

}


/************************************************************/
/* Initialize Memory                                                                                                    */
/************************************************************/
void initialize() {
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in RISCV assembly format)    */
/************************************************************/
void print_program(){
	uint32_t temp_pc = MEM_TEXT_BEGIN, i = 0;

	while(i < PROGRAM_SIZE){
		uint32_t instruction = mem_read_32(temp_pc);
		printf("%s\n", inst_to_string(instruction));
		temp_pc += 4;
		i++;
		//exit loop at some point
	}
}

/************************************************************/
/* Print the current pipeline                                                                                    */
/************************************************************/
void show_pipeline(){
		printf("Current PC: %d\n\
IF/ID.IR: %s\n\
IF/ID.PC: %d\n\n\
ID/EX.IR: %s \n\
ID/EX.A: %d\n\
ID/EX.B: %d\n\
ID/EX.imm: %d\n\n\
EX/MEM.IR: %s\n\
EX/MEM.A: %d\n\
EX/MEM.B: %d\n\
EX/MEM.ALU: %d\n\n\
MEM/WB.IR: %s\n\
MEM/WB.ALUOutput: %d\n\
MEM/WB.LMD: %x\n",
	CURRENT_STATE.PC, inst_to_string(IF_ID.IR), IF_ID.PC, inst_to_string(ID_EX.IR), ID_EX.A, ID_EX.B, ID_EX.imm,
	inst_to_string(EX_MEM.IR), EX_MEM.A, EX_MEM.B, EX_MEM.ALUOutput, inst_to_string(MEM_WB.IR), MEM_WB.ALUOutput, MEM_WB.LMD);
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {
	printf("\n**************************\n");
	printf("Welcome to MU-RISCV SIM...\n");
	printf("**************************\n\n");

	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}

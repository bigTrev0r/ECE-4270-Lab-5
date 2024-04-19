#include "print_inst.h"
#include "riscv_utils.h"

#include <string.h>


int digits(int number) {
   	return number ? (int)(log10(number) + 1) : 0;
}



char * inst_to_string(uint32_t args){
	uint8_t type = (uint8_t)(args & 0x7f);
	switch(type)
	{
		case(0x03): //IL
		{
			return ILoad_print(rd_get(args), funct3_get(args), rs1_get(args), bigImm_get(args));
		}
		case(0x13): //Iimm
		{
			return Iimm_print(opcode_get(args),rd_get(args) , funct3_get(args) , rs1_get(args) , bigImm_get(args));
		}
		case(0x23): //S 
		{
			return S_print(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args));
		}
		case(0x33): //R
		{
			return R_print(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args));
		}
		case(99): //B
		{
			return B_print(funct3_get(args),rs1_get(args),rs2_get(args),get_b_imm(args));
		}
		case(103): //jump
		{
			return Iimm_print(opcode_get(args), rd_get(args) , funct3_get(args) , rs1_get(args) , bigImm_get(args));
		}
		case(111):
		{
			return J_print(rd_get(args),get_j_imm(args));
		}
		default:
			return 0;

	}
}

char* R_print(uint32_t rd, uint32_t f3, uint32_t rs1,uint32_t rs2,uint32_t f7)
{
	char * arg_string;
	switch(f3){
		case 0:
			switch(f7){
				case 0:		//add
					arg_string = "add";
					break;
				case 32:	//sub
					arg_string = "sub";
					break;
				default:
					return 0;
				}	
			break;
		case 1:				//left shift logical
			arg_string = "sll";
			break;
		case 2:				//set less than
			arg_string = "slt";
			break;
		case 3:				//set less than unsigned
			arg_string = "sltu";
			break;
		case 4:				//xor
			arg_string = "xor";
			break;
		case 5:
			switch(f7){
				case 0:		//right shift logical
					arg_string = "srl";
					break;
				case 32:	//right shift arithmetic
					arg_string = "sra";
					break;
			}


		case 6:				//or
			arg_string = "or";
			break;
		case 7:				//and
			arg_string = "and";
			break;
		default:
			return 0;
	}
    char* inst = malloc(sizeof(char) * (strlen(arg_string) + digits(rd) + digits(rs1) + digits(rs2) + 7));
    // strcat(inst, arg_string);
    // strcat(inst, " x");
    // strcat(inst, itoa(rd));
    // strcat(inst, " x");
    // strcat(inst, itoa(rs1));
    // strcat(inst, " x");
    // strcat(inst, itoa(rs2));
	sprintf(inst, "%s x%u x%u x%u",arg_string,rd,rs1,rs2);
    return inst;
	
}

char* ILoad_print(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	char * arg_string;
	switch (f3)
	{
	case 0: //lb
		arg_string = "lb";
		break;
	case 1: //lh
		arg_string = "lh";
		break;
	case 2: //lw
		arg_string = "lw";
		break;
	case 4:
        arg_string = "lbu";
		break;
	case 5:
        arg_string = "lhu";
		break;
	default:
		return 0;
	}
    char* inst = malloc(sizeof(char) * (strlen(arg_string) + digits(rd) + digits(imm) + digits(rs1) + 7));
    // strcat(inst, arg_string);
    // strcat(inst, " x");
    // strcat(inst, itoa(rd));
    // strcat(inst, " ");
    // strcat(inst, itoa(imm));
    // strcat(inst, "(x");
    // strcat(inst, itoa(rs1));
    // strcat(inst, ")");
    sprintf(inst, "%s x%u %u(x%u)",arg_string,rd,imm,rs1);
    return inst;
}

char* Iimm_print(uint32_t opcode, uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm)
{
	uint32_t imm0_4 = (imm << 7) >> 7;
	uint32_t imm5_11 = imm >> 5;

	char * arg_string;
	switch (f3)
	{
	case 0: //addi
		arg_string = "addi";
		if(opcode == 0x63) arg_string = "jalr";
		break;

	case 4: //xori
		arg_string = "xori";
		break;
	
	case 6: //ori
		arg_string = "ori";
		break;
	
	case 7: //andi
		arg_string = "andi";
		break;
	
	case 1: //slli
		arg_string = "slli";
		break;
	
	case 5: //srli and srai
		switch (imm5_11)
		{
		case 0: //srli
			arg_string = "srli";
			break;

		case 32: //srai
			arg_string = "srai";
			break;
		
		default:
			return 0;
		}
		break;
	
	case 2:		//slti
		arg_string = "slti";
		break;

	case 3:		//sktiu
		arg_string = "sltiu";
		break;

	default:
		return 0;
	}

    char* inst = malloc(sizeof(char) * (strlen(arg_string) + digits(rd) + digits(rs1) + digits(imm0_4) + 6));
    // strcat(inst, arg_string);
    // strcat(inst, " x");
    // strcat(inst, itoa(rd));
    // strcat(inst, " x");
    // strcat(inst, itoa(rs1));
    // strcat(inst, " ");
    // strcat(inst, itoa(imm0_4));
    sprintf(inst, "%s x%u x%u %u",arg_string,rd,rs1,imm0_4);
    return inst;
}

char* S_print(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm11) {
	// Recombine immediate
	uint32_t imm = (imm11 << 5) + imm4;
	char * arg_string;

	switch (f3)
	{
	case 0: //sb
		arg_string = "sb";
		break;
	
	case 1: //sh
		arg_string = "sh";
		break;

	case 2: //sw
		arg_string = "sw";
		break;

	default:
		return 0;
	}

    char* inst = malloc(sizeof(char) * (strlen(arg_string) + digits(rs2) + digits(imm) + digits(rs1) + 7));
    // strcat(inst, arg_string);
    // strcat(inst, " x");
    // strcat(inst, itoa(rs2));
    // strcat(inst, " ");
    // strcat(inst, itoa(imm));
    // strcat(inst, "(x");
    // strcat(inst, itoa(rs1));
    // strcat(inst, ")");

    sprintf(inst, "%s x%u %u(x%u)",arg_string,rs2,imm,rs1);
    return inst;
}

const char * b_argname_map[6] = {"beq","bne","blt","bge","bltu","bgeu"};

char * B_print(uint32_t funct3, uint32_t rs1, uint32_t rs2, uint32_t imm)
{
	imm = _bit2_cast(imm);
	int32_t offset = (funct3 >= 4) << 1;
	char * arg_string = strdup(b_argname_map[funct3 - offset]);
	char * inst = malloc(255);
	sprintf(inst,"%s x%u, x%u, %d",arg_string,rs1,rs2,imm);

	free(arg_string);
	return inst;
}

char* J_print(uint32_t rd, uint32_t imm){
	imm = _bit2_cast(imm);
	char * arg_string;
	
	arg_string = "jal";

	char * inst = malloc(255);
	sprintf(inst, "%s x%u, %d",arg_string,rd,imm); 
	
	return inst;

}

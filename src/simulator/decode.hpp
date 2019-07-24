#ifndef DECODE_HPP
#define DECODE_HPP

void decode_binary_input(int binary);
void r_type(int rs, int rt, int rd, int shamt, int funct);
void i_type(int opcode, int rs, int rt, int16_t immediate);
void j_type(int opcode, int target);

#endif
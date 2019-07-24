#include <vector>
#include <cstdint>
#include <stdlib.h>

#include "registers.hpp"

// constructor
Register::Register(){
    PC = 0; //initialised to 0
    registers.resize(32);
    registers[0] = 0; // setting $zero to 0
}

// read (return) register value
uint32_t Register::read_register(uint32_t index)const{
    return registers[index];
}

// write (set) into register
void Register::write_register(uint32_t index, uint32_t input){
    if(index != 0){
        registers[index] = input;
    }
}

// read program counter
uint32_t Register::read_PC(){
    return PC;
}

// update program counter
void Register::update_4_PC(){
    PC += 4;
}

// update program counter in case of branches/jumps
void Register::update_address_PC(uint32_t next_address){ //next_address is the PC's next value
    PC = next_address;
}

// read instruction register
uint32_t Register::read_IR(){
    return IR;
}

// write to instruction register
void Register::write_IR(uint32_t instruction){
    IR = instruction;
}

// high and low registers

// read HI register
uint32_t Register::read_HI(){
    return HI;
}

// read LO register
uint32_t Register::read_LO(){
    return LO;
}

// write HI register
void Register::write_HI(uint32_t input){
    HI = input;
}

// write LO register
void Register::write_LO(uint32_t input){
    LO = input;
}

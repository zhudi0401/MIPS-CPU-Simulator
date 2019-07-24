#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <arpa/inet.h>
#include <stdlib.h>

#include "memory_map.hpp"
#include "registers.hpp"
#include "decode.hpp"
#include "global.hpp"

// creates global memory and register objects
Memory mips_memory;
Register mips_register;

int main(int argc, char* argv[]){
    //load binary into ADDR_INSTR
    int word;
    std::fstream file;
    file.open(argv[1], std::ios::in | std::ios::binary);
    int number_of_instructions = 0;

    while(true){
        file.seekg(0, std::ios::cur);
        file.read(reinterpret_cast<char*>(&word), 4);
        if(file.eof()) break;
        if(number_of_instructions >= 4194304) break;
        mips_memory.load_ADDR_INSTR(number_of_instructions, ntohl(word));
        number_of_instructions++;
    }
    file.close();

    // loop performs all of the instructions in instruction memory
    while(true){
        if(mips_register.read_PC() >= 0x1000000/4){
            exit(-11);
        }
        // divide PC by 4 to get instruction memory index and load that to IR
        mips_register.write_IR(mips_memory.read_ADDR_INSTR(mips_register.read_PC()/4));
        decode_binary_input(mips_register.read_IR());
        // set flags to false after every instruction
        mips_memory.set_read_false();
        mips_memory.set_write_false();
        mips_memory.set_execute_false();
    }
}

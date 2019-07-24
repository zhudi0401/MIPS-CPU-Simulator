#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "memory_map.hpp"
#include "global.hpp"

// constructor: initialises flags and determines ADDR/DATA memory sizes
Memory::Memory(){
    ADDR_INSTR.resize(4194304);
    ADDR_DATA.resize(16777216);
    read = false;
    write = false;
    execute = false;
}

// read from memory
int Memory::read_ADDR_INSTR(int index){
    return ADDR_INSTR[index];
}

int Memory::read_ADDR_DATA(int index){
    return ADDR_DATA[index];
}

// get user input by reading from this address in memory
uint32_t Memory::read_ADDR_GETC(){
    // take one character from stdin
    char c;
    c = getchar();
    if(std::cin.eof()){ // if end of line 
      return 0xFFFFFFFF;
    }
    else{
      return c; 
    }
}

// write to data memory: takes in data and index in DATA_mem vector
void Memory::write_ADDR_DATA(int data_in, int index){
    ADDR_DATA[index] = data_in;
}

// outputs a char using putchar: data outputed is passed through data_in
void Memory::write_ADDR_PUTC(int data_in){
  // If the write fails, the appropriate Error should be signalled.
  // will this error code work? seems sketchy to Oliver...
  if(data_in == 0){
    return;
  }
  else if(putchar(data_in)){
      return;
  }
  else{
    exit(-21);
  }
}

//write instrucitons to INSTR memory: only used when loading binary
void Memory::load_ADDR_INSTR(int index, int number){
    // to write to executable memory
    ADDR_INSTR[index] = number;
}

// Set memory legality flags to true
void Memory::set_read_true(){
  read = true;
}

void Memory::set_write_true(){
  write = true;
}

void Memory::set_execute_true(){
  execute = true;
}

// Read memory legality flags
bool Memory::get_read(){
  return read;
}

bool Memory::get_write(){
  return write;
}

bool Memory::get_execute(){
  return execute;
}

// Set memory legality flags to false
void Memory::set_read_false(){
  read = false;
}

void Memory::set_write_false(){
  write = false;
}

void Memory::set_execute_false(){
  execute = false;
}

// used to find correct memory address and determine if a certain instruction is legal in that region of memory
uint32_t Memory::access(int32_t offset, int data, int opcode){

    // offset out of range
    if(offset < 0x0 || offset > 0x30000008){
        exit(-11);
    }

    // exit program when accessed
    else if(offset == 0x0){
      if(!read && !write && execute){
        exit( mips_register.read_register(2));
      }
      else{
        exit(-11);
      }
    }

    // unused memory: memory error is accessed
    else if(offset >= 0x4 && offset < 0x4 + 0xFFFFFFC){
      exit(-11); // is this error?
    }

    // read from INSTR memory: error if trying to write
    else if(offset >= 0x10000000 && offset < 0x10000000 + 0x1000000){
      offset = offset  & 0xFFFFFFFC;
      if(write){
        exit(-11);
      }
      else if(read){
        return read_ADDR_INSTR((offset - 0x10000000)/4);
      }
      else if(execute){
        return (offset - 0x10000000);
      }
      else{
        exit(-11);
      }
    }

    // unused memory: memory error is accessed
    else if(offset >= 0x11000000 && offset < 0x11000000 + 0xF000000){
      exit(-11); // is this error?
    }

    // read/write from/to DATA memory: error if trying to execute
    else if(offset >= 0x20000000 && offset < (0x20000000 +  0x4000000)){
      offset = offset  & 0xFFFFFFFC;
      if(read && !write && !execute){
        return read_ADDR_DATA((offset - 0x20000000)/4);
      }
      else if(write && !read && !execute){
        write_ADDR_DATA(data, (offset - 0x20000000)/4);
      }
      else{
        exit(-11);
      }
    }

    // unused memory: memory error is accessed
    else if(offset >= 0x24000000 && offset < 0x24000000 + 0xC000000){
      exit(-11); // is this error?
    }

    // get user input
    else if(offset >= 0x30000000 && offset <= 0x30000000 + 0x3){
      if(write || execute){
        exit(-11);
      }
      else if(read){
        //LW and LWL/LWRcase
        if((opcode == 0b100011) || (opcode == 0b100010) || (opcode == 0b100110)){
          uint32_t x;
          x = read_ADDR_GETC();
          if(x == 0xFFFFFFFF){
            return -1;
          }
          else{
            return x;
          }
        }
        //LB and LBU cases
        else if(((opcode == 0b100000) || (opcode == 0b100100)) && (offset == 0x30000003)){
          uint32_t x;
          x = read_ADDR_GETC();
          if(x == 0xFFFFFFFF){
            return -1;
          }
          else{
            return x;
          }
        }
        else if(((opcode == 0b100000) || (opcode == 0b100100)) && (offset != 0x30000003)){
          read_ADDR_GETC();
          return 0;
        }
        //LH and LHU cases
        else if(((opcode == 0b100101) || (opcode == 0b100001)) && (offset == 0x30000002)){
          uint32_t x;
          x = read_ADDR_GETC();
          if(x == 0xFFFFFFFF){
            return -1;
          }
          else{
            return x;
          }
        }
        else if(((opcode == 0b100101) || (opcode == 0b100001)) && (offset != 0x30000002)){
          read_ADDR_GETC();
          return 0;           
        }
      }
      else{
        exit(-20);
      }
    }

    //write to output
    else if(offset >= 0x30000004 && offset <= 0x30000004 + 0x3){
      if(read || execute){
        exit(-11);
      }
      else if(write){
        if(opcode == 0b101011){
          write_ADDR_PUTC(data & 0xFF);
        }
        //SB case
        else if((opcode == 0b101000) && (offset == 0x30000007)){
          write_ADDR_PUTC(data & 0xFF);
        }
        else if((opcode == 0b101000) && (offset != 0x30000007)){
          write_ADDR_PUTC(0);
        }
        //SH case
        else if((opcode == 0b101001) && (offset == 0x30000006)){
          write_ADDR_PUTC(data & 0xFF);          
        }
        else if((opcode == 0b101001) && (offset != 0x30000006)){
          write_ADDR_PUTC(0);
        }
      }
      else{
        exit(-20);
      }
    }

    // unused memory: memory error is accessed
    else if(offset >= 0x30000008 && offset < 0x30000008 + 0xCFFFFFF8){
      exit(-11); // is this error?
    }

    // if none of the previous paths are taken there is a memory error
    else{
      exit(-11);
    }
}

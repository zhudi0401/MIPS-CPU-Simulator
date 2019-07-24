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

void decode_binary_input(int binary){
    //get opcode and make sure it doesn't get sign extended
    int opcode = (binary >> 26) & 0x3F;

    // decode R-type instructions
    if(opcode == 0){
        // rs
        int rs = binary & 0b00000011111000000000000000000000;
        rs = rs >> 21;

        // rt
        int rt = binary & 0b00000000000111110000000000000000;
        rt = rt >> 16;

        // rd
        int rd = binary & 0b00000000000000001111100000000000;
        rd = rd >> 11;

        // sa
        int shamt = binary & 0b00000000000000000000011111000000;
        shamt = (shamt >> 6)& 0x0000001F;

        // funct
        int funct = binary & 0b00000000000000000000000000111111;

        // call function -> performs instruction depending on funct
        return r_type(rs, rt, rd, shamt, funct);
    }

    // decode J-type instructions
    else if(opcode == 0b000011 || opcode == 0b000010){
        //target
        int target = binary & 0b00000011111111111111111111111111;
        // call function -> performs instruction depending on opcode
        return j_type(opcode, target);
    }

    // decode I-type instructions
    else if(opcode == 0b001000 || opcode == 0b001001 || opcode == 0b001100 || \
        opcode == 0b000100 || opcode == 0b000001 || opcode == 0b000111 || opcode == 0b000110 || \
        opcode == 0b000101 || opcode == 0b100000 || opcode == 0b100100 || opcode == 0b100001 || \
        opcode == 0b100101 || opcode == 0b001111 || opcode == 0b100011 || opcode == 0b001101 || \
        opcode == 0b101000 || opcode == 0b001010 || opcode == 0b001011 || opcode == 0b101001 || \
        opcode == 0b101011 || opcode == 0b001110 || opcode == 0b100010 || opcode == 0b100110){

        // rs
        int rs = binary & 0b00000011111000000000000000000000;
        rs = rs >> 21;

        // rt
        int rt = binary & 0b00000000000111110000000000000000;
        rt = rt >> 16;

        //immediate
        int16_t immediate = binary & 0b00000000000000001111111111111111;
        // call function -> performs instruction depending on opcode
        return i_type(opcode, rs, rt, immediate);
    }
    // invalid instruction if opcode doesn't correspond to one of the above
    else{
        exit(-12);
    }
}

void r_type(int rs, int rt, int rd, int shamt, int funct){

    // ADD
    if(funct == 0b100000){
      uint32_t sum = mips_register.read_register(rs) + mips_register.read_register(rt);
      if(((mips_register.read_register(rs) & 0x80000000) == 0) && ((mips_register.read_register(rt) & 0x80000000) == 0) && ((sum & 0x80000000) != 0)){
        exit(-10);
      }
      else if(((mips_register.read_register(rs) & 0x80000000) != 0) && ((mips_register.read_register(rt) & 0x80000000) != 0) && ((sum & 0x80000000) == 0)){
        exit(-10);
      }
      else{
        mips_register.write_register(rd, sum);
        mips_register.update_4_PC();
      }
    }

    // ADDU
    else if(funct == 0b100001){
        mips_register.write_register(rd, mips_register.read_register(rs) + mips_register.read_register(rt));
        // update PC by four
        mips_register.update_4_PC();
    }

    // AND
    else if(funct == 0b100100){
        mips_register.write_register(rd, mips_register.read_register(rs) & mips_register.read_register(rt));
        // update PC by four
        mips_register.update_4_PC();
    }

    // DIV
    else if(funct == 0b011010){
        int32_t tmp_rs = mips_register.read_register(rs);
        int32_t tmp_rt = mips_register.read_register(rt);
        // quotient
        if(tmp_rt != 0){
            int32_t quotient = tmp_rs / tmp_rt;
            mips_register.write_LO(quotient);
            // remainder
            int32_t remainder = tmp_rs % tmp_rt;
            mips_register.write_HI(remainder);           
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // DIVU
    else if(funct == 0b011011){
        if(mips_register.read_register(rt) != 0){
            // remainder
            mips_register.write_HI(mips_register.read_register(rs) % mips_register.read_register(rt));

            // quotient
            mips_register.write_LO(mips_register.read_register(rs) / mips_register.read_register(rt));
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // JALR
    else if(funct == 0b001001){
        int32_t tmp = mips_register.read_register(rs);
        mips_register.write_register(rd, 0x10000000 + mips_register.read_PC() + 8);
        if(tmp % 4 == 0){
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(tmp,0,0));
        }
        else{
            exit(-11);
        }
    }

    // JR
    else if(funct == 0b001000){
        int32_t tmp = mips_register.read_register(rs);
        if(tmp % 4 == 0){ // makes sure address is aligned (should we check && (number_of_instructions*4 + 0x20000000) >= tmp)
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(tmp,0,0));
        }
        else{
            exit(-11);
        }
    }

    // MFHI
    else if(funct == 0b010000){
        mips_register.write_register(rd, mips_register.read_HI());
        // update PC by four
        mips_register.update_4_PC();
    }

    // MFLO
    else if(funct == 0b010010){
        mips_register.write_register(rd, mips_register.read_LO());
        // update PC by four
        mips_register.update_4_PC();
    }

    // MTHI
    else if(funct == 0b010001){
        mips_register.write_HI(mips_register.read_register(rs));
        // update PC by four
        mips_register.update_4_PC();
    }

    // MTLO
    else if(funct == 0b010011){
        mips_register.write_LO(mips_register.read_register(rs));
        // update PC by four
        mips_register.update_4_PC();
    }

    // MULT
    else if(funct == 0b011000){
        int64_t product;
        // make registers "signed"
        int64_t tmp_rs = mips_register.read_register(rs);
        int64_t tmp_rt = mips_register.read_register(rt);
        tmp_rs = tmp_rs << 32;
        tmp_rs = tmp_rs >> 32;
        tmp_rt = tmp_rt << 32;
        tmp_rt = tmp_rt >> 32;
        
        product = tmp_rs * tmp_rt;
        mips_register.write_LO(product & 0x00000000FFFFFFFF);
        mips_register.write_HI((product >> 32) & 0x00000000FFFFFFFF);
        // update PC by four
        mips_register.update_4_PC();
    }

    // MULTU
    else if(funct == 0b011001){
        uint64_t product;
        uint64_t tmp_rs = mips_register.read_register(rs);
        uint64_t tmp_rt = mips_register.read_register(rt);
        product = tmp_rs * tmp_rt;
        mips_register.write_LO(product & 0x00000000FFFFFFFF);
        mips_register.write_HI((product >> 32) & 0x00000000FFFFFFFF);

        // update PC by four
        mips_register.update_4_PC();
    }

    // OR
    else if(funct == 0b100101){
        mips_register.write_register(rd, mips_register.read_register(rs) | mips_register.read_register(rt));

        // update PC by four
        mips_register.update_4_PC();
    }

    // NOR
    else if(funct == 0b100111){
        mips_register.write_register(rd, ~(mips_register.read_register(rs) | mips_register.read_register(rt)));

        // update PC by four
        mips_register.update_4_PC();
    }

    // SLL
    else if(funct == 0b000000){
        mips_register.write_register(rd, mips_register.read_register(rt) << shamt);

        // update PC by four
        mips_register.update_4_PC();
    }

    // SLLV
    else if(funct == 0b000100){
        mips_register.write_register(rd, mips_register.read_register(rt) << (mips_register.read_register(rs) & 0b00000000000000000000000000011111));

        // update PC by four
        mips_register.update_4_PC();
    }

    // SLT
    else if(funct == 0b101010){
        int32_t rs_signed = mips_register.read_register(rs);
        int32_t rt_signed = mips_register.read_register(rt);
        if(rs_signed < rt_signed){
            mips_register.write_register(rd, 1);
        }
        else{
            mips_register.write_register(rd, 0);
        }

        // update PC by four
        mips_register.update_4_PC();
    }

    // SLTU
    else if(funct == 0b101011){
        if(mips_register.read_register(rs) < mips_register.read_register(rt)){
            mips_register.write_register(rd, 1);
        }
        else{
            mips_register.write_register(rd, 0);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // SRA
    else if(funct == 0b000011){
        int32_t signed_rt  = mips_register.read_register(rt);
        mips_register.write_register(rd, signed_rt >> shamt);
        // update PC by four
        mips_register.update_4_PC();
    }

    // SRAV
    else if(funct == 0b000111){
        int32_t signed_rt  = mips_register.read_register(rt);
        mips_register.write_register(rd, signed_rt >> (mips_register.read_register(rs) & 0x1F));
        // update PC by four
        mips_register.update_4_PC();
    }

    // SRL
    else if(funct == 0b000010){
        mips_register.write_register(rd, mips_register.read_register(rt) >> shamt);
        // update PC by four
        mips_register.update_4_PC();
    }

    // SRLV
    else if(funct == 0b000110){
        mips_register.write_register(rd, mips_register.read_register(rt) >> (mips_register.read_register(rs) & 0x1F));
        // update PC by four
        mips_register.update_4_PC();
    }

    // SUB
    else if(funct == 0b100010){
        int sub = mips_register.read_register(rs) - mips_register.read_register(rt);
        if(((mips_register.read_register(rs) & 0x80000000) == 0) && ((mips_register.read_register(rt) & 0x80000000) != 0) && ((sub & 0x80000000) != 0)){
            exit(-10);
        }
        else if(((mips_register.read_register(rs) & 0x80000000) != 0) && ((mips_register.read_register(rt) & 0x80000000) == 0) && ((sub & 0x80000000) == 0)){
            exit(-10);
        }
        else{
          mips_register.write_register(rd, sub);
          // update PC by fours
          mips_register.update_4_PC();
        }
    }

    // SUBU
    else if(funct == 0b100011){
        mips_register.write_register(rd, mips_register.read_register(rs) - mips_register.read_register(rt));
        // update PC by four
        mips_register.update_4_PC();
    }

    // XOR
    else if(funct == 0b100110){
        mips_register.write_register(rd, mips_register.read_register(rs) ^ mips_register.read_register(rt));
        // update PC by four
        mips_register.update_4_PC();
    }

    else{
        exit(-12);
    }
}

void i_type(int opcode, int rs, int rt, int16_t immediate){
    // ADDI
    if(opcode == 0b001000){
      // extend 16 bits of immediate into 32 immediate
      int32_t immediate_32 = immediate;

      int sum = mips_register.read_register(rs) + immediate_32;
      if(((mips_register.read_register(rs) & 0x80000000) == 0) && ((immediate_32 & 0x80000000) == 0) && ((sum & 0x80000000) != 0)){
        exit(-10);
      }
      else if(((mips_register.read_register(rs) & 0x80000000) != 0) && ((immediate_32 & 0x80000000) != 0) && ((sum & 0x80000000) == 0)){
        exit(-10);
      }
      else{
        mips_register.write_register(rt, sum);
        mips_register.update_4_PC();
      }
    }

    // ADDIU
    else if(opcode == 0b001001){
      // extend 16 bit immediate into 32 bit immediate
      int32_t immediate_32 = immediate;
      int sum = mips_register.read_register(rs) + immediate_32;
      mips_register.write_register(rt, sum);
      mips_register.update_4_PC();
    }

    // ANDI
    else if(opcode == 0b001100){
      // zero extend 16 bit immediate into 32 bit immediate
      int32_t immediate_zero_ext = immediate & 0b00000000000000001111111111111111;
      mips_register.write_register(rt, mips_register.read_register(rs) & immediate_zero_ext);
      // update PC by four
      mips_register.update_4_PC();
    }

    // BEQ
    else if(opcode == 0b000100){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        if(mips_register.read_register(rs) == mips_register.read_register(rt)){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BGEZ
    else if(opcode == 0b000001 && rt == 0b00001){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        // check the MSB bit
        if((mips_register.read_register(rs) & 0x80000000) == 0){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // i.e. does jump
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BGEZAL
    else if(opcode == 0b000001 && rt == 0b10001){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        mips_register.write_register(31, mips_register.read_PC()+8+0x10000000);
        // check the MSB bit
        if((mips_register.read_register(rs) & 0x80000000) == 0){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // i.e. does jump
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BGTZ
    else if(opcode == 0b000111 && rt == 0b00000){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        // check the MSB bit
        if(((mips_register.read_register(rs) & 0x80000000) == 0) && (mips_register.read_register(rs) != 0)){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // i.e. does jump
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BLEZ
    else if(opcode == 0b000110 && rt == 0b00000){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        // check the MSB bit
        if(((mips_register.read_register(rs) & 0x80000000) != 0) || (mips_register.read_register(rs) == 0)){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BLTZ
    else if(opcode == 0b000001 && rt == 0b00000){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        // check the MSB bit
        if((mips_register.read_register(rs) & 0x80000000) != 0){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BLTZAL
    else if(opcode == 0b000001 && rt == 0b10000){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;

        mips_register.write_register(31, mips_register.read_PC()+8+0x10000000);
        // check the MSB bit
        if((mips_register.read_register(rs) & 0x80000000) != 0){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // i.e. does jump
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // BNE
    else if(opcode == 0b000101){
        // sign extend
        int32_t immediate32 = immediate;
        int32_t effective_address = (immediate32 << 2) + 0x10000000 + mips_register.read_PC() + 4;
        if(mips_register.read_register(rs) != mips_register.read_register(rt)){
            // do instruction in branch delay slot
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            // update PC by four
            mips_register.update_4_PC();
        }
    }

    // LB
    else if(opcode == 0b100000){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which byte
        if((effective_address & 0b11) == 0){
            // memory word is actually memory byte
            int8_t memory_byte = (memory_word >> 24) & 0x000000FF;
            int32_t word_sgn_ext = memory_byte;
            mips_register.write_register(rt, word_sgn_ext);
        }
        else if((effective_address & 0b11) == 1){
            // memory word is actually memory byte
            int8_t memory_byte = (memory_word >> 16) & 0x000000FF;
            int32_t word_sgn_ext = memory_byte;
            mips_register.write_register(rt, word_sgn_ext);
        }
        else if((effective_address & 0b11) == 2){
            // memory word is actually memory byte
            int8_t memory_byte = (memory_word >> 8) & 0x000000FF;
            int32_t word_sgn_ext = memory_byte;
            mips_register.write_register(rt, word_sgn_ext);
        }
        else{
            // memory word is actually memory byte
            int8_t memory_byte = memory_word & 0x000000FF;
            int32_t word_sgn_ext = memory_byte;
            mips_register.write_register(rt, word_sgn_ext);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // LBU
    else if(opcode == 0b100100){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which byte
        if((effective_address & 0b11) == 0){
            // memory word is actually memory byte
            memory_word = (memory_word >> 24) & 0x000000FF;
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 1){
            // memory word is actually memory byte
            memory_word = (memory_word >> 16) & 0x000000FF;
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 2){
            // memory word is actually memory byte
            memory_word = (memory_word >> 8) & 0x000000FF;
            mips_register.write_register(rt, memory_word);
        }
        else{
            // memory word is actually memory byte
            memory_word = memory_word & 0x000000FF;
            mips_register.write_register(rt, memory_word);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // LH
    else if(opcode == 0b100001){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which byte
        if((effective_address & 0b11) == 0){
            // memory word is actually memory byte
            int16_t memory_hword = (memory_word >> 16) & 0x0000FFFF;
            int32_t word_sgn_ext = memory_hword;
            mips_register.write_register(rt, word_sgn_ext);
        }
        else if((effective_address & 0b11) == 2){
            // memory word is actually memory byte
            int16_t memory_hword = memory_word & 0x0000FFFF;
            int32_t word_sgn_ext = memory_hword;
            mips_register.write_register(rt, word_sgn_ext);
        }
        else{
            exit(-11);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // LHU
    else if(opcode == 0b100101){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which byte
        if((effective_address & 0b11) == 0){
            // memory word is actually memory byte
            memory_word = (memory_word >> 16) & 0x0000FFFF;
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 2){
            // memory word is actually memory byte
            memory_word = memory_word & 0x0000FFFF;
            mips_register.write_register(rt, memory_word);
        }
        else{
            exit(-11);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // LUI
    else if(opcode == 0b001111){
        mips_register.write_register(rt, immediate << 16);
        // update PC by four
        mips_register.update_4_PC();
    }

    // LW
    else if(opcode == 0b100011){
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        if((effective_address & 0x00000003) != 0){
            exit(-11);
        }
        else{
          uint32_t data = mips_memory.access(effective_address,0,opcode);
          mips_register.write_register(rt, data);
        }
        mips_register.update_4_PC();
    }

    //LWL
    else if(opcode == 0b100010){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which bytes to overwrite
        if((effective_address & 0b11) == 0){
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 1){
            memory_word = (memory_word << 8) | (mips_register.read_register(rt) & 0x000000FF);
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 2){
            memory_word = (memory_word << 16) | (mips_register.read_register(rt) & 0x0000FFFF);
            mips_register.write_register(rt, memory_word);
        }
        else{
            memory_word = (memory_word << 24) | (mips_register.read_register(rt) & 0x00FFFFFF);
            mips_register.write_register(rt, memory_word);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    //LWR
    else if(opcode == 0b100110){
        // set read true
        mips_memory.set_read_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word = mips_memory.access(effective_address,0,opcode);
        // check which bytes to overwrite
        if((effective_address & 0b11) == 0){
            memory_word = ((memory_word >> 24) & 0x000000FF) | (mips_register.read_register(rt) & 0xFFFFFF00);
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 1){
            memory_word = ((memory_word >> 16) & 0x0000FFFF) | (mips_register.read_register(rt) & 0xFFFF0000);
            mips_register.write_register(rt, memory_word);
        }
        else if((effective_address & 0b11) == 2){
            memory_word = ((memory_word >> 8) & 0x00FFFFFF) | (mips_register.read_register(rt) & 0xFF000000);
            mips_register.write_register(rt, memory_word);
        }
        else{
            mips_register.write_register(rt, memory_word);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // ORI
    else if(opcode == 0b001101){
      // zero extend 16 bit immediate into 32 bit immediate
      int32_t immediate_zero_ext = immediate & 0b00000000000000001111111111111111;
      mips_register.write_register(rt, mips_register.read_register(rs) | immediate_zero_ext);
      // update PC by four
      mips_register.update_4_PC();
    }

    // SB
    else if(opcode == 0b101000){
        // set read true
        mips_memory.set_read_true();
        // LSB 8 bits of rt
        uint32_t tmp = mips_register.read_register(rt) & 0xFF;
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word;
        if(effective_address >= 0x30000004 && effective_address <= 0x30000004 + 0x3){
            memory_word = 0;
        }
        else{
            memory_word = mips_memory.access(effective_address & 0xFFFFFFFC,0,0);
        }
        // set read false
        mips_memory.set_read_false();
        // set write true
        mips_memory.set_write_true();
        // check which byte
        if((effective_address & 0b11) == 0){
            tmp = tmp << 24;
            memory_word = memory_word & 0x00FFFFFF;
            memory_word = memory_word | tmp;
            mips_memory.access(effective_address,memory_word,opcode);
        }
        else if((effective_address & 0b11) == 1){
            tmp = tmp << 16;
            memory_word = memory_word & 0xFF00FFFF;
            memory_word = memory_word | tmp;
            mips_memory.access(effective_address,memory_word,opcode);
        }
        else if((effective_address & 0b11) == 2){
            tmp = tmp << 8;
            memory_word = memory_word & 0xFFFF00FF;
            memory_word = memory_word | tmp;
            mips_memory.access(effective_address,memory_word,opcode);
        }
        else{
            memory_word = memory_word & 0xFFFFFF00;
            memory_word = memory_word | tmp;
            mips_memory.access(effective_address,memory_word,opcode);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // SLTI
    else if(opcode == 0b001010){
        int32_t rs_signed = mips_register.read_register(rs);
        int32_t immediate_signed = immediate;
        if(rs_signed < immediate_signed){
            mips_register.write_register(rt, 1);
        }
        else{
            mips_register.write_register(rt, 0);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // SLTIU
    else if(opcode == 0b001011){
        int32_t immediate32_signed = immediate;
        uint32_t immediate32_unsigned = immediate32_signed;
        if(mips_register.read_register(rs) < immediate32_unsigned){
            mips_register.write_register(rt, 1);
        }
        else{
            mips_register.write_register(rt, 0);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // SH
    else if(opcode == 0b101001){
        // set read true
        mips_memory.set_read_true();
        // LSB 16 bits of rt
        uint32_t tmp = mips_register.read_register(rt) & 0xFFFF;
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        uint32_t memory_word;
        if(effective_address >= 0x30000004 && effective_address <= 0x30000004 + 0x3){
            memory_word = 0;
        }
        else{
            memory_word = mips_memory.access(effective_address & 0xFFFFFFFC,0,0);
        }
        // set read false
        mips_memory.set_read_false();
        // set write true
        mips_memory.set_write_true();
        // check which byte
        if((effective_address & 0b11) == 0){
            tmp = tmp << 16;
            memory_word = memory_word & 0x0000FFFF;
            memory_word = memory_word | tmp;
                        mips_memory.access(effective_address,memory_word,opcode);
        }
        else if((effective_address & 0b11) == 2){
            memory_word = memory_word & 0xFFFF0000;
            memory_word = memory_word | tmp;
                       mips_memory.access(effective_address,memory_word,opcode);
        }
        else{
            exit(-11);
        }
        // update PC by four
        mips_register.update_4_PC();
    }

    // SW
    else if(opcode == 0b101011){
        mips_memory.set_write_true();
        int32_t immediate32 = immediate;
        // effective address
        int32_t effective_address = immediate32 + mips_register.read_register(rs);
        if((effective_address & 0x00000003) != 0){
            exit(-11);
        }
        else{
            mips_memory.access(effective_address, mips_register.read_register(rt),opcode);
        }
        mips_register.update_4_PC();
        // do exceptions other time!!!
    }

    // XORI
    else if(opcode == 0b001110){
      // zero extend 16 bit immediate into 32 bit immediate
      int32_t immediate_zero_ext = immediate & 0b00000000000000001111111111111111;
      mips_register.write_register(rt, mips_register.read_register(rs) ^ immediate_zero_ext);
      // update PC by four
      mips_register.update_4_PC();
    }
    else{
        exit(-20);
    }
}

void j_type(int opcode, int target){
    // J
    if(opcode == 0b000010){
        int32_t tmp_addr = target << 2;
        int32_t effective_address = tmp_addr | ((0x10000000 + mips_register.read_PC() + 4) & 0xF0000000);
        if(effective_address % 4 == 0){
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            exit(-11);
        }
    }

    // JAL
    else if(opcode == 0b000011){
        int tmp_addr = target << 2;
        int effective_address = tmp_addr | ((0x10000000 + mips_register.read_PC() + 4) & 0xF0000000);
        mips_register.write_register(31, 0x10000000 + mips_register.read_PC() + 8);
        if(effective_address % 4 == 0){
            decode_binary_input(mips_memory.read_ADDR_INSTR((mips_register.read_PC()+4)/4));
            // set execute flag true
            mips_memory.set_execute_true();
            // getting and setting next pc address or ending simulation if address == 0x0
            mips_register.update_address_PC(mips_memory.access(effective_address,0,0));
        }
        else{
            exit(-11);
        }
    }
    else{
        exit(-20);
    }
}
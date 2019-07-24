#ifndef REGISTERS_HPP
#define REGISTERS_HPP

class Register{
    private:
        // 32 registers
        std::vector<uint32_t> registers;

        uint32_t PC; // special-purpose register: PC tracks the current execution point within the code
        uint32_t IR; // instruction register
        uint32_t HI; // high
        uint32_t LO; // low

    public:
        // constructor
        Register();

        // high and low
        uint32_t read_HI();
        uint32_t read_LO();
        void write_HI(uint32_t input);
        void write_LO(uint32_t input);

        // program counter
        uint32_t read_PC();
        void update_4_PC();
        void update_address_PC(uint32_t next_address);

        uint32_t read_IR();
        void write_IR(uint32_t instruction);

        // all registers
        uint32_t read_register(uint32_t index)const; //index corresponds to index in the vector but also register number
        void write_register(uint32_t index, uint32_t input);
};

#endif

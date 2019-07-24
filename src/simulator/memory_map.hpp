#ifndef MEMORY_MAP_HPP
#define MEMORY_MAP_HPP

class Memory{
    private:
        // flags to determine which operations on memory are legal
        bool read;
        bool write;
        bool execute;

        std::vector<int> ADDR_INSTR; // executable memory
        std::vector<int> ADDR_DATA; // Read-write data area. Should be zero-initialised.
    public:
        // constructor
        Memory();

        // read from memory
        int read_ADDR_INSTR(int index);
        int read_ADDR_DATA(int index);
        uint32_t read_ADDR_GETC();

        //write to memory
        void write_ADDR_PUTC(int data_in);
        void write_ADDR_DATA(int data_in, int index);
        void load_ADDR_INSTR(int index, int number);

        // used to find correct memory address and determine if a certain instruction is legal in that region of memory
        uint32_t access(int32_t offset, int data, int opcode);

        // functions to set flags which determine legality of instructions on memory
        void set_read_true();
        void set_write_true();
        void set_execute_true();

        void set_read_false();
        void set_write_false();
        void set_execute_false();

        bool get_read();
        bool get_write();
        bool get_execute();
};

#endif

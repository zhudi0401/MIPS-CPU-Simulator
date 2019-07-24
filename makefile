# Example usage:
# If you have testbench/my_test.c, you can execute the following to create a MIPS binary
# make testbench/my_test.mips.bin

# For simulator
CC = g++
CPPFLAGS = -W -Wall

# For MIPS binaries. Turn on all warnings, enable all optimisations and link everything statically
MIPS_CC = mips-linux-gnu-gcc
MIPS_OBJCOPY = mips-linux-gnu-objcopy
MIPS_OBJDUMP = mips-linux-gnu-objdump
MIPS_CPPFLAGS = -W -Wall -O3 -fno-builtin -march=mips1 -mfp32
MIPS_LDFLAGS = -nostdlib -Wl,-melf32btsmip -march=mips1 -nostartfiles -mno-check-zero-division -Wl,--gpsize=0 -static -Wl,-Bstatic -Wl,--build-id=none

# Compile C file (.c) into MIPS object file (.o)
%.mips.o: %.c
	$(MIPS_CC) $(MIPS_CPPFLAGS) -c $< -o $@

# Assemble MIPS assembly file (.s) into MIPS object file (.o)
%.mips.o: %.s
	$(MIPS_CC) $(MIPS_CPPFLAGS) -c $< -o $@

# Link MIPS object file (.o), producing .elf, using memory locations specified in spec
%.mips.elf: %.mips.o
	$(MIPS_CC) $(MIPS_CPPFLAGS) $(MIPS_LDFLAGS) -T linker.ld $< -o $@

# Extract binary instructions only from linked object file (.elf)
%.bin: %.mips.elf
	$(MIPS_OBJCOPY) -O binary --only-section=.text $< $@

# Disassemble linked object file (.elf), pulling out instructions as MIPS assembly file (.s)
%.mips.s : %.mips.elf
	$(MIPS_OBJDUMP) -j .text -D $< > $@

simulator: memory_map.o registers.o decode.o main.o
	mkdir -p bin
	rm -f bin/mips_simulator
	$(CC) $(CPPFLAGS) src/simulator/memory_map.o src/simulator/registers.o src/simulator/decode.o src/simulator/main.o -o bin/mips_simulator

memory_map.o: src/simulator/memory_map.cpp src/simulator/memory_map.hpp
	$(CC) $(CPPFLAGS) -std=c++0x -c src/simulator/memory_map.cpp -o src/simulator/memory_map.o

main.o: src/simulator/main.cpp
	$(CC) $(CPPFLAGS) -std=c++0x -c src/simulator/main.cpp -o src/simulator/main.o

registers.o: src/simulator/registers.cpp src/simulator/registers.hpp
	$(CC) $(CPPFLAGS) -std=c++0x -c src/simulator/registers.cpp -o src/simulator/registers.o

decode.o: src/simulator/decode.cpp src/simulator/decode.hpp
	$(CC) $(CPPFLAGS) -std=c++0x -c src/simulator/decode.cpp -o src/simulator/decode.o

testbench:
	mkdir -p bin
	rm -f -rf bin/mips_testbench
	cp src/testbench/testbench bin/mips_testbench
	chmod +x bin/mips_testbench

clean:
	rm -f -rf src/simulator/*.o bin
	rm -f *.csv
	rm -rf test

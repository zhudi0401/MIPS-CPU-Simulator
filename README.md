# arch2-2018-cw-BrownLobster 

## Simulator build and execution ##

### The simulator is buildable using:

```
make simulator
```
### Simulate using:
```
bin/mips_simulator filename.bin
```
## Testbench build and execution

### Testbench timeout:
The testbench has a timeout for individual tests. It is currently set at 15 seconds per test. 
The user can change this by editing the `test_timeout` variable in the script.
Binaries should not expect a return value of `124` as this is reserved for the timeout. 

### The testbench is buildable using:
```
make testbench
```
### Test using:
* Test output will be printed to terminal:
```
bin/mips_testbench [relative path to simulator] 
```

* Test output will be printed to a csv file:
```
bin/mips_testbench [relative path to simulator] > filename.csv
```

### Additional tests:

* Add Mips Binaries:

  New tests can be added by adding binaries to `src/testbench/binaries`
  
  Binaries should obey the following naming template: `Instruction_Author_ExpectedResult_furtherIdetifier.bin`
  
  * e.g: `ADDU_oss1017_17_addu1.bin`
  
  For putC and getC it is also necessary to add a txt file with the expected char(s) to be written/taken as input, it should have the same name than the corresponding binary but with a ".txt" extension
  
  * e.g: `PUTC_oss1017_0_SWputc.txt`
  
    contents: X (this is what the corresponding binary should have as output)
  
  * e.g: `GETC_oss1017_0_SWgetc.txt`
  
    contents: X (this is what the corresponding binary will take as input to stdin)
  
* Add C code to be made into MIPS binaries:

  New tests can be added by adding C code to `src/testbench/c-code`
  
  Files should obey the following naming template: `Instruction_Author_ExpectedResult_furtherIdetifier.c`
  
  A corresponding binary file will be created when the testbench is executed. Binaries in `src/testbench/binaries` will be tested first, those based on c-code (in `src/testbench/c-code`) will be executed next.

# disassembler
Interpreting bytecode and spitting out the Y-86 assembly that would generate it

Run `make` to build and then `./disassembler [test-file] [output-file]` to run. 

`[test-file]` is a `.mem` file, and `[output-file]` is optional.
The program will print to stdout if `[output-file]` is not provided.

The Y-86 that that generated the `.mem` bytecode can be found in the corresponding `.ys` file.

For instance, running `./disassembler test_files/sum_64.mem` will decode the executable bytecode and interpret it as Y-86 assembly, and print the following code to the desired output file.

```
.pos 0x100
    irmovq  $0x140, %rdx 
    xorq    %rax, %rax 
    mrmovq  0x0(%rdx), %rbx 
    addq    %rbx, %rax 
    irmovq  $0x8, %rbx 
    addq    %rbx, %rdx 
    irmovq  $0x170, %rbx 
    subq    %rdx, %rbx 
    jne     0x10c 
    ret      
    halt     

.pos 0x140
    .quad   0x1234567890abc            
    .quad   0x1111111111111            
    halt     

.pos 0x158
    .quad   0x2468a            
    .quad   0x00ba            
    nop      
    nop      
    nop      
    nop      
    halt     
```

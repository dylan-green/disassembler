# disassembler
Interpreting bytecode and spitting out the Y-86 assembly that would generate it

Run `make` to build and then `./disassembler [test-file] [output-file]` to run. 
`[test-file]` is a `.mem` file, and `[output-file]` is an optional `.txt` file.
The program will print to stdout if `[output-file]` is not provided.

The Y-86 that that generated the `.mem` bytecode can be found in the corresponding `.ys` file. Comments have been added that to display the actual byte sequence for each instruction, i.e. `addq  %r8, %rdi     # 6087`.

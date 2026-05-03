# mini-decompiler

A minimal ELF decompiler for x86-64 binaries, written in C++.
Extracts a single function, disassembles it with Capstone, and reconstructs pseudo-C.

## Dependencies

- `g++` with C++17
- `capstone` (`sudo pacman -S capstone` on Arch Linux)
- `binutils` (for `objdump` / `readelf`)

## Build

```bash
make
```

## Usage

```bash
./mini-decompiler <elf-binary> [hex-address]
```

`hex-address` is the virtual address of the function to decompile (default: `0x1119`).
Find it with:
```bash
objdump -d <binary> | grep "<your_func>:"
```

## Examples

```bash
$ ./mini-decompiler samples/simple_add.bin 0x1119
[Pseudo-C Output]
int func(int a) {
    return a + 5;
}

$ ./mini-decompiler samples/simple_mul.bin 0x1119
[Pseudo-C Output]
int func(int a) {
    return a + a + a;
}

$ ./mini-decompiler samples/simple_sub.bin 0x1119
[Pseudo-C Output]
int func(int a) {
    return a - 3;
}
```

## Architecture

| File | Responsibility |
|---|---|
| `src/elf_parser.cpp` | Read ELF64 header, locate and extract `.text` bytes |
| `src/disassembler.cpp` | Wrap Capstone, decode bytes → `Instruction` structs |
| `src/lifter.cpp` | Pattern-match instructions → pseudo-C via register/stack tracking |
| `src/main.cpp` | Wire everything together, CLI entry point |


## Limitations

Decompilation is fundamentally lossy. Variable names, types, and structure cannot be
fully recovered from stripped binaries. This tool is an educational demonstration of
instruction lifting and not a production decompiler.

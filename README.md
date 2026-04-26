# mini-decompiler

A minimal ELF decompiler for x86-64 binaries, written in C++.
Extracts a single function, disassembles it with Capstone, and reconstructs pseudo-C.

## What it does

- Parses ELF64 binaries and extracts the `.text` section
- Disassembles a target function using [Capstone](https://www.capstone-engine.org/)
- Lifts x86-64 instructions to pseudo-C via pattern matching and register state tracking
- Handles: stack-spill/reload (`-O0` ABI patterns), `add`, `sub`, register-to-register ops

## What it doesn't do (by design)

- No multi-function support
- No control flow reconstruction (no `if`/`else`, no loops)
- No optimized binaries (`-O2` register allocation is much harder)
- No GUI

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

## Supported instruction patterns

| Assembly | Pseudo-C |
|---|---|
| `mov [rbp-N], edi` | argument spill (tracked) |
| `mov eax, [rbp-N]` | stack reload into register |
| `mov dst, src` | register copy |
| `add eax, 5` | `result = result + 5` |
| `add eax, edx` | `result = result + other` |
| `sub eax, 3` | `result = result - 3` |
| `sub eax, edx` | `result = result - other` |
| `push/pop rbp` | frame noise (skipped) |
| `ret` | `return <eax expression>` |

## Limitations

Decompilation is fundamentally lossy. Variable names, types, and structure cannot be
fully recovered from stripped binaries. This tool is an educational demonstration of
instruction lifting — not a production decompiler.

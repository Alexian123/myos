# myos

## Dependencies (Ubuntu/Debian packages) - WIP
- wget
- tar
- build-essential
- bison
- flex
- libgmp3-dev
- libmpc-dev
- libmpfr-dev
- texinfo
- libisl-dev
- mtools
- nasm
- qemu-system-x86
- bochs
- bochs-x
- bochsbios
- bochs-term

## Build & Run Instructions

- Build toolchain (GCC cross compiler & binutils):
```sh
make toolchain
```

- Build project
```sh
make    # defaults to 'make all'
```

- Build and run in QEMU
```sh
make run
```

- Build and debug in Bochs
```sh
make debug
```

- Clean project:
```sh
make clean
```
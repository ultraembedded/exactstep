# ExactStep - Instruction Accurate Instruction Set Simulator

Github: https://github.com/ultraembedded/exactstep

ExactStep is a simple multi-target instruction set simulator supporting RISC-V, ARM-v6m (and others - OpenRISC, MSP430 and MIPS to come soon).  
The emphasis of this project is on ease of extension, allowing its use as a library for cosimulation, peripheral development and System-C bus interfacing, rather than on raw execution performance.

Unlike QEMU and other CPU emulators which make use of dynamic binary translation, ExactStep executes one instruction per call to **cpu::step()**.

![](docs/screenshot.png)

## Cloning
```
$ git clone https://github.com/ultraembedded/exactstep.git
```

## Building

This project uses make and ELF + BFD libraries.

If you are using a Debian based Linux distro (Ubuntu / Linux Mint), you can install the required dependencies using;

```
$ sudo apt-get install libelf-dev binutils-dev
```

To build the default command line simulator (CLI);
```
$ cd exactstep
$ make
```

## Usage

```
Usage:
-f filename.bin/elf = Executable to load
-P platform         = (Optional) Platform to simulate (rv32im-basic|rv64im-basic|armv6m-basic)
-D device.dtb       = (Optional) Device tree blob (binary)
-t                  = (Optional) Enable program trace
-v 0xX              = (Optional) Trace Mask
-c nnnn             = (Optional) Max instructions to execute
-r 0xnnnn           = (Optional) Stop at PC address
-e 0xnnnn           = (Optional) Trace from PC address
-b 0xnnnn           = (Optional) Memory base address (for binary loads)
-s nnnn             = (Optional) Memory size (for binary loads)
```

The default architecture is a RV32-IM CPU model. To run a basic ELF;
```sh
$ ./exactstep -f your_elf.elf 
```

## License

[BSD 3-Clause](LICENSE)
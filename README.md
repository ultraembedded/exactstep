# ExactStep - Instruction Accurate Instruction Set Simulator

Github: https://github.com/ultraembedded/exactstep

ExactStep is a simple multi-target instruction set simulator supporting RISC-V (RV32IMAC, RV64IMAC), ARM-v6m (and others - OpenRISC, MSP430 and MIPS to come soon).  
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
  --elf        | -f FILE       File to load (ELF or BIN)
  --march      | -m MISA       Machine variant (e.g. RV32IMAC, RV64I, ...)
  --platform   | -P PLATFORM   Platform to simulate (basic|virt)
  --dtb        | -D FILE       Device tree blob (binary)
  --trace      | -t 1/0        Enable instruction trace
  --trace-mask | -v 0xXX       Trace mask (verbosity level)
  --cycles     | -c NUM        Max instructions to execute
  --stop-pc    | -r PC         Stop at PC address
  --trace-pc   | -e PC         Trace from PC address
  --mem-base   | -b VAL        Memory base address (for binary loads)
  --mem-size   | -s VAL        Memory size (for binary loads)
  --dump-file  | -p FILE       File to dump memory contents to after completion
  --dump-start | -j SYM/A      Symbol name for memory dump start (or 0xADDR)
  --dump-end   | -k SYM/A      Symbol name for memory dump end (or 0xADDR)
```

The default architecture is a RV32IMAC CPU model. To run a basic ELF;
```sh
$ ./exactstep -f your_elf.elf 
```

## License

[BSD 3-Clause](LICENSE)
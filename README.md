# ExactStep - Instruction Accurate Instruction Set Simulator

![Build](https://github.com/ultraembedded/exactstep/workflows/Build/badge.svg)

Github: https://github.com/ultraembedded/exactstep

ExactStep is a simple multi-target instruction set simulator supporting RISC-V (RV32IMAC, RV64IMAC), MIPS (mips-i), and ARM-v6m (with others to come soon).  
The emphasis of this project is on ease of extension, allowing its use as a library for cosimulation, peripheral development and System-C bus interfacing, rather than on raw execution performance.

Unlike QEMU and other CPU emulators which make use of dynamic binary translation, ExactStep executes one instruction per call to **cpu::step()**.

![](docs/screenshot.png)

## Cloning
```
git clone https://github.com/ultraembedded/exactstep.git
```

## Building

This project uses make and ELF, BFD, FDT libraries.

If you are using a Debian based Linux distro (Ubuntu / Linux Mint), you can install the required dependencies using;

```
sudo apt-get install libelf-dev binutils-dev libfdt-dev
```

To build the default command line simulator and RISC-V Linux simulators;
```
cd exactstep
make
```

## ExactStep: Usage
*exactstep* supports various emulated CPU architectures (RISC-V, MIPS, ARM), and is used to run bare-metal executables compiled for those ISAs;
```
./exactstep
Usage:
  --elf        | -f FILE       File to load (ELF or BIN)
  --march      | -m MISA       Machine variant (e.g. RV32IMAC, RV64I, armv6, mips, ...)
  --platform   | -P PLATFORM   Platform to simulate (basic|virt)
  --dtb        | -D FILE       Device tree blob (binary)
  --trace      | -t 1/0        Enable instruction trace
  --trace-mask | -v 0xXX       Trace mask (verbosity level)
  --cycles     | -c NUM        Max instructions to execute
  --stop-pc    | -r PC         Stop at PC address
  --trace-pc   | -e PC         Trace from PC address
  --elf-phys   | -E            Load to ELF section to physical addresses (suitable for bootloaders)
  --mem-base   | -b VAL        Memory base address (for binary loads)
  --mem-size   | -s VAL        Memory size (for binary loads)
  --dump-file  | -p FILE       File to dump memory contents to after completion
  --dump-start | -j SYM/A      Symbol name for memory dump start (or 0xADDR)
  --dump-end   | -k SYM/A      Symbol name for memory dump end (or 0xADDR)
  --dump-reg-f | -R FILE       File to dump register file contents to after completion
  --dump-reg-s | -S NUM        Number of register file entries to dump
  --vda        | -V FILE       Disk image for VirtIO block device (/dev/vda)
  --tap        | -T TAP        Tap device for VirtIO net device
```

The default architecture is a RV32IMAC CPU model. To run a basic ELF;
```sh
./exactstep -f your_elf.elf 
```

## Exactstep-riscv-linux: Usage
*exactstep-riscv-linux* is a RISC-V (32-bit or 64-bit) specific simulator which contains a built-in SBI (Supervisor Binary Interface) implementation that enables booting RISC-V Linux kernels compiled for supervisor mode.
Root filesystems can also be provided by initrd, VirtIO block device, or VirtIO network (nfs) boot.

```
./exactstep-riscv-linux
Usage:
  --elf        | -f FILE       File to load (ELF)
  --bin        | -b FILE       File to load (binary)
  --march      | -m MISA       Machine variant (e.g. RV32IMAC, RV64I, ...)
  --platform   | -P PLATFORM   Platform to simulate (basic|virt)
  --dtb        | -D FILE       Device tree blob (binary)
  --dtb-base   | -B 0xaddr     Device tree blob load address
  --trace      | -t 1/0        Enable instruction trace
  --trace-mask | -v 0xXX       Trace mask (verbosity level)
  --cycles     | -c NUM        Max instructions to execute
  --stop-pc    | -r PC         Stop at PC address
  --trace-pc   | -e PC         Trace from PC address
  --vda        | -V FILE       Disk image for VirtIO block device (/dev/vda)
  --tap        | -T TAP        Tap device for VirtIO net device
  --initrd     | -i FILE       initrd binary (optional)
```

Example usage (with a device tree compiled to a DTB file using the Linux Kernel dtc util);
```sh
./exactstep-riscv-linux --elf ./vmlinux-rv32ima-5.0 --dtb ./config.dtb --initrd ./initrd.cpio 
```

## Running RISC-V Compliance Tests

ExactStep passes the RISC-V Compliance Tests for the rv32i, rv32im, rv32imc, rv64i, rv64im categories;
```
# Get compliance test suite
git clone https://github.com/ultraembedded/riscv-compliance.git 
cd riscv-compliance

# Set path to built exactstep executable
export TARGET_SIM=/path/to/github/exactstep/exactstep

# Run test suite
make RISCV_TARGET=exactstep
```

## Running RISC-V Linux
```
# Get prebuilt bootloader + kernel + rootfs images
git clone https://github.com/ultraembedded/riscv-linux-prebuilt.git
cd riscv-linux-prebuilt

# Boot Linux (with Busybox userspace)
exactstep --march RV64IMAC --elf opensbi-kernel-busybox/qemu-virt-rv64-5.4-rc7-busybox-1.32.0.elf --dtb opensbi-kernel-busybox/qemu-virt-rv64-config.dtb
```

## License

[BSD 3-Clause](LICENSE)

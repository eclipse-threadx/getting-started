---
page_type: sample
description: Using the Arm MPS3 AN524 with Azure RTOS
languages:
- c
products:
- azure-rtos
---

# Getting started with the Arm MPS3 FGPA board and the AN524

**Total completion time**:  30 minutes

In this tutorial you use Azure RTOS on the AN524, which is an Arm Cortex-M33
image that can be run on the MPS3 FPGA prototyping board or in QEMU.

## Prerequisites

* [Git](https://git-scm.com/downloads) for cloning the repository
* Hardware or [QEMU](#using-qemu)

    > * The [Arm MPS3 FPGA Prototyping Board](https://developer.arm.com/tools-and-software/development-boards/fpga-prototyping-boards/mps3) (MPS3)
    > * The [AN524 SSE-200 FPGA image for MPS3](https://developer.arm.com/tools-and-software/development-boards/fpga-prototyping-boards/download-fpga-images)
* To use QEMU, `srec_cat` must be installed to merge TF-M and Azure RTOS
  example application. You can install it via `$ sudo apt-get install srecord`
  on Ubuntu or `$ brew install srecord` on OS X.
* We've automated the process of [updating application image onto the Arm MPS3
  FPGA Prototyping Board](#updating-application-image). If you wish to use this
  , then `pyserial` python module should be installed:
    > * pip3 install pyserial

## System Setup

### FTDI VCP Drivers

Your OS may not properly enumerate the four serial ports provided by an FTDI
USB serial chip on the MPS3 board. If they don't appear, you must install the
latest VCP driver and restart your system:
https://ftdichip.com/drivers/vcp-drivers/

You should see five distinct serial ports (one for the LPC1768 debug MCU, and
four from the FTDI chip), where the first two ports from the FTDI chip are of
relevance to this demo:

- `tty.usbserial-xxxxxxxxA` = The MPS3 bootloader
- `tty.usbserial-xxxxxxxxB` = Azure (etc.) serial output

### AN524 PyOCD Support

If you wish to debug, MPS3 AN524 support is available in pyocd via a CMSIS
pack, which can be installed as follows:

```bash
$ pyocd pack -u
$ pyocd pack -i SSE-200-MPS3
$ pyocd list --targets | grep MPS3
```

You can then start a GDB debug session via:

```bash
$ pyocd gdbserver --target=sse-200-mps3
```

Then in another terminal:

```bash
$ arm-none-eabi-gdb-py -s build/app/mps3_524.elf \
  -ex "target remote tcp:localhost:3333"
(gdb) monitor reset halt
(gdb) break main
(gdb) continue
```

> Please note that there is a known issue with resetting the AN524 with the
  debugger (https://github.com/pyocd/pyOCD/issues/1078). The `pyocd_user.py`
  script in this folder (`Arm/MPS3_AN524`) provides a workaround for this
  issue, and pyocd should be run from the folder where this script is located.

## Building the Sample

The sample requires both a GCC cross compiler for armv7m, usually called `arm-none-eabi-gcc`,
and the `srec_cat` utility, which is commonly part of the `srecord` packace on linux
distributions. Build the sample with the provided `rebuild.sh` tool.
  
```bash
$ cd Arm/MPS3_AN524
$ tools/rebuild.sh
```

### Configuring the sample

There are a few configuration options available to, for example, change the
secure partitions used by TF-M. These options are defined and documented in 
`Arm/MPS3_AN524/app/config.cmake`. You may pass your own overriding options to
cmake by defining the `APP_CONFIG` variable to a path to a file that sets these
options.

The ThreadX CMSIS RTOS2 interface statically allocates threads, thread stacks
and semaphores. The size of these allocations may also be configured.
Configuration options for these are defined and documented in
`Arm/MPS3_AN524/lib/threadx/CMmakeLists.txt`.

### Running TF-M NS regression tests
TF-M NS regression tests config option `TFM_REGRESSION` is documented in
`Arm/MPS3_AN524/app/config.cmake` and is disabled by default. To run TF-M NS
regression tests, set the config option to `ON` and rebuild the application.
Then run it either on [QEMU](#using-qemu) or [hardware](#updating-application-image)

### Running PSA Arch tests
PSA arch tests config option `TFM_PSA_TEST` is documented in
`Arm/MPS3_AN524/app/config.cmake` and is disabled by default. To run PSA arch
tests, set the config option to any of `CRYPTO`|`PROTECTED_STORAGE`|
`INTERNAL_TRUSTED_STORAGE`|`STORAGE`|`INITIAL_ATTESTATION`.
Then run it either on [QEMU](#using-qemu) or [hardware](#updating-application-image)

*Note*  
The TF-M version 1.3 and PSA architecture crypto test suite version 1.1 causes
resets on QEMU. However, the crypto test suite completes without any issues on
Arm MPS3 FPGA board. Other test suites can be run either on QEMU or Arm MPS3
FPGA board.

Following summary will be printed at the end of crypto tests:

```bash
************ Crypto Suite Report **********
TOTAL TESTS     : 63
TOTAL PASSED    : 39
TOTAL SIM ERROR : 0
TOTAL FAILED    : 17
TOTAL SKIPPED   : 7
******************************************
```

## Updating application image
> This has only been tested on OS X, some modifications might be needed to make
  `Arm/MPS3_AN524/tools/flash.py` to work in Windows/Linux environments.

The python script `Arm/MPS3_AN524/tools/flash.py` automates steps needed to
update an application image onto the Arm MPS3 FPGA Prototyping Board. if you
wish to use this, then add `-f` command line option while invoking `rebuild.sh`.
If `-f` option is used then FPGA MCC serial port must be provided using `-p`
option.

```bash
$ cd Arm/MPS3_AN524
$ tools/rebuild.sh -f -p /dev/tty.usbserial-14543100
```

## Using QEMU
QEMU can be used to emulate MPS3 FPGA board with AN524 and run Azure RTOS
example application. However, AN524 memory remap feature is required
successfully run the application. This feature was added to QEMU after current
6.0.0 release. Therefore we need to [build QEMU from source](#building-qemu).

After build is successful and path to QEMU executable `qemu-system-arm` is
added to `PATH` environment variable, QEMU can be started from build script by
using `-q` option.

```bash
$ cd Arm/MPS3_AN524
$ tools/rebuild.sh -q
```

### Building QEMU
Follow the steps below to build QEMU from source:

```bash
cd /path/for/qemu/source
git clone https://github.com/qemu/qemu.git

mkdir build && cd build

../configure --disable-docs --disable-sdl --disable-debug-info --disable-cap-ng \
    --disable-libnfs --disable-libusb --disable-libiscsi --disable-usb-redir \
    --disable-linux-aio --disable-guest-agent --disable-libssh --disable-vnc-png \
    --disable-seccomp  --disable-tpm --disable-numa --disable-glusterfs \
    --disable-virtfs --disable-xen --disable-curl --disable-attr --disable-curses \
    --disable-iconv --target-list="aarch64-softmmu arm-softmmu"

make
```

Once build completes successfully, run the following command to check supported machines.

```bash
./qemu-system-arm -M help
```
It should contain the following machines:
```bash
mps2-an385           ARM MPS2 with AN385 FPGA image for Cortex-M3
mps2-an386           ARM MPS2 with AN386 FPGA image for Cortex-M4
mps2-an500           ARM MPS2 with AN500 FPGA image for Cortex-M7
mps2-an505           ARM MPS2 with AN505 FPGA image for Cortex-M33
mps2-an511           ARM MPS2 with AN511 DesignStart FPGA image for Cortex-M3
mps2-an521           ARM MPS2 with AN521 FPGA image for dual Cortex-M33
mps3-an524           ARM MPS3 with AN524 FPGA image for dual Cortex-M33
mps3-an547           ARM MPS3 with AN547 FPGA image for Cortex-M55
musca-a              ARM Musca-A board (dual Cortex-M33)
musca-b1             ARM Musca-B1 board (dual Cortex-M33)
```

Make sure that path to QEMU executable `qemu-system-arm` is added to `PATH`
environment variable.

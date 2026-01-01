# UEFI Games
Snake in bootloader form!

<img width="912" height="752" alt="image" src="https://github.com/user-attachments/assets/c203e14a-79a9-4c88-846a-da35472202af" />

## Building
You will need Clang and LLD. Additionally, `xorriso` and `mtools` are used by the Makefile to create a bootable ISO.

I am developing this on x86_64 Debian, itself running under QEMU emulation via UTM on aarch64 macOS. YMMV with other setups, but any x86_64 Linux should work fine.

The current target for the UEFI application is x86_64. I have not tested other architectures, but I believe it should be straightforward to build this for aarch64 or RISC-V.

To build an EFI executable and an ISO 9660 CD image containing it at \BOOT\EFI\BOOTX64.EFI in a FAT file system, run `make`.

## Running the UEFI executable
### QEMU via UTM
Create a `pc-q35-10.0` emulated x86_64 machine with a virtual CD/DVD drive, but no disk, network, or sound. Use the ISO file in the virtual CD drive.

### Real hardware
Not tested yet.

## Acknowledgements
This project includes (vendors) these EFI header files, with some additions/modifications: [yoppeh/efi at commit 761b114](https://github.com/yoppeh/efi/tree/761b114e3b186adb82516d5fa8e7a4c559f56ba5).

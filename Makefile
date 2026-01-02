# See https://dvdhrm.github.io/2019/01/31/goodbye-gnuefi/
CC = clang
CFLAGS = -Wall -Wextra -O2 --target=x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -mno-stack-arg-probe -DEFI_PLATFORM=1
LDFLAGS = --target=x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link
INCLUDES = -I./external/yoppeh-efi

OBJ = main.o
TARGET = games.efi
ISO = games.iso
IMG = efiboot.img

all: $(ISO)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(ISO): $(TARGET)
	mkdir -p iso_root/EFI/BOOT
	cp $(TARGET) iso_root/EFI/BOOT/BOOTX64.EFI

	dd if=/dev/zero of=$(IMG) bs=1M count=40
	/sbin/mkfs.vfat -F 32 $(IMG)
	mmd -i $(IMG) ::/EFI
	mmd -i $(IMG) ::/EFI/BOOT
	mcopy -i $(IMG) $(TARGET) ::/EFI/BOOT/BOOTX64.EFI

	# required for xorriso to find it with the -e flag
	cp $(IMG) iso_root/

	xorriso -as mkisofs -R -J -V "EFI_APP" \
		-o $@ \
		-e $(IMG) \
		-no-emul-boot \
		-isohybrid-gpt-basdat \
		-append_partition 2 0xef $(IMG) \
		-partition_offset 16 \
		./iso_root

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET) $(ISO) $(IMG) iso_root

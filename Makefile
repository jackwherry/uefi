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

	dd if=/dev/zero of=$(IMG) bs=1M count=10
	/sbin/mkfs.vfat $(IMG)
	mmd -i $(IMG) ::/EFI
	mmd -i $(IMG) ::/EFI/BOOT
	mcopy -i $(IMG) $(TARGET) ::/EFI/BOOT/BOOTX64.EFI

	xorriso -as mkisofs -R -J -V "EFI_APP" \
		-o $@ \
		--efi-boot $(IMG) \
		-no-emul-boot \
		./iso_root ./$(IMG)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET) $(ISO) $(IMG) iso_root

CFLAGS = -Wall -Wextra -O2
LDFAGS = 

# See https://dvdhrm.github.io/2019/01/31/goodbye-gnuefi/
CFLAGS += --target=x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS += --target=x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

INCLUDES = $(shell pkg-config --cflags gnu-efi)
OBJ = main.o

%.o: %.c
	clang $(CFLAGS) $(INCLUDES) -o $@ -c $<

BOOTX64.EFI: $(OBJ)
	clang $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) BOOTX64.EFI

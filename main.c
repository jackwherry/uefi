#include "efi.h"

CHAR16 *hello_str = L"Hello, world!\r\n";

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	st->ConOut->OutputString(st->ConOut, hello_str);
	return EFI_SUCCESS;
}

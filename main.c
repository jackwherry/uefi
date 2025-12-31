#include "efi.h"

CHAR16 *hello_str = L"Hello, world! build 4\r\n";

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	st->ConOut->OutputString(st->ConOut, hello_str); // this returns a status but we're just gonna assume it succeeded :)

	// empty console input buffer
	st->ConIn->Reset(st->ConIn, EFI_FALSE);

	EFI_INPUT_KEY Key;
	UINTN index;
	st->BootServices->WaitForEvent(1, &st->ConIn->WaitForKey, &index);
	st->ConIn->ReadKeyStroke(st->ConIn, &Key); // read to clear state

	return EFI_SUCCESS;
}

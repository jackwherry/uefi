#include "efi.h"

// EFI_ERROR definitions, lifted from EDK2 headers
// TODO: rewrite this less obtusely
// "A value of native width with the highest bit set." -- MdePkg/Include/X64/ProcessorBind.h
#define MAX_BIT  0x8000000000000000ULL
typedef EFI_STATUS RETURN_STATUS; // this was backwards in the EDK2 headers but I figured they're the same
#define RETURN_ERROR(a) (((RETURN_STATUS)(a)) >= MAX_BIT)
#define EFI_ERROR(A) RETURN_ERROR(A)

CHAR16 *hello_str = L"UEFI Games v0.1.0\r\nbuild 5\r\n\r\n\r\nPress 1 for Snake or 2 for 3D engine\r\n";
EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE *SystemTable;

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	ImageHandle = ih;
	SystemTable = st;

	EFI_STATUS status;

	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, hello_str);
	if (EFI_ERROR(status)) {
		return status;
	}

	// empty console input buffer
	status = SystemTable->ConIn->Reset(SystemTable->ConIn, EFI_FALSE);
	if (EFI_ERROR(status)) {
		return status;
	}

	EFI_INPUT_KEY Key;
	UINTN index;
	SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &index);
	SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key); // read to clear state

	return status;
}

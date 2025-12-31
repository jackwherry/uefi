#include "efi.h"

// "A value of native width with the highest bit set." -- MdePkg/Include/X64/ProcessorBind.h
#define MAX_BIT  0x8000000000000000ULL

// substantially simplified from the version in the EDK2 headers
#define EFI_ERROR(a) (((EFI_STATUS)(a)) >= MAX_BIT)

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE *SystemTable;

EFI_STATUS raycast_main() {
	EFI_STATUS status;
	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"raycast_main() entered\r\n");
	if (EFI_ERROR(status)) {
		return status;
	}
	return EFI_SUCCESS;
}

EFI_STATUS snake_main() {
	EFI_STATUS status;
	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"snake_main() entered\r\n");
	if (EFI_ERROR(status)) {
		return status;
	}

	BOOLEAN quit = EFI_FALSE;
	UINTN n = 0;
	while (!quit) {
		n++;
		// clear screen, draw box, draw snake position
		status = SystemTable->ConOut->Reset(SystemTable->ConOut, EFI_FALSE);
		if (EFI_ERROR(status)) {
			return status;
		}

		for (UINTN i = 0; i < n; i++) {
			status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"s");
			if (EFI_ERROR(status)) {
				return status;
			}
		}

		// update snake position, check collision
		// get input
		// wait a second
		status = SystemTable->BootServices->Stall(1 * 1000000);
		if (EFI_ERROR(status)) {
			return status;
		}
	}

	return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	ImageHandle = ih;
	SystemTable = st;

	EFI_STATUS status;

	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
		L"\r\nUEFI Games v0.1.0\r\nbuild 13\r\n\r\n\r\nPress 1 for Snake or 2 for 3D engine\r\n");
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

	if (Key.UnicodeChar == L'1') {
		status = snake_main();
	} else if (Key.UnicodeChar == L'2') {
		status = raycast_main();
	}

	return status;
}

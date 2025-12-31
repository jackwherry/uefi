#include "efi.h"

// "A value of native width with the highest bit set." -- MdePkg/Include/X64/ProcessorBind.h
#define MAX_BIT  0x8000000000000000ULL

// substantially simplified from the version in the EDK2 headers
#define EFI_ERROR(a) (((EFI_STATUS)(a)) >= MAX_BIT)

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE *SystemTable;

EFI_STATUS raycast_main(void) {
	EFI_STATUS status;
	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"raycast_main() entered\r\n");
	if (EFI_ERROR(status)) {
		return status;
	}
	return EFI_SUCCESS;
}


#define GRID_DIM 19
EFI_STATUS snake_main(void) {
	EFI_STATUS status;

	enum object {
		none,
		head,
		body,
		food,
		wall
	};

	// the game grid; get an item at coord (x, y): grid[x + y * GRID_DIM]
	enum object grid[GRID_DIM * GRID_DIM];
	for (int i = 0; i < GRID_DIM * GRID_DIM; i++) {
		grid[i] = none;
	}

	// first row is wall
	for (int i = 0; i < GRID_DIM; i++) {
		grid[i] = wall;
	}

	// last row is wall
	for (int i = 0; i < GRID_DIM; i++) {
		grid[i + (GRID_DIM - 1) * GRID_DIM] = wall;
	}

	// first column is wall
	for (int i = 0; i < GRID_DIM; i++) {
		grid[0 + i * GRID_DIM] = wall;
	}

	// last column is wall
	for (int i = 0; i < GRID_DIM; i++) {
		grid[(GRID_DIM - 1) + i * GRID_DIM] = wall;
	}
	
	enum dir {
		north,
		south,
		east,
		west
	};

	int snake_length = 3;
	int speed_mult = 1000000; // starting speed is 1 second (measured in microseconds)
	enum dir snake_dir = north;

	BOOLEAN quit = EFI_FALSE;
	while (!quit) {
		// clear screen
		status = SystemTable->ConOut->Reset(SystemTable->ConOut, EFI_FALSE);
		if (EFI_ERROR(status)) {
			return status;
		}

		status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
			L"\r\n           S  N  A  K  E\r\n");
		if (EFI_ERROR(status)) {
			return status;
		}

		// draw grid
		for (int i = 0; i < GRID_DIM; i++) {
			for (int j = 0; j < GRID_DIM; j++) {
				CHAR16 cell_contents;
				enum object obj = grid[i + j * GRID_DIM];
				if (obj == none) {
					cell_contents = L' ';
				} else if (obj == head) {
					cell_contents = L'@';
				} else if (obj == body) {
					cell_contents = L'o';
				} else if (obj == food) {
					cell_contents = L'*';
				} else if (obj == wall) {
					cell_contents = L'#';
				} else {
					cell_contents = L'?';
				}
				status = SystemTable->ConOut->OutputString(SystemTable->ConOut, &cell_contents);
				if (EFI_ERROR(status)) {
					return status;
				}
			}
			status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
			if (EFI_ERROR(status)) {
				return status;
			}
		}

		// print score (snake length - 3) and speed multiplier
		status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\nScore: xxx    Delay: xxxxxxx");
		if (EFI_ERROR(status)) {
			return status;
		}

		// update snake position, check collision
		// get input
		// wait a second
		status = SystemTable->BootServices->Stall(speed_mult);
		if (EFI_ERROR(status)) {
			return status;
		}

		speed_mult -= ((snake_length - 3) * 1000); // 1 ms quicker each time the score goes up
	}

	return EFI_SUCCESS;
}

EFI_STATUS menu(void) {
	EFI_STATUS status;

	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
		L"\r\nUEFI Games v0.1.0\r\nbuild 23	\r\n\r\n\r\n");
	if (EFI_ERROR(status)) {
		return status;
	}

	BOOLEAN quit = EFI_FALSE;
	while (!quit) {
		status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
			L"Press 1 for Snake or 2 for 3D engine\r\n");
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
		} else {
			status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\nUnknown option\r\n");
			if (EFI_ERROR(status)) {
				return status;
			}
		}
	}

	return status;
}

// EFI application entry point
EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	ImageHandle = ih;
	SystemTable = st;

	return menu();
}

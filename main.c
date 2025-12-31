#include "efi.h"

//
// DECLARATIONS
//

// "A value of native width with the highest bit set." -- MdePkg/Include/X64/ProcessorBind.h
#define MAX_BIT  0x8000000000000000ULL

// substantially simplified from the version in the EDK2 headers
#define EFI_ERROR(a) (((EFI_STATUS)(a)) >= MAX_BIT)

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE *SystemTable;

// declared up here so that we can go back to the menu from within games
EFI_STATUS menu(void);

//
// RAYCAST
//

EFI_STATUS raycast_main(void) {
	EFI_STATUS status;
	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"raycast_main() entered\r\n");
	if (EFI_ERROR(status)) {
		return status;
	}
	return EFI_SUCCESS;
}

//
// SNAKE
//

#define GRID_DIM 19
EFI_STATUS snake_main(void) {
	EFI_STATUS status;

	enum dir {
		north,
		south,
		east,
		west
	};

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

	int snake_length = 3;
	enum dir snake_dir = north;
	int head_x = 9;
	int head_y = 10;
	int tail_x = 9;
	int tail_y = 12; // todo: better to just find the head in the grid and create a special tail type?

	grid[head_x + head_y * GRID_DIM] = head;
	grid[head_x + (head_y + 1) * GRID_DIM] = body;
	grid[tail_x + tail_y * GRID_DIM] = body;

	int speed_mult = 1000000; // starting speed is 1 second (measured in microseconds)
	BOOLEAN quit = EFI_FALSE;
	while (!quit) {
		// clear screen
		status = SystemTable->ConOut->Reset(SystemTable->ConOut, EFI_FALSE);
		if (EFI_ERROR(status)) {
			return status;
		}

		status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
			L"\r\n            S  N  A  K  E\r\n");
		if (EFI_ERROR(status)) {
			return status;
		}

		// draw grid
		for (int y = 0; y < GRID_DIM; y++) {
			for (int x = 0; x < GRID_DIM; x++) {
				CHAR16 cell_contents;
				enum object obj = grid[x + y * GRID_DIM];
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

				CHAR16 cell_buffer[3];
				cell_buffer[0] = cell_contents;
				cell_buffer[1] = L' ';
				cell_buffer[2] = L'\0';

				status = SystemTable->ConOut->OutputString(SystemTable->ConOut, cell_buffer);
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

		// determine next head position
		int next_x = -1;
		int next_y = -1;
		if (snake_dir == north) {
			next_x = head_x;
			next_y = head_y - 1;
		} else if (snake_dir == south) {
			next_x = head_x;
			next_y = head_y + 1;
		} else if (snake_dir == east) {
			next_x = head_x + 1;
			next_y = head_y;
		} else if (snake_dir == west) {
			next_x = head_x - 1;
			next_y = head_y;
		}

		// check for collisions
		enum object next_cell_contents = grid[next_x + next_y * GRID_DIM];
		if (next_cell_contents == wall || next_cell_contents == body) {
			// game over!

			status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
				L"GAME OVER!\r\nPress any key...\r\n");
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

			return EFI_SUCCESS;
		}

		// update head position
		head_x = next_x;
		head_y = next_y;

		// TODO: update in grid; need queue to figure out where to get the tail from?

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

	// clear screen
	status = SystemTable->ConOut->Reset(SystemTable->ConOut, EFI_FALSE);
	if (EFI_ERROR(status)) {
		return status;
	}

	status = SystemTable->ConOut->OutputString(SystemTable->ConOut, 
		L"\r\nUEFI Games v0.1.0\r\nbuild 30	\r\n==========================================\r\n\r\n");
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

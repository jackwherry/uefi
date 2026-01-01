#include "efi.h"
#include "protocol/efi-rng.h"

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

// a small subset of MdePkg/Include/Protocol/SimpleTextIn.h
#define SCAN_UP         0x0001
#define SCAN_DOWN       0x0002
#define SCAN_RIGHT      0x0003
#define SCAN_LEFT       0x0004

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

	// // get random position for first food
	// status = SystemTable->

	int snake_length = 3;
	enum dir snake_dir = north;
	int head_x = 9;
	int head_y = 10;

	grid[head_x + head_y * GRID_DIM] = head;
	grid[head_x + (head_y + 1) * GRID_DIM] = body;
	grid[head_x + (head_y + 2) * GRID_DIM] = body;

	// currently used only for history circular buffer
	struct coord {
		int x;
		int y;
	};

	// Circular buffer for keeping track of the age of the snake segments, so that we can
	// put the least recent segment in the new head position to "move" the snake 
	struct coord history[GRID_DIM * GRID_DIM];
	int head_idx;
	int tail_idx; // indeces within circular buffer for most and least recent segments, resp.

	history[0] = (struct coord){9, 12}; // tail
	history[1] = (struct coord){9, 11}; // body
	history[2] = (struct coord){9, 10}; // head
	tail_idx = 0;
	head_idx = 2;

	int delay = 1000000; // starting speed is 1 second (measured in microseconds)
	BOOLEAN quit = EFI_FALSE;
	while (!quit) {
		// PART I: DRAW
		// clear screen
		//status = SystemTable->ConOut->Reset(SystemTable->ConOut, EFI_FALSE);
		status = SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, 0, 0);
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

		// PART II: UPDATE

		// current head becomes body
		grid[head_x + head_y * GRID_DIM] = body;

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
				L"\r\nGAME OVER!\r\nPress any key...\r\n");
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

		head_x = next_x;
		head_y = next_y;

		head_idx = (head_idx + 1) % (GRID_DIM * GRID_DIM);
		history[head_idx].x = head_x;
		history[head_idx].y = head_y;

		grid[head_x + head_y * GRID_DIM] = head;

		// we already ate the food (it's been overwritten by our new head),
		// but next_cell_contents has not been updated
		if (next_cell_contents == food) {
			snake_length++;
			// no need to move the tail index since we want the snake to grow

			// TODO: spawn new food
		} else {
			int tail_x = history[tail_idx].x;
			int tail_y = history[tail_idx].y;
			grid[tail_x + tail_y * GRID_DIM] = none; // delete old tail

			// advance tail index
			tail_idx = (tail_idx + 1) % (GRID_DIM * GRID_DIM);
		}


		// PART III: GET INPUT
		EFI_INPUT_KEY Key;
		
		status = SystemTable->BootServices->CheckEvent(SystemTable->ConIn->WaitForKey);
		if (status == EFI_SUCCESS) {
			// a key has been pressed, read it
			SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);

			if (Key.ScanCode == SCAN_UP && snake_dir != south) {
				snake_dir = north;
			} else if (Key.ScanCode == SCAN_DOWN && snake_dir != north) {
				snake_dir = south;
			} else if (Key.ScanCode == SCAN_RIGHT && snake_dir != west) {
				snake_dir = east;
			} else if (Key.ScanCode == SCAN_LEFT && snake_dir != east) {
				snake_dir = west;
			}
		} // otherwise it's EFI_NOT_READY and we can just continue the loop



		// PART IV: DELAY
		status = SystemTable->BootServices->Stall(delay);
		if (EFI_ERROR(status)) {
			return status;
		}

		delay -= ((snake_length - 3) * 1000); // 1 ms quicker each time the score goes up
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
		L"\r\nUEFI Games v0.1.0\r\nbuild 34	\r\n==========================================\r\n\r\n");
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

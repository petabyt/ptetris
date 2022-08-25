#ifndef PTETRIS_H
#define PTETRIS_H

#include <stdint.h>
#include <stdio.h>

// Max array size
#ifndef PT_MAX_WIDTH
	#define PT_MAX_WIDTH 40
	#define PT_MAX_HEIGHT 40
#endif

// Darken the color around blocks
#ifndef PT_COL_DARKEN
	#define PT_COL_DARKEN(x) x
#endif

#ifndef PT_RAND
	uint32_t pt_rand_x = 0xab893919;
	int pt_rand(int max) {
		return pt_rand_x & (max - 1);
	}

	#define PT_RAND(x) pt_rand(x)
#endif

// Main runtime structure, tweak these in menu
struct PtRuntime {
	int width;
	int height;
	int block_size;
	int score;
}pt = {
	10,
	20,
	20,
	0,
};

#define BLOCK_LEN 4

// uint32_t pt_colors[BLOCK_LEN + 1] is expected to be defined

struct PtBlock {
	int curr;
	int x;
	int y;
	uint8_t b[][4][4];
}pt_blocks = {
	0, 0, 0,
	{
		{
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0},
			{0, 1, 0, 0},
		},
		{
			{0, 0, 0, 0},
			{0, 0, 2, 0},
			{0, 2, 2, 2},
			{0, 0, 0, 0},
		},
		{
			{0, 3, 0, 0},
			{0, 3, 0, 0},
			{0, 3, 3, 0},
			{0, 0, 0, 0},
		},
		{
			{0, 0, 0, 0},
			{0, 4, 4, 0},
			{0, 4, 4, 0},
			{0, 0, 0, 0},
		},
	}
};

struct PtField {
	uint8_t b[PT_MAX_WIDTH][PT_MAX_HEIGHT];
};

struct PtField pt_main_field = {0};

enum PtShift {
	PT_NORMAL = 0,
	PT_IMPOSSIBLE = 1,
	PT_SIDE_SWIPE = 2,
	PT_GAME_OVER = 3,
};

enum PtButton {
	PT_DOWN,
	PT_LEFT,
	PT_RIGHT,
	PT_ROT,
	PT_QUIT,
};

void pt_draw_block(int bx, int by, int col) {
	bx *= pt.block_size;
	by *= pt.block_size;

	for (int x = 0; x < pt.block_size; x++) {
		for (int y = 0; y < pt.block_size; y++) {
			if (x == 0 || y == 0 || y == pt.block_size - 1 || x == pt.block_size - 1) {
				pt_pixel(bx + x, by + y, PT_COL_DARKEN(pt_colors[col]));
			} else {
				pt_pixel(bx + x, by + y, pt_colors[col]);
			}
		}
	}
}

int pt_field_rotate() {
	uint8_t b[4][4];

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			b[x][y] = pt_blocks.b[pt_blocks.curr][3 - y][x];
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			pt_blocks.b[pt_blocks.curr][x][y] = b[x][y];
		}
	}
}

int pt_field_shift(int ox, int oy) {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (pt_blocks.b[pt_blocks.curr][x][y] == 0) continue;

			if (pt_blocks.b[pt_blocks.curr][x][y] != 0 &&
					pt_main_field.b[x + pt_blocks.x][y + pt_blocks.y] != 0) {
				return PT_GAME_OVER;
			}

			// Detect gravity/hit
			if (pt_main_field.b[x + pt_blocks.x][y + pt_blocks.y + oy] != 0
					|| (y + pt_blocks.y + oy) >= pt.height) {
				return PT_IMPOSSIBLE;
			}

			// Detect impossible side swipe
			if (pt_main_field.b[pt_blocks.x + x + ox][pt_blocks.y + y + oy] != 0
					|| (pt_blocks.x + x + ox) >= pt.width || (pt_blocks.x + x + ox) <= -1) {
				pt_blocks.y += oy;
				return PT_SIDE_SWIPE;
			}
		}
	}

	pt_blocks.x += ox;
	pt_blocks.y += oy;

	return PT_NORMAL;
}

void pt_render() {
	for (int x = 0; x < pt.width; x++) {
		for (int y = 0; y < pt.height; y++) {
			pt_draw_block(x, y, pt_main_field.b[x][y]);
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (pt_blocks.b[pt_blocks.curr][x][y] != 0) {
				pt_draw_block(x + pt_blocks.x, y + pt_blocks.y, pt_blocks.b[pt_blocks.curr][x][y]);
			}
		}
	}
	
}

int pt_handle_input(int key) {
	switch (key) {
	case PT_DOWN:
		return pt_field_shift(0, 1);
	case PT_LEFT:
		return pt_field_shift(-1, 0);
	case PT_RIGHT:
		return pt_field_shift(1, 0);
	case PT_ROT:
		return pt_field_rotate();
	case PT_QUIT:
		return PT_GAME_OVER;
	}
}

void pt_merge() {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (pt_blocks.b[pt_blocks.curr][x][y] == 0) continue;
			pt_main_field.b[x + pt_blocks.x][y + pt_blocks.y]
				= pt_blocks.b[pt_blocks.curr][x][y];
		}
	}	
}

void pt_shift_down(int yo) {
	for (int x = 0; x < pt.width; x++) {
		for (int y = yo - 1; y != 0; y--) {
			pt_main_field.b[x][y + 1] = pt_main_field.b[x][y];
		}
	}
}

void pt_check_lines() {
	for (int y = 0; y < pt.height; y++) {
		int full = 0;
		for (int x = 0; x < pt.width; x++) {
			if (pt_main_field.b[x][y] != 0) {
				full++;
			}
		}

		if (full == pt.width) {
			pt.score++;
			printf("Score: %d\n", pt.score);
			pt_shift_down(y);
		}
	}
}

int pt_step() {
	// Operate gravity
	switch (pt_field_shift(0, 1)) {
	case PT_IMPOSSIBLE:
		pt_merge();
		pt_blocks.curr = pt_rand(BLOCK_LEN);
		pt_blocks.x = pt.width / 2 - 3;
		pt_blocks.y = 0;
		break;
	case PT_GAME_OVER:
		return 1;
	}

	pt_check_lines();
	return 0;
}

#endif

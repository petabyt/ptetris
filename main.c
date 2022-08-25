//Make it so we don't need to include any other C files in our build.
#define CNFG_IMPLEMENTATION

#include "../rawdraw/CNFG.h"
#include "../rawdraw/os_generic.h"

void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

// TODO: runtime state config struct

#define PT_WIDTH 20
#define PT_HEIGHT 20

#define PT_SPAWN PT_WIDTH / 2
#define PT_BLOCK_SIZE 20

uint32_t pt_colors[5] = {
	0xc4c4c400,
	0x12345678,
	0x0026ff00,
	0xff110000,
	0xe600ff00,
};

uint8_t PtBlocks[][4][4] = {
	{
		{0, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 1, 0, 0},
	},
	{
		{0, 0, 0, 0},
		{0, 2, 0, 0},
		{2, 2, 2, 0},
		{0, 0, 0, 0},
	},
	{
		{0, 3, 0, 0},
		{0, 3, 0, 0},
		{0, 3, 0, 0},
		{0, 3, 3, 0},
	},

};

struct PtField {
	uint8_t b[PT_WIDTH][PT_HEIGHT];
};

struct PtField pt_main_field = {0};
struct PtField pt_temp_field = {0};

void pt_pixel(int x, int y, int col) {
	CNFGColor(col);
	CNFGTackPixel(x, y);
}

uint32_t pt_rand_x = 0x84742792;
int pt_rand(int max) {
	return pt_rand_x & (max - 1);
}

void pt_draw_block(int bx, int by, int col) {
	bx *= PT_BLOCK_SIZE;
	by *= PT_BLOCK_SIZE;

	for (int x = 0; x < PT_BLOCK_SIZE; x++) {
		for (int y = 0; y < PT_BLOCK_SIZE; y++) {
			if (x == 0 || y == 0 || y == PT_BLOCK_SIZE - 1 || x == PT_BLOCK_SIZE - 1) {
				pt_pixel(bx + x, by + y, pt_colors[col] / 0x2);
			} else {
				pt_pixel(bx + x, by + y, pt_colors[col]);
			}
		}
	}
}

void pt_draw_shape(int shape) {
	uint8_t c = 2;
	switch (shape) {
		case 0:
			pt_temp_field.b[PT_SPAWN][0] = c;
			pt_temp_field.b[PT_SPAWN][1] = c;
			pt_temp_field.b[PT_SPAWN][2] = c;
			pt_temp_field.b[PT_SPAWN][3] = c;
			break;
		case 1:
			pt_temp_field.b[PT_SPAWN][0] = c;
			pt_temp_field.b[PT_SPAWN][1] = c;
			pt_temp_field.b[PT_SPAWN + 1][1] = c;
			pt_temp_field.b[PT_SPAWN - 1][1] = c;
			break;
		case 2:
			pt_temp_field.b[PT_SPAWN - 2][0] = c;
			pt_temp_field.b[PT_SPAWN - 1][0] = c;
			pt_temp_field.b[PT_SPAWN][0] = c;
			pt_temp_field.b[PT_SPAWN + 1][0] = c;
			pt_temp_field.b[PT_SPAWN + 1][1] = c;
			break;

	}
}

enum PtShift {
	PT_NORMAL = 0,
	PT_IMPOSSIBLE = 1,
	PT_SIDE_SWIPE = 2,
	PT_GAME_OVER = 3,
};

void pt_merge() {
	for (int x = 0; x < PT_WIDTH; x++) {
		for (int y = 0; y < PT_HEIGHT; y++) {
			if (pt_temp_field.b[x][y] != 0) {
				pt_main_field.b[x][y] = pt_temp_field.b[x][y];
			}

			pt_temp_field.b[x][y] = 0;
		}
	}	
}

void pt_copy(struct PtField *dest, struct PtField *src) {
	for (int i = 0; i < (int)sizeof(struct PtField); i++) {
		((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
	}
}

int pt_field_rotate() {
	struct PtField field_tmp = {0};

	// Get start of block
	int bx = 0;
	int by = 0;
	for (int x = 0; x < PT_WIDTH; x++) {
		for (int y = 0; y < PT_HEIGHT; y++) {
			if (pt_temp_field.b[x][y] != 0) {
				if (x > bx) bx = x;
				if (y > by) by = y;
			}
		}
	}

	bx -= 3;
	by -= 1;

	printf("%d,%d\n", bx, by);

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			field_tmp.b[y + bx][x + by] = pt_temp_field.b[x + bx][y + by];
		}
	}

	pt_copy(&pt_temp_field, &field_tmp);
}

int pt_field_shift(int ox, int oy) {
	struct PtField field_tmp = {0};
	for (int x = 0; x < PT_WIDTH; x++) {
		for (int y = 0; y < PT_HEIGHT; y++) {
			if (pt_temp_field.b[x][y] == 0) continue;

			// Impossible overlap
			if (pt_temp_field.b[x][y] != 0 && pt_main_field.b[x][y] != 0) {
				return PT_GAME_OVER;
			}

			// Detect gravity collision
			if (pt_main_field.b[x][y + oy] != 0 || y + oy >= PT_HEIGHT) {
				return PT_IMPOSSIBLE;
			}

			// Detect impossible side swipe
			if (pt_main_field.b[x + ox][y + oy] != 0 || x + ox >= PT_WIDTH || x + ox <= -1) {
				return PT_SIDE_SWIPE;
			}

			// Shift block
			if (pt_temp_field.b[x][y] != 0) {
				field_tmp.b[x + ox][y + oy] = pt_temp_field.b[x][y];
			}
		}
	}

	pt_copy(&pt_temp_field, &field_tmp);

	return PT_NORMAL;
}

void pt_render() {
	for (int x = 0; x < PT_WIDTH; x++) {
		for (int y = 0; y < PT_HEIGHT; y++) {
			pt_draw_block(x, y, pt_main_field.b[x][y]);
			if (pt_temp_field.b[x][y] != 0) {
				pt_draw_block(x, y, pt_temp_field.b[x][y]);
			}
		}
	}	
}

enum PtButton {
	PT_DOWN,
	PT_LEFT,
	PT_RIGHT,
	PT_ROT,
	PT_QUIT,
};

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

void HandleKey( int keycode, int bDown ) {
	if( keycode == CNFG_KEY_ESCAPE ) exit( 0 );
	printf( "Key: %d -> %d\n", keycode, bDown );
	if (bDown != 0) return;

	// mess with random number
	pt_rand_x += keycode;

	switch (keycode) {
	case 65361:
		pt_handle_input(PT_LEFT);
		return;
	case 65363:
		pt_handle_input(PT_RIGHT);
		return;
	case 65362:
		pt_handle_input(PT_ROT);
	}
}

int main() {
	int frame = 0;

	// Initial shape
	pt_draw_shape(pt_rand(3));

	CNFGSetup("ptetris", 640, 480);

	while(CNFGHandleInput()) {
		CNFGClearFrame();

		pt_render();

		// Basic frame skip for smooth input
		frame++;
		if (frame <= 5) goto end;
		frame = 0;

		// Operate gravity
		switch (pt_field_shift(0, 1)) {
		case PT_IMPOSSIBLE:
			pt_merge();
			pt_draw_shape(pt_rand(3));
			break;
		case PT_GAME_OVER:
			return 0;
		}

		end:;
		CNFGSwapBuffers();
	}
}

#define CNFG_IMPLEMENTATION

#include "../rawdraw/CNFG.h"
#include "../rawdraw/os_generic.h"

void pt_pixel(int x, int y, int col) {
	CNFGColor(col);
	CNFGTackPixel(x, y);
}

uint32_t pt_colors[5] = {
	0xc4c4c400, // background color
	0xff110000,
	0x0026ff00,
	0xffe60000,
	0xcc42eb00,
};

#include "ptetris.h"

void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

void HandleKey( int keycode, int bDown ) {
	// mess with random number
	pt_rand_x += keycode + bDown;

	if( keycode == CNFG_KEY_ESCAPE ) exit( 0 );
	//printf( "Key: %d -> %d\n", keycode, bDown );
	if (bDown != 1) return;

	switch (keycode) {
	case 65361:
		pt_handle_input(PT_LEFT);
		return;
	case 65363:
		pt_handle_input(PT_RIGHT);
		return;
	case 65362:
		pt_handle_input(PT_ROT);
		return;
	case 65364:
		pt_handle_input(PT_DOWN);
		return;
	}

	pt_render();
}


int main() {
	int frame = 0;

	CNFGSetup("ptetris", 640, 480);

	while(CNFGHandleInput()) {
		CNFGClearFrame();

		pt_render();

		CNFGColor(-1);
		CNFGPenY = 50;
		CNFGPenX = 250;
		char buffer[128];
		snprintf(buffer, 128, "Score: %d", pt.score);
		CNFGDrawText(buffer, 10);

		// Basic frame skip for smooth input
		frame++;
		if (frame <= 30) goto end;
		frame = 0;

		if (pt_step()) {
			return 0;
		}

		end:;
		CNFGSwapBuffers();
		OGUSleep(0x1000);
	}
}

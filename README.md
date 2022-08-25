# Portable Tetris

Meant to be extremely portable, so it can be  
ported to nearly any platform with little effort.

## Required Code:
```
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
```

## API
### `pt_handle_input(PT_LEFT);`
Handle an input once. Will perform logic.
### `pt_render()`
Render game, do no logic. Adjust pt_pixel to your liking.
## `pt_step()`
Perform gravity operation. See PtShift for return codes.

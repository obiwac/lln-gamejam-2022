#include "log.h"
#include "win.h"

int draw(void* param) {
	win_t* win = param;

	return 0;
}

int main(void) {
	LOG("Hello!\n")

	win_t* win = create_win(800, 480);

	win_loop(win, draw, win);

	LOG("Done\n");
	return 0;
}

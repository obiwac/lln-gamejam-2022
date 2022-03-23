#include "log.h"
#include "win.h"

int main(void) {
	LOG("Hello!\n")

	win_t* win = create_win(800, 480);

	LOG("Done\n");
	return 0;
}

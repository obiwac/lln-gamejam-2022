#include "gl/gl.h"

#include "log.h"
#include "win.h"

#define GL_REQUIRE(game, name) (game)->gl.name = (void*) eglGetProcAddress("gl"#name);

typedef struct {
	win_t* win;
	gl_funcs_t gl;
} game_t;

int draw(void* param) {
	game_t* self = param;
	gl_funcs_t* gl = &self->gl;

	gl->ClearColor(1.0, 0.0, 1.0, 1.0);
	gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return 0;
}

static void gl_func_not_loaded(void) {
	WARN("OpenGL function not yet loaded. Use 'GL_REQUIRE'.\n")
}

int main(void) {
	LOG("Hello!\n")

	game_t game = {
		.win = create_win(800, 480)
	};

	// load OpenGL functions manually

	for (int i = 0; i < sizeof(game.gl) / sizeof(game.gl.Accum); i++) {
		((void**) &game.gl)[i] = gl_func_not_loaded;
	}

	GL_REQUIRE(&game, ClearColor)
	GL_REQUIRE(&game, Clear)

	win_loop(game.win, draw, &game);

	LOG("Done\n");
	return 0;
}

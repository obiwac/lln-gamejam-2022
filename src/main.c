#include "gl/gl.h"

#include "log.h"
#include "win.h"

#include "vertex.h"
#include "shader.h"
#include "object.h"

#define GL_REQUIRE(game, name) (game)->gl.name = (void *)eglGetProcAddress("gl" #name);

typedef struct
{
	win_t *win;
	gl_funcs_t gl;
} game_t;

static object_t *testTriangle;

int draw(void *param)
{
	game_t *self = param;
	gl_funcs_t *gl = &self->gl;

	gl->ClearColor(1.0, 0.0, 1.0, 1.0);
	gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render_object(gl, testTriangle);
	return 0;
}

static void gl_func_not_loaded(void)
{
	WARN("OpenGL function not yet loaded. Use 'GL_REQUIRE'.\n")
}

int main(void)
{

	game_t game = {
		.win = create_win(800, 480)};

	// load OpenGL functions manually

	for (int i = 0; i < sizeof(game.gl) / sizeof(game.gl.Accum); i++)
	{
		((void **)&game.gl)[i] = gl_func_not_loaded;
	}

	GL_REQUIRE(&game, ClearColor)
	GL_REQUIRE(&game, Clear)
	GL_REQUIRE(&game, GenBuffers)
	GL_REQUIRE(&game, BindBuffer)
	GL_REQUIRE(&game, BufferData)
	GL_REQUIRE(&game, CreateShader)
	GL_REQUIRE(&game, ShaderSource)
	GL_REQUIRE(&game, CompileShader)
	GL_REQUIRE(&game, GetShaderiv)
	GL_REQUIRE(&game, GetShaderInfoLog)
	GL_REQUIRE(&game, CreateProgram)
	GL_REQUIRE(&game, AttachShader)
	GL_REQUIRE(&game, LinkProgram)
	GL_REQUIRE(&game, DeleteShader)
	GL_REQUIRE(&game, VertexAttribPointer)
	GL_REQUIRE(&game, EnableVertexAttribArray)
	GL_REQUIRE(&game, BindVertexArray)
	GL_REQUIRE(&game, DrawArrays)
	GL_REQUIRE(&game, GenVertexArrays)
	GL_REQUIRE(&game, UseProgram)
	GL_REQUIRE(&game, BindAttribLocation)

	testTriangle = create_object(&game.gl, create_shader(&game.gl, "default"));

	win_loop(game.win, draw, &game);

	LOG("Done\n");
	return 0;
}

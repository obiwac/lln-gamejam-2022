#include "gl/gl.h"

#include "log.h"
#include "win.h"

#include "vertex.h"
#include "shader.h"
#include "object.h"

#include "matrix.h"

#define GL_REQUIRE(game, name) (game)->gl.name = (void *)eglGetProcAddress("gl" #name);

typedef struct
{
	win_t *win;
	gl_funcs_t gl;

	matrix_t p_matrix;
	matrix_t mv_matrix;
	matrix_t mvp_matrix;
} game_t;

static object_t *testTriangle;
static matrix_t mvp;
int draw(void *param)
{
	game_t *self = param;
	gl_funcs_t *gl = &self->gl;

	gl->ClearColor(1.0, 0.0, 1.0, 1.0);
	gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader_uniform(testTriangle->shader, "tint", ((float[4]) { 0.0, 0.0, 1.0, 1.0 }));

	// projection matrix

	matrix_identity(self->p_matrix);
	matrix_perspective(self->p_matrix, 90, (float) self->win->x_res / self->win->y_res, 0.1, 500);

	// model-view matrix

	matrix_identity(self->mv_matrix);
	matrix_translate(self->mv_matrix, (float[3]) { 0, 0, -1 });
	matrix_rotate_2d(self->mv_matrix, (float[2]) { 6.28 / 6, 0 });

	// model-view-projection matrix

	matrix_multiply(self->mvp_matrix, self->p_matrix, self->mv_matrix);

	shader_uniform(testTriangle->shader, "mvp_matrix", &self->mvp_matrix);

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
	GL_REQUIRE(&game, GetUniformLocation)
	GL_REQUIRE(&game, UniformMatrix4fv)
	GL_REQUIRE(&game, Uniform4fv)
	GL_REQUIRE(&game, Uniform1f)
	GL_REQUIRE(&game, Uniform1i)

<<<<<<< HEAD
	shader_t* shader = create_shader(&game.gl, "default");
=======
	shader_t *shader = create_shader(&game.gl, "default");
	game.mvp_matrix_uniform = shader_uniform_location(shader, "mvp_matrix");
	game.tint_uniform = shader_uniform_location(shader, "tint");
>>>>>>> 01124794c95b679419ec37686c5d0f8675b331a0

	testTriangle = create_object(&game.gl, shader);

	win_loop(game.win, draw, &game);

	LOG("Done\n");
	return 0;
}

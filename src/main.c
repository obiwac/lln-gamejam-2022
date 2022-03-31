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

static float x = 0;

int draw(void *param, float dt)
{
	game_t *self = param;
	gl_funcs_t *gl = &self->gl;

	LOG("FPS: %f\n", 1 / dt)

	char caption[256];
	snprintf(caption, sizeof caption, "Gamejam 2022 (%d FPS)", (int) (1 / dt));
	win_set_caption(self->win, caption);

	// clear buffers

	gl->ClearColor(0.1, 0.1, 0.1, 1.0);
	gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader_uniform(testTriangle->shader, "tint", ((float[4]) { 0.0, 0.0, 1.0, 1.0 }));

	// projection matrix

	matrix_identity(self->p_matrix);
	matrix_perspective(self->p_matrix, 90, (float) self->win->x_res / self->win->y_res, 0.1, 500);

	// model-view matrix

	x += dt;

	matrix_identity(self->mv_matrix);
	matrix_translate(self->mv_matrix, (float[3]) { 0, 0, -1 });
	matrix_rotate_2d(self->mv_matrix, (float[2]) { x, sin(x * 5 / 3) / 3 });


	// model-view-projection matrix

	matrix_multiply(self->mvp_matrix, self->p_matrix, self->mv_matrix);
	
	shader_uniform(testTriangle->shader, "mvp_matrix", &self->mvp_matrix);

	//Set textures
	gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D,testTriangle->tex_albedo);

	render_object(gl, testTriangle);

	return 0;
}

static void gl_func_not_loaded(void)
{
	WARN("OpenGL function not yet loaded. Use 'GL_REQUIRE'.\n")
}

int main(int argc, char** argv)
{

	// TODO debugging arguments for stuff like verbose logging

	game_t game = {
		.win = create_win(800, 480)
	};

	game.win->gl = &game.gl;

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
	GL_REQUIRE(&game, Viewport)
	GL_REQUIRE(&game, DrawElements)
	GL_REQUIRE(&game, BindTexture)
	GL_REQUIRE(&game, TexImage2D)
	GL_REQUIRE(&game, GenTextures)
	GL_REQUIRE(&game, TexParameteri)
	GL_REQUIRE(&game, ActiveTexture)
	GL_REQUIRE(&game, GenerateMipmap)

	shader_t* shader = create_shader(&game.gl, "default");
	
	testTriangle = create_object(&game.gl, shader);
	
	//Load Checkboard texture for the triangle:
	testTriangle->tex_albedo = loadTexture2D(&game.gl,"rsc/Textures/checkboard.png");

	win_loop(game.win, draw, &game);

	LOG("Done\n");
	return 0;
}

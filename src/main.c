#include "gl/gl.h"

#define TAU 6.28318

#include "log.h"
#include "win.h"
#include "matrix.h"
#include "vertex.h"
#include "model_loader.h"
#include "player.h"

// forward declarations

typedef struct fbo_t fbo_t;

typedef struct {
	win_t *win;
	gl_funcs_t gl;

	matrix_t p_matrix;
	matrix_t mv_matrix;
	matrix_t mvp_matrix;

	// entities

	player_t* player;
	// TODO list of entities

	// fbos

	fbo_t* combine_fbo;

	// maintain list of fbos for resizing

	fbo_t** fbos;
	size_t fbo_count;
} game_t;

#include "events.h"

#include "vertex.h"
#include "shader.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

#define GL_REQUIRE(game, name) (game)->gl.name = (void *)eglGetProcAddress("gl" #name);

static object_t* default_quad;
static skybox_t* sky;

void draw_skybox(game_t* self,skybox_t* skybox)
{
	gl_funcs_t* gl = &self->gl;

	matrix_t viewmat,mvp;
	matrix_copy(viewmat,self->mv_matrix);

	viewmat[3][0] = 0.0f;
	viewmat[3][1] = 0.0f;
	viewmat[3][2] = 0.0f;

	matrix_multiply(mvp, self->p_matrix, viewmat);
	shader_uniform(skybox->object->shader, "mvp_ma", &mvp);

	gl->DepthFunc(GL_EQUAL);
	gl->DepthMask(GL_FALSE);
	gl->BindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);

	render_object(&self->gl, skybox->object);

	gl->DepthMask(GL_TRUE);
	gl->DepthFunc(GL_LESS);
	gl->Enable(GL_DEPTH_TEST);
}

int draw(void *param, float dt)
{
	game_t *self = param;
	gl_funcs_t *gl = &self->gl;

	if (dt > 1. / 20) {
		dt = 1. / 60;
	}

	// LOG("FPS: %f\n", 1 / dt)

	// WINDOW SHIT

	char caption[256];
	snprintf(caption, sizeof caption, "Gamejam 2022 (%d FPS)", (int) (1 / dt));
	win_set_caption(self->win, caption);

	use_fbo(self->combine_fbo);

	// PHYSICS SHIT

	update_mouse(self);
	player_update(self->player, dt);

	// RENDER SHIT

	// Draw Skybox:

	draw_skybox(self,sky);

	// projection matrix

	matrix_identity(self->p_matrix);
	matrix_translate(self->mv_matrix,(float[3]){dt,0,1});
	matrix_perspective(self->p_matrix, 90, (float) self->win->x_res / self->win->y_res, 0.1, 500);

	// model-view matrix

	matrix_identity(self->mv_matrix);

	matrix_rotate_2d(self->mv_matrix, (float[2]) { self->player->entity.rot[0] + TAU / 4, self->player->entity.rot[1] });
	matrix_translate(self->mv_matrix, (float[3]) { -self->player->entity.pos[0], -self->player->entity.pos[1], -self->player->entity.pos[2] });

	// model-view-projection matrix
	matrix_multiply(self->mvp_matrix, self->p_matrix, self->mv_matrix);

	for (size_t i = 0; i < render_object_count; i++) {
		matrix_transform(object_a[i]->transform_matrix, object_a[i]->transform.translation, object_a[i]->transform.rotation, object_a[i]->transform.scale);

		shader_uniform(object_a[i]->shader, "transform_matrix", &object_a[i]->transform_matrix);
		shader_uniform(object_a[i]->shader, "tint", ((float[4]) { 0.0, 0.0, 1.0, 1.0 }));
		shader_uniform(object_a[i]->shader, "mvp_matrix", &self->mvp_matrix);

		gl->ActiveTexture(GL_TEXTURE0);
		gl->BindTexture(GL_TEXTURE_2D, object_a[i]->tex_albedo);

		render_object(gl, object_a[i]);
	}

	//Combine All Texture for final result
	gl->BindFramebuffer(GL_FRAMEBUFFER,0);
	gl->ClearColor(0.7, 0.1, 0.1, 1.0);
    gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gl->BindTexture(GL_TEXTURE_2D,self->combine_fbo->colour_texture);

	render_object(gl, default_quad);

	return 0;
}

int resize(void* param, uint32_t x_res, uint32_t y_res) {
	game_t* self = param;
	gl_funcs_t* gl = &self->gl;

	// resize FBO textures

	for (size_t i = 0; i < self->fbo_count; i++) {
		fbo_resize(self->fbos[i]);
	}

	return 0;
}

skybox_t* create_skybox(gl_funcs_t* gl)
{
		//Create a cube - "cube"map :)
	vertex_t cube[] =
	 {
	 	{.position = {-1.0f, -1.0, -1.0f}},
	 	{.position = {1.0f, -1.0f, -1.0f}},
	 	{.position = {1.0f, 1.0f, -1.0f}},
	 	{.position = {-1.0f,  1.0f, -1.0f}},
	 	{.position = {-1.0f,  -1.0f, 1.0f}},
	 	{.position = {1.0f,  -1.0f, 1.0f}},
	 	{.position = {1.0f,  1.0f, 1.0f}},
	 	{.position = {-1.0f,  1.0f, 1.0f}}
	 };
	uint32_t indices[6 * 6] =
	{
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
	};
	skybox_t* self =  (skybox_t *)calloc(1, sizeof(*self));
	shader_t* shader = create_shader(gl, "skybox");
	self->texture =  loadcubemap(gl,"rsc/Textures/skybox/%s");
	self->object =   create_object(gl, shader,true,cube,sizeof(cube),indices,sizeof(indices));
	return self;
}

static void gl_func_not_loaded(void)
{
	WARN("OpenGL function not yet loaded. Use 'GL_REQUIRE'.\n")
}

int main(int argc, char** argv)
{

	// TODO debugging arguments for stuff like verbose logging

	win_t* win = create_win(800, 480);

	game_t game = {
		.win = win,
		.player = new_player()
	};

	gl_funcs_t *gl = &game.gl;
	win->gl = gl;

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
	GL_REQUIRE(&game, GenFramebuffers)
	GL_REQUIRE(&game, BindFramebuffer)
	GL_REQUIRE(&game, FramebufferTexture2D)
	GL_REQUIRE(&game, DepthMask)
	GL_REQUIRE(&game, GetError)
	GL_REQUIRE(&game, DeleteTextures)
	GL_REQUIRE(&game, Enable)
	GL_REQUIRE(&game, Disable)
	GL_REQUIRE(&game, DepthFunc)

	sky = create_skybox(&game.gl);
	shader_t* shader = create_shader(&game.gl, "default");
	//Todo mode loading
	vertex_t vert[] =
	 {
	 	{.position = {-0.5f, -0.5f, 0.0f},.texcoord = {0.0f,0.0f},.normal ={1.0f,1.0f,1.0f}},
	 	{.position = {0.5f, -0.5f, 0.0f},.texcoord = {1.0f,0.0f},.normal ={1.0f,1.0f,1.0f}},
	 	{.position = {0.0f, 0.5f, 0.0f},.texcoord = {0.5f,1.0f},.normal ={1.0f,1.0f,1.0f}},
	 };
	unsigned int indices[3] = {0,1,2};

	object_t *testTriangle;
	//testTriangle = create_object(&game.gl, shader,true,vert,sizeof(vert),indices,sizeof(indices));
	//testTriangle->tex_albedo = loadTexture2D(&game.gl,"rsc/Textures/checkboard.png");


	vertex_t quad[] =
	 {
	 	{.position = {1.0f,  1.0, 0.0f},.texcoord = {1.0f, 1.0f},.normal={1.0,1.0,1.0}},
	 	{.position = {1.0f, -1.0f, 0.0f},.texcoord = {1.0f, 0.0f},.normal={1.0,1.0,1.0}},
	 	{.position = {-1.0f, -1.0f, 0.0f},.texcoord = {0.0f, 0.0f},.normal={1.0,1.0,1.0}},
	 	{.position = {-1.0f,  1.0f, 0.0f},.texcoord = {0.0f, 1.0f},.normal={1.0,1.0,1.0}}
	 };
	unsigned int q_incides[6] = {0,1,3,1,2,3};
	shader_t* combine_shader = create_shader(&game.gl, "combine");
	default_quad = create_object(&game.gl, combine_shader, false, quad, sizeof(quad), q_incides, sizeof(q_incides));

	game.combine_fbo = new_fbo(&game, 1.0, 1.0);

	// register callbacks & start gameloop

	load_model(&game.gl,"obj-to-ivx/output.ivx",shader,true);

	win->draw_cb = draw;
	win->draw_param = &game;

	win->resize_cb = resize;
	win->resize_param = &game;

	win->keypress_cb = keypress;
	win->keypress_param = &game;

	win->keyrelease_cb = keyrelease;
	win->keyrelease_param = &game;

	win_loop(win);

	LOG("Done\n");
	return 0;
}

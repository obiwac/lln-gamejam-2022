#include "gl/gl.h"

#include <stdbool.h>
void die(bool hard);
#define TAU 6.28318

#include "log.h"
#include "win.h"
#include "matrix.h"
#include "vertex.h"
#include "model_loader.h"
#include "skybox.h"
#include "collider.h"

// forward declarations

typedef struct fbo_t fbo_t;
typedef struct player_t player_t;
typedef struct entity_t entity_t;

typedef struct
{
	matrix_t shake_mat;
	float shake_intensity;
	int time;
	bool shake;
}shaker_t;

typedef struct {
	win_t *win;
	gl_funcs_t gl;

	matrix_t p_matrix;
	matrix_t mv_matrix;
	matrix_t mvp_matrix;

	shader_t* shader;
	shader_t* entity_shader;

	// entities

	player_t* player;

	size_t entity_count;
	entity_t** entities;

	// colliders

	size_t collider_count;
	collider_t** colliders;

	// fbos

	fbo_t* combine_fbo;

	// maintain list of fbos for resizing

	fbo_t** fbos;
	size_t fbo_count;

	shaker_t shaker;

	float time;

	float frame_tint[4];

	bool fire;
	bool dirt;

} game_t;

#include "player.h"
#include "events.h"
#include "vertex.h"
#include "shader.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

#define GL_REQUIRE(game, name) (game)->gl.name = (void *)eglGetProcAddress("gl" #name);

static object_t* default_quad;
static skybox_t* sky;

static entity_t* firefighter = NULL;
static entity_t* villager = NULL;
static entity_t* nain = NULL;

entity_t* add_entity(game_t* self, const char* model, const char* texture, void (*ai_cb) (entity_t* entity, float dt)) {
	gl_funcs_t* gl = &self->gl;
	entity_t* entity = new_entity(self);

	entity->ai_cb = ai_cb;

	entity->object = load_model(gl, model, self->entity_shader, true);
	entity->object->tex_albedo = loadTexture2D(gl, texture);

	self->entities = realloc(self->entities, ++self->entity_count * sizeof *self->entities);
	self->entities[self->entity_count - 1] = entity;

	return entity;
}

int draw(void *param, float dt)
{
	game_t *self = param;
	gl_funcs_t *gl = &self->gl;

	if (dt > 1. / 20) {
		dt = 1. / 60;
	}

	self->time += dt;

	// LOG("FPS: %f\n", 1 / dt)

	// WINDOW SHIT

	char caption[256];
	snprintf(caption, sizeof caption, "Gamejam 2022 (%d FPS)", (int) (1 / dt));
	win_set_caption(self->win, caption);

	use_fbo(self->combine_fbo);

	// PHYSICS SHIT

	update_mouse(self);
	player_update(self->player, self->collider_count, self->colliders, dt);

	for (size_t i = 0; i < self->entity_count; i++) {
		entity_t* entity = self->entities[i];
		entity_update(entity, self->collider_count, self->colliders, dt);
	}

	// projection matrix

	matrix_identity(self->p_matrix);
	matrix_translate(self->mv_matrix,(float[3]){dt,0,1});
	matrix_perspective(self->p_matrix, 90, (float) self->win->x_res / self->win->y_res, 0.1, 500);

	// model-view matrix

	matrix_identity(self->mv_matrix);

	matrix_rotate_2d(self->mv_matrix, (float[2]) { self->player->entity.rot[0] + TAU / 4, self->player->entity.rot[1] });
	matrix_translate(self->mv_matrix, (float[3]) { -self->player->entity.pos[0], -self->player->entity.pos[1] - self->player->eyelevel, -self->player->entity.pos[2] });

	// model-view-projection matrix
	matrix_multiply(self->mvp_matrix, self->p_matrix, self->mv_matrix);

	// RENDER SHIT

	// Draw Skybox:
	draw_skybox(&self->gl,self->p_matrix,self->mv_matrix,sky);

	for (size_t i = 0; i < render_object_count; i++) {
		matrix_transform(object_a[i]->transform_matrix, object_a[i]->transform.translation, object_a[i]->transform.rotation, object_a[i]->transform.scale);

		shader_uniform(object_a[i]->shader, "transform_matrix", &object_a[i]->transform_matrix);
		shader_uniform(object_a[i]->shader, "tint", ((float[4]) { 0.0, 0.0, 1.0, 1.0 })) ;
		shader_uniform(object_a[i]->shader, "time", self->time);
		shader_uniform(object_a[i]->shader, "mvp_matrix", &self->mvp_matrix);

		shader_uniform(object_a[i]->shader, "camera_pos", ((float[4]) {
			self->player->entity.pos[0],
			self->player->entity.pos[1],
			self->player->entity.pos[2],
			1.0,
		}));

		// texture uniforms

		gl->ActiveTexture(GL_TEXTURE0);
		gl->BindTexture(GL_TEXTURE_2D, object_a[i]->tex_albedo);

		shader_uniform(object_a[i]->shader, "tex_albedo", (uint32_t) 0);

		gl->ActiveTexture(GL_TEXTURE1);
		gl->BindTexture(GL_TEXTURE_CUBE_MAP, sky->texture);

		shader_uniform(object_a[i]->shader, "skybox", (uint32_t) 1);

		// render object

		render_object(gl, object_a[i]);
	}

	//Combine All Texture for final result
	gl->BindFramebuffer(GL_FRAMEBUFFER,0);
	gl->ClearColor(0.1, 0.1, 0.1, 1.0);
    gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D, self->combine_fbo->colour_texture);
	matrix_identity(self->shaker.shake_mat);
	if(self->shaker.shake)
	{
		matrix_translate(self->shaker.shake_mat,(float[3]){sin(rand())/self->shaker.shake_intensity,sin(rand())/self->shaker.shake_intensity,-100});
	}
	shader_uniform(default_quad->shader,"frame_tint",self->frame_tint);
	shader_uniform(default_quad->shader,"shake_mat",&self->shaker.shake_mat);
	render_object(gl, default_quad);

	// triggers

	if (self->player->entity.pos[1] < -50) {
		die(true);
	}

	if (!villager && self->player->entity.pos[0] > 7) {
		self->dirt = true;

		villager = add_entity(self, "rsc/villager.ivx", "rsc/villager.png", villager_ai);

		villager->pos[0] = 28;
		villager->pos[1] = 5;
		villager->pos[2] = 0;

		villager->jump_height = 5;
		villager->rot[0] = -TAU / 4;

		self->player->entity.vel[0] = 20;
		self->player->entity.vel[1] = sqrt(-2 * GRAVITY * 10);
	}

	if (villager && !villager->dead) {
		for (size_t i = 0; i < self->entity_count; i++) {
			entity_t* entity = self->entities[i];

			if (entity->ai_cb != pig_ai) {
				continue;
			}

			if (!entity->dead) {
				goto skip;
			}
		}

		villager->dead = true;
		self->dirt = false;

		skip: {}
	}

	if (villager && villager->dead && !firefighter && self->player->entity.pos[0] > 31) {
		firefighter = add_entity(self, "rsc/firefighter.ivx", "rsc/cardboard.png", firefighter_ai);

		firefighter->pos[0] = 51;
		firefighter->pos[1] = 5;
		firefighter->pos[2] = 4;

		firefighter->rot[0] = TAU / 2;
		firefighter->jump_height = 5;

		self->fire = true;

		self->player->entity.vel[0] = 30;
		self->player->entity.vel[1] = sqrt(-2 * GRAVITY * 10);
	}

	if(self->fire){
	if(dt * SMOOTHING > 1){
		self->frame_tint[0] = 0.94;
		self->frame_tint[1] = 0.5;
		self->frame_tint[2] = 0.180;
	}else
	{
		self->frame_tint[0] += (0.94 - self->frame_tint[0] * dt * SMOOTHING);
		self->frame_tint[1] += (0.5 - self->frame_tint[1] * dt * SMOOTHING);
		self->frame_tint[2] += (0.180 - self->frame_tint[2] * dt * SMOOTHING);
	}
	}else if (self->dirt)
	{
		self->shaker.shake = true;
		self->shaker.shake_intensity =  10;
	}else
	{
		self->frame_tint[0] = 1;
		self->frame_tint[1] = 1;
		self->frame_tint[2] = 1;
		self->frame_tint[3] = 1;
		self->shaker.shake = false;
	}

	if (firefighter && firefighter->dead && !nain && self->player->entity.pos[0] > 57) {
		self->fire = false;
		nain = add_entity(self, "rsc/nain.ivx", "rsc/tete.png", nain_ai);

		self->player->entity.vel[0] = 30;
		self->player->entity.vel[1] = sqrt(-2 * GRAVITY * 10);

		nain->pos[0] = 84;
		nain->pos[1] = 1;
		nain->pos[2] = -5.5;

		nain->object->transform.scale[0] = 0.1;
		nain->object->transform.scale[1] = 0.1;
		nain->object->transform.scale[2] = 0.1;
	}

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

void add_collider(game_t* self, float pos1[3], float pos2[3]) {
	self->colliders = realloc(self->colliders, ++self->collider_count * sizeof *self->colliders);
	self->colliders[self->collider_count - 1] = new_collider(pos1, pos2);
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
		.win = win
	};

	game.player = new_player(&game);

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
	game.shader = create_shader(&game.gl, "default");
	game.entity_shader = create_shader(&game.gl, "entity");
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
	game.shaker.shake_intensity = 10;
	game.shaker.shake = false;

	game.frame_tint[0] = 1.0f;
	game.frame_tint[1] = 1.0f;
	game.frame_tint[2] = 1.0f;
	game.frame_tint[3] = 1.0f;


	game.fire = false;
	game.dirt = false;

	// add colliders


	add_collider(&game, (float[3]) { -6.8, 0, -4.2 }, (float[3]) { 4.2, -0.1, 4.4 }); //First island
	add_collider(&game, (float[3]) { 17.5, 0, -5.9 }, (float[3]) { 30.5, -0.1, 6.8 }); //Second island --- 2 Z + 2
	add_collider(&game, (float[3]) { 43.9, 0, -6.4 }, (float[3]) { 57.9, -0.1, 6.1 }); //Third island
	add_collider(&game, (float[3]) { 72.1, 0, -9.2 }, (float[3]) { 85.1, -0.1, 9.3}); //4 island


	add_collider(&game, (float[3]) { -7.5f, 0, -0.16 }, (float[3]) { -6.0, 5, 0.4}); //Pillar fuse


	add_collider(&game, (float[3]) { -4.4f, 0, 1.2 }, (float[3]) { -3.01, 5, 2}); //Pillar fuse2

	add_collider(&game, (float[3]) { -4.3f, 0, -1.9 }, (float[3]) { -2.7, 5, -1.2}); //Pillar fuse3



	add_collider(&game, (float[3]) { 45.0f, 0, 2.1 }, (float[3]) { 55.3, 2.5, 6.1}); //Volcan  µ



	add_collider(&game, (float[3]) { 74.5f, 0, 4.5 }, (float[3]) { 75.73, 1.8, 5.1}); //crystale droite

	add_collider(&game, (float[3]) { 80.4f, 0, 5.3}, (float[3]) { 81.4, 3, 6.5}); //crystale droite plus loin

	add_collider(&game, (float[3]) { 79.4f, 0, 0.2 }, (float[3]) { 81.2, 4, 1.5}); //crystale droite plus loin loin  µ
	add_collider(&game, (float[3]) { 77.8f, 0, -7.05 }, (float[3]) { 83.2, 4, -3.6}); //gros crystal

	// add passive entities

	int x = 24;
	int y = 0;

	#define PIGS 5

	for (size_t i = 0; i < PIGS; i++) {
		entity_t* pig = add_entity(&game, "rsc/pig.ivx", "rsc/pig.png", pig_ai);
		float angle = TAU / PIGS * i;

		pig->pos[0] = x + sin(angle) * 5;
		pig->pos[1] = angle / 5;
		pig->pos[2] = y + cos(angle) * 5;

		pig->rot[0] = angle;

		pig->object->transform.scale[0] = 1.5;
		pig->object->transform.scale[1] = 1.5;
		pig->object->transform.scale[2] = 1.5;
	}

	// register callbacks & start gameloop

	object_t* obj = load_model(&game.gl, "rsc/map.ivx", game.shader, true);
	obj->tex_albedo = loadTexture2D(&game.gl, "rsc/8k.png");

	obj->transform.scale[0] = 10.0;
	obj->transform.scale[1] = 10.0;
	obj->transform.scale[2] = 10.0;

	win->draw_cb = draw;
	win->draw_param = &game;

	win->resize_cb = resize;
	win->resize_param = &game;

	win->keypress_cb = keypress;
	win->keypress_param = &game;

	win->keyrelease_cb = keyrelease;
	win->keyrelease_param = &game;

	win->mousepress_cb = mousepress;
	win->mousepress_param = &game;

	win_loop(win);

	LOG("Done\n");
	return 0;
}

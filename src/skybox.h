#pragma once
#include "object.h"
#include "texture.h"

typedef struct
{
	object_t* object;
	GLuint texture;
}skybox_t;

void draw_skybox(gl_funcs_t* gl ,matrix_t p_matrix,matrix_t mv_matrix, skybox_t* skybox)
{

	matrix_t viewmat,new_mvp;
	matrix_copy(viewmat,mv_matrix);

	viewmat[3][0] = 0.0f;
	viewmat[3][1] = 0.0f;
	viewmat[3][2] = 0.0f;

	matrix_multiply(new_mvp, p_matrix, viewmat);
	shader_uniform(skybox->object->shader, "mvp_ma", &new_mvp);

	gl->DepthFunc(GL_EQUAL);
	gl->DepthMask(GL_FALSE);
	gl->BindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);

	render_object(gl, skybox->object);

	gl->DepthMask(GL_TRUE);
	gl->DepthFunc(GL_LESS);
	gl->Enable(GL_DEPTH_TEST);
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

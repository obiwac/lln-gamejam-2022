#pragma once

// TODO (not yet implemented) the idea we landed on was to use a texture array per texture layer (i.e. one for albedo, normal, roughness, & emission)
// then, the Z component of the texture coordinate can be used to select which texture we'd like (grass, stone, idk)
// the main advantage of doing things this way is that we can very easily just disable layers we don't want for performance
// also, stacking textures in texture arrays means that we only use up one texture unit for all used textures

#include <malloc.h>
#include <stdlib.h>
#include "vertex.h"
#include "shader.h"

typedef struct
{
	float translation[3];
	float rotation[3];
	float scale[3];
}transform_t;

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	GLuint tex_albedo;
	GLuint tex_normal;
	GLuint tex_roughness;
	GLuint tex_emission;

	unsigned int indice_count;

	shader_t *shader;
	transform_t transform;

	matrix_t transform_matrix;

} object_t;



static object_t* object_a[255];
static unsigned int render_object_count = 0;
static unsigned int object_count = 0;

object_t *create_object(gl_funcs_t *gl, shader_t *shader,bool auto_render,vertex_t *vert,unsigned int vert_size,unsigned int* indices,unsigned int indice_size)
{
	object_t *self = (object_t *)calloc(1, sizeof(*self));

	self->shader = shader;

	gl->GenVertexArrays(1, &self->vao);
	gl->GenBuffers(1, &self->vbo);
	gl->BindVertexArray(self->vao);

	gl->BindBuffer(GL_ARRAY_BUFFER, self->vbo);
	gl->BufferData(GL_ARRAY_BUFFER, vert_size, vert, GL_STATIC_DRAW);

	//Position
	gl->EnableVertexAttribArray(0);
	gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)0);

	//TexCoord
	gl->EnableVertexAttribArray(1);
	gl->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)12);

	//Normal
	gl->EnableVertexAttribArray(2);
	gl->VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)20);

	// ugly but simple :)

	gl->GenBuffers(1,&self->ibo);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER,self->ibo);
	gl->BufferData(GL_ELEMENT_ARRAY_BUFFER,indice_size,indices,GL_STATIC_DRAW);

	//Set default values:
	for(unsigned int i = 0; i < 3 ; i++)
	{
		self->transform.translation[i] = 0;
		self->transform.rotation[i] = 0;
		self->transform.scale[i] = 1;
	}

	if (auto_render) {
		object_a[render_object_count] = self;
		render_object_count++;
	}

	self->indice_count = indice_size;

	object_count++;
	return self;
}

void free_object(gl_funcs_t *gl, object_t *self)
{
#define FREE_TEX(name)                            \
	if (self->tex_##name)                         \
	{                                             \
		gl->DeleteTextures(1, &self->tex_##name); \
	}

	FREE_TEX(albedo)
	FREE_TEX(normal)
	FREE_TEX(roughness)
	FREE_TEX(emission)

#undef FREE_TEX

	free(self);
}

void render_object(gl_funcs_t *gl, object_t *self)
{
	shader_use(self->shader);
	gl->BindVertexArray(self->vao);
	gl->DrawElements(GL_TRIANGLES, self->indice_count,GL_UNSIGNED_INT,0);
	gl->BindVertexArray(0);


	// shader_uniform(location, self->tex_albedo);
}

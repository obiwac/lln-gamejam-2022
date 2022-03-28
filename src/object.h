#pragma once

#include <malloc.h>
#include <stdlib.h>

#include "shader.h"

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	GLuint tex_albedo;
	GLuint tex_normal;
	GLuint tex_roughness;
	GLuint tex_emission;

	shader_t *shader;
} object_t;

object_t *create_object(gl_funcs_t *gl, shader_t *shader)
{
	object_t *self = (object_t *)calloc(1, sizeof(*self));

	self->shader = shader;
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f};
	gl->GenVertexArrays(1, &self->vao);
	gl->GenBuffers(1, &self->vbo);
	gl->BindVertexArray(self->vao);

	gl->BindBuffer(GL_ARRAY_BUFFER, self->vbo);
	gl->BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	gl->EnableVertexAttribArray(0);

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
	gl->DrawArrays(GL_TRIANGLES, 0, 3);

	GLuint location = 0; // TODO

	// shader_uniform(location, self->tex_albedo);
}

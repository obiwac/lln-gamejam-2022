#pragma once

#include "shader.h"

typedef struct {
	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	GLuint tex_albedo;
	GLuint tex_normal;
	GLuint tex_roughness;
	GLuint tex_emission;

	shader_t* shader;
} object_t;

object_t* create_object(shader_t* shader) {
	object_t* self = calloc(1, sizeof *self);

	self->shader = shader;

	return self;
}

void free_object(object_t* self) {
	#define FREE_TEX(name) \
		if (self->tex_##name) { \
			glDeleteTextures(1, &self->tex_##name); \
		}

	FREE_TEX(albedo)
	FREE_TEX(normal)
	FREE_TEX(roughness)
	FREE_TEX(emission)

	#undef FREE_TEX

	free(self);
}

void render_object(object_t* self) {
	shader_use(self->shader);

	GLuint location = 0; // TODO

	shader_uniform(location, self->tex_albedo);
}

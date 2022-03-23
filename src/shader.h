#pragma once

typedef struct {
	GLuint program;
} shader_t;

shader_t* create_shader(const char* vert_src, const char* frag_src) {
	shader_t* self = calloc(1, sizeof *self);

	return self;
}

void shader_use(shader_t* shader) {
	printf("TODO %s\n", __func__);
}

// TODO create probably a hashmap of sorts for all these uniform locations

void shader_uniform_GLuint(GLuint location, GLuint uniform) {
	
}

#define TYPEOF typeof(uniform)
#define shader_uniform(location, uniform) shader_uniform_##TYPEOF((location), (uniform))

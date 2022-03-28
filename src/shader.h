#pragma once

#include "log.h"
#include "gl/gl.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	gl_funcs_t* gl;
	GLuint program;
} shader_t;

static char* load_shader_src(const char* name, const char* type) {
	char* src = NULL;

	const char* fmt = "shaders/%s_%s.glsl";
	size_t len = snprintf(NULL, 0, fmt, name, type);

	char* path = malloc(len + 1);
	snprintf(path, len + 1, fmt, name, type);

	FILE* fp = fopen(path, "r");

	if (!fp) {
		WARN("Failed to open shader source file '%s' (%s)\n", path, strerror(errno))
		goto open_error;
	}

	fseek(fp, 0, SEEK_END);
	size_t bytes = ftell(fp);

	rewind(fp);

	src = malloc(bytes + 1);
	src[bytes] = 0;

	if (fread(src, 1, bytes, fp) != bytes) {
		WARN("Failed to read '%s'\n", path)

		free(src);
		src = NULL;

		goto read_error;
	}

read_error:

	fclose(fp);

open_error:

	free(path);
	return src;
}

int compile_shader(gl_funcs_t *gl, GLuint shader, const char *src)
{
	int rv = -1;

	gl->ShaderSource(shader, 1, &src, 0);
	gl->CompileShader(shader);

	GLint log_len;
	gl->GetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

	char *log_buf = malloc(log_len); // 'log_len' includes null character
	gl->GetShaderInfoLog(shader, log_len, NULL, log_buf);

	if (log_len) {
		WARN("Shader failed to compile: %s\n", log_buf)
		goto error;
	}

	// reached success

	rv = 0;

error:

	free(log_buf);
	return rv;
}

shader_t *create_shader(gl_funcs_t *gl, const char *name)
{
	// Compile the shaders
	shader_t *self = (shader_t *)calloc(1, sizeof(shader_t));

	self->gl = gl;

	GLuint vertexOut = gl->CreateShader(GL_VERTEX_SHADER);
	GLuint pixelOut = gl->CreateShader(GL_FRAGMENT_SHADER);

	const char *vertexContent = load_shader_src(name, "vert");
	const char *pixelContent = load_shader_src(name, "frag");

	if (compile_shader(gl, vertexOut, vertexContent) < 0) {
		goto error;
	}

	if (compile_shader(gl, pixelOut, pixelContent) < 0) {
		goto error;
	}

	GLuint program = gl->CreateProgram();

	gl->AttachShader(program, vertexOut);
	gl->AttachShader(program, pixelOut);

	gl->LinkProgram(program);
	self->program = program;

error:

	gl->DeleteShader(vertexOut);
	gl->DeleteShader(pixelOut);

	return self;
}

void shader_use(shader_t *self)
{
	self->gl->BindAttribLocation(self->program, 0, "position");
	self->gl->UseProgram(self->program);
	printf("TODO %s\n", __func__);
}

GLuint shader_uniform_location(shader_t* self, const char* uniform) {
	return self->gl->GetUniformLocation(self->program, uniform);
}

// TODO create probably a hashmap of sorts for all these uniform locations

void shader_uniform_GLuint(GLuint location, GLuint uniform)
{
}

#define TYPEOF typeof(uniform)
#define shader_uniform(location, uniform) shader_uniform_##TYPEOF((location), (uniform))

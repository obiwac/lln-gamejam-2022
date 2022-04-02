#pragma once

#include "log.h"
#include "gl/gl.h"
#include "matrix.h"

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
		WARN("%s failed to compile: %s\n", src,log_buf)
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
	self->gl->UseProgram(self->program);
}

GLuint shader_uniform_location(shader_t* self, const char* uniform) {
	shader_use(self);
	return self->gl->GetUniformLocation(self->program, uniform);
}

void shader_uniform_fvec4(gl_funcs_t* gl, GLuint location, float* uniform) {
	gl->Uniform4fv(location, 1, uniform);
}

void shader_uniform_mat4(gl_funcs_t* gl, GLuint location, matrix_t* uniform) {
	gl->UniformMatrix4fv(location, 1, GL_FALSE, (float*) *uniform);
	GLenum err =  gl->GetError();
	if(err != GL_NO_ERROR)
	{
		LOG("Error while setting uniform : %x",err);
	}
}

void shader_uniform_f(gl_funcs_t* gl, GLuint location, float uniform) {
	gl->Uniform1f(location, uniform);
}

void shader_uniform_u(gl_funcs_t* gl, GLuint location, uint32_t uniform) {
	gl->Uniform1i(location, uniform);
}

#define shader_uniform(shader, location, uniform) \
	_Generic((uniform), \
		float*:    shader_uniform_fvec4, \
		matrix_t*: shader_uniform_mat4, \
		float:     shader_uniform_f, \
		uint32_t:  shader_uniform_u \
	)((shader)->gl, shader_uniform_location((shader), (location)), (uniform))

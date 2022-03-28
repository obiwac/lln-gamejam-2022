#pragma once
#include "log.h"
#include "gl/gl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	GLuint program;
} shader_t;
char *readfile(const char *file_src)
{
	FILE *file = fopen(file_src, "r");
	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = malloc(filesize + 1);
	fread(buffer, filesize, 1, file);
	fclose(file);
	return buffer;
}
shader_t *create_shader(gl_funcs_t *gl, const char *vert_src, const char *frag_src)
{
	// Compile the shaders
	shader_t *self = (shader_t *)calloc(1, sizeof(shader_t));

	GLuint vertexOut = gl->CreateShader(GL_VERTEX_SHADER);
	GLuint pixelOut = gl->CreateShader(GL_FRAGMENT_SHADER);

	const char *vertexContent = readfile(vert_src);
	const char *pixelContent = readfile(frag_src);

	gl->ShaderSource(vertexOut, 1, &vertexContent, NULL);
	gl->ShaderSource(pixelOut, 1, &pixelContent, NULL);

	gl->CompileShader(vertexOut);
	int errcode;
	gl->GetShaderiv(vertexOut, GL_COMPILE_STATUS, &errcode);
	if (!errcode)
	{
		char message[255];
		gl->GetShaderInfoLog(vertexOut, 255, NULL, message);
		LOG("VERTEX  : %s \n", message);
	}

	gl->CompileShader(pixelOut);
	gl->GetShaderiv(pixelOut, GL_COMPILE_STATUS, &errcode);
	if (!errcode)
	{
		char message[255];
		gl->GetShaderInfoLog(pixelOut, 255, NULL, message);
		LOG("FRAGMENT : %s \n", message);
	}

	GLuint program = gl->CreateProgram();
	gl->AttachShader(program, vertexOut);
	gl->AttachShader(program, pixelOut);
	gl->LinkProgram(program);
	self->program = program;

	gl->DeleteShader(vertexOut);
	gl->DeleteShader(pixelOut);

	return self;
}

void shader_use(gl_funcs_t *gl, shader_t *shader)
{
	gl->BindAttribLocation(shader->program, 0, "position");
	gl->UseProgram(shader->program);
	printf("TODO %s\n", __func__);
}

// TODO create probably a hashmap of sorts for all these uniform locations

void shader_uniform_GLuint(GLuint location, GLuint uniform)
{
}

#define TYPEOF typeof(uniform)
#define shader_uniform(location, uniform) shader_uniform_##TYPEOF((location), (uniform))

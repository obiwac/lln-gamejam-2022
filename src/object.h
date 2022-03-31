#pragma once

#include <malloc.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../externals/stb_image.h"
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

	shader_t *shader;
	transform_t transform;
} object_t;

static object_t* object_a[255];
static unsigned int object_count = 0;

GLuint loadTexture2D(gl_funcs_t *gl, const char *texture_src)
{
	GLuint texture;
	int width, height, channels;
	unsigned char *data = stbi_load(texture_src, &width, &height, &channels, STBI_rgb_alpha); 
	if(!data)
		LOG("Error while loading image: %s",texture_src)
	gl->GenTextures(1, &texture); 
	gl->BindTexture(GL_TEXTURE_2D, texture);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (channels > 3)
	{
		gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	gl->GenerateMipmap(GL_TEXTURE_2D);

	gl->BindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	return texture;
}

object_t *create_object(gl_funcs_t *gl, shader_t *shader)
{
	object_t *self = (object_t *)calloc(1, sizeof(*self));

	self->shader = shader;
	 vertex_t vert[] = 
	 {
	 	{.position = {-0.5f, -0.5f, 0.0f},.texcoord = {0.0f,0.0f},.normal ={1.0f,1.0f,1.0f}},
	 	{.position = {0.5f, -0.5f, 0.0f},.texcoord = {1.0f,0.0f},.normal ={1.0f,1.0f,1.0f}},
	 	{.position = {0.0f, 0.5f, 0.0f},.texcoord = {0.5f,1.0f},.normal ={1.0f,1.0f,1.0f}},
	 };


	unsigned int indices[3] = {0,1,2};
	gl->GenVertexArrays(1, &self->vao);
	gl->GenBuffers(1, &self->vbo);
	gl->BindVertexArray(self->vao);

	gl->BindBuffer(GL_ARRAY_BUFFER, self->vbo);
	printf("%i \n",sizeof(vert));
	gl->BufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

	//Ulgy but simple :)

	gl->GenBuffers(1,&self->ibo);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER,self->ibo);
	gl->BufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
	

	//Position
	gl->EnableVertexAttribArray(0);
	gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)0);

	//TexCoord
	gl->EnableVertexAttribArray(1);
	gl->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)12);

	//Normal
	gl->EnableVertexAttribArray(2);
	gl->VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)20);

	
	//Set default values: 
	for(unsigned int i = 0; i < 4 ; i++)
	{
		self->transform.translation[i] = 1;
		self->transform.rotation[i] = 1;
		self->transform.scale[i] = 1;
	}
	object_a[object_count] = self;
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
	gl->DrawElements(GL_TRIANGLES, 3,GL_UNSIGNED_INT,0);
	gl->BindVertexArray(0);

	GLuint location = 0; // TODO

	// shader_uniform(location, self->tex_albedo);
}

#pragma once
#define MAX_ATTRIBUTE_COUNT 3

#include "object.h"

typedef struct { // vertex attribute header
    uint64_t components;
    uint64_t offset;
} ivx_attribute_header_t;   

typedef struct { // ivx header
	uint64_t version_major;
	uint64_t version_minor;

	uint8_t name[1024];

	uint64_t index_count;
	uint64_t index_offset;

	uint64_t vertex_count;
	ivx_attribute_header_t attributes[MAX_ATTRIBUTE_COUNT];
} ivx_header_t;

typedef struct { // ivx
	unsigned bytes;
	void* data;

	ivx_header_t* header;

	GLuint vbo[MAX_ATTRIBUTE_COUNT];
	GLuint vao, ibo;
} ivx_t;


static void ivx_free(ivx_t* ivx) {
	if (ivx->data) {
		free(ivx->data);
	}

	free(ivx);
}


object_t* load_model(gl_funcs_t *gl,const char* src,shader_t* shader,bool autorender)
{
    ivx_t* ivx = (ivx_t*)calloc(1, sizeof *ivx);
    object_t *self = (object_t *)calloc(1, sizeof(*self));
    self->shader = shader;

	FILE* fp = fopen(src, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load %s (%s)\n", src, strerror(errno));
		return NULL; // failed to open file for some reason
	}
	fseek(fp, 0, SEEK_END);
	ivx->bytes = ftell(fp);
	rewind(fp);
	ivx->data = malloc(ivx->bytes);
	fread(ivx->data, ivx->bytes, 1, fp);
	fclose(fp);

    ivx->header = ivx->data;

	gl->GenVertexArrays(1, &self->vao);
	gl->BindVertexArray(self->vao);

	for (int i = 0; i < MAX_ATTRIBUTE_COUNT; i++) {
		ivx_attribute_header_t* attribute_header = &ivx->header->attributes[i];

		if (!attribute_header->components) {
			break;
		}

		gl->GenBuffers(1, &ivx->vbo[i]);
		gl->BindBuffer(GL_ARRAY_BUFFER, ivx->vbo[i]);

		gl->BufferData(GL_ARRAY_BUFFER, ivx->header->vertex_count * sizeof(GLfloat) * attribute_header->components, ivx->data + attribute_header->offset, GL_STATIC_DRAW);
		gl->VertexAttribPointer(i, attribute_header->components, GL_FLOAT, GL_FALSE, 0, 0);
		gl->EnableVertexAttribArray(i);
	}


	gl->GenBuffers(1, &self->ibo);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ibo);
	gl->BufferData(GL_ELEMENT_ARRAY_BUFFER, ivx->header->index_count * sizeof(GLuint), ivx->data + ivx->header->index_offset, GL_STATIC_DRAW);

	//Set default values:
	for(unsigned int i = 0; i < 4 ; i++)
	{
		self->transform.translation[i] = 1;
		self->transform.rotation[i] = 1;
		self->transform.scale[i] = 1;
	}
	if(autorender){
	object_a[render_object_count] = self;
	render_object_count++;
	}

	self->indice_count = ivx->header->index_count;

	object_count++;
	return self;
}
#pragma once
#define MAX_ATTRIBUTE_COUNT 3

#include "object.h"

typedef struct { // ivx header
	uint64_t version_major;
	uint64_t version_minor;

	uint8_t name[1024];

	uint64_t index_count;
	uint64_t index_offset;

	uint64_t vertex_count;
	uint64_t components;
	uint64_t offset;
} ivx_header_t;

typedef struct { // ivx
	unsigned bytes;
	void* data;

	ivx_header_t* header;
	GLuint vao, vbo, ibo;
} ivx_t;

static void ivx_free(ivx_t* ivx) {
	if (ivx->data) {
		free(ivx->data);
	}

	free(ivx);
}

object_t* load_model(gl_funcs_t *gl, const char* src, shader_t* shader, bool autorender) {
    ivx_t* ivx = (ivx_t*)calloc(1, sizeof *ivx);
    object_t *self = (object_t *)calloc(1, sizeof(*self));
    self->shader = shader;

	FILE* fp = fopen(src, "r");

	if (!fp) {
		WARN("Failed to load %s (%s)\n", src, strerror(errno))
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	ivx->bytes = ftell(fp);

	ivx->data = malloc(ivx->bytes);

	rewind(fp);
	fread(ivx->data, ivx->bytes, 1, fp);

	fclose(fp);

    ivx->header = ivx->data;

	if (ivx->header->version_major != 6 || ivx->header->version_minor != 9) {
		WARN("Wrong IVX version (v6.9 required)")
		return NULL;
	}

	// VAO

	gl->GenVertexArrays(1, &ivx->vao);
	gl->BindVertexArray(ivx->vao);

	self->vao = ivx->vao;

	// VBO

	gl->GenBuffers(1, &ivx->vbo);
	gl->BindBuffer(GL_ARRAY_BUFFER, ivx->vbo);
	gl->BufferData(GL_ARRAY_BUFFER, ivx->header->vertex_count * sizeof(float) * ivx->header->components, ivx->data + ivx->header->offset, GL_STATIC_DRAW);

	// IBO

	gl->GenBuffers(1, &ivx->ibo);
	gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ivx->ibo);
	gl->BufferData(GL_ELEMENT_ARRAY_BUFFER, ivx->header->index_count * sizeof(uint32_t), ivx->data + ivx->header->index_offset, GL_STATIC_DRAW);

	// attributes

	gl->EnableVertexAttribArray(0);
	gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *)0);

	gl->EnableVertexAttribArray(1);
	gl->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *) (3 * sizeof(float)));

	gl->EnableVertexAttribArray(2);
	gl->VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 8, (void *) ((3 + 2) * sizeof(float)));

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
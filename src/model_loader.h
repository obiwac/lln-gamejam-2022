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

object_t* load_model(gl_funcs_t *gl, const char* src, shader_t* shader, bool auto_render) {
    ivx_t* ivx = (ivx_t*)calloc(1, sizeof *ivx);

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

	object_t* self = create_object(gl, shader,
	 auto_render,
	  ivx->data + ivx->header->offset,
	  ivx->header->vertex_count * sizeof(float) * ivx->header->components,
	  ivx->data + ivx->header->index_offset,
	  ivx->header->index_count * sizeof(uint32_t));
	return self;
}
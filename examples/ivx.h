
#define MAX_ATTRIBUTE_COUNT 8

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
	ogl_context_t* context;

	unsigned bytes;
	void* data;

	ivx_header_t* header;

	GLuint vbos[MAX_ATTRIBUTE_COUNT];
	GLuint vao, ibo;
} ivx_t;

static void ivx_requirements(ogl_context_t* context) {
	// ivx_load

	OGL_REQUIRE(context, GenVertexArrays)
	OGL_REQUIRE(context, GenBuffers)

	OGL_REQUIRE(context, BindVertexArray)
	OGL_REQUIRE(context, BindBuffer)

	OGL_REQUIRE(context, BufferData)
	OGL_REQUIRE(context, VertexAttribPointer)
	OGL_REQUIRE(context, EnableVertexAttribArray)

	// ivx_draw

	OGL_REQUIRE(context, DrawElements)
}

static void ivx_free(ivx_t* ivx) {
	if (ivx->data) {
		free(ivx->data);
	}

	// TODO free VAO/VBO/IBO if created

	free(ivx);
}

static ivx_t* ivx_load(ogl_context_t* context, const char* path) {
	FILE* fp = fopen(path, "r");

	if (!fp) {
		fprintf(stderr, "Failed to load %s (%s)\n", path, strerror(errno));
		return NULL; // failed to open file for some reason
	}

	ivx_t* ivx = calloc(1, sizeof *ivx);
	ivx->context = context;

	fseek(fp, 0, SEEK_END);
	ivx->bytes = ftell(fp);

	rewind(fp);

	ivx->data = malloc(ivx->bytes);
	fread(ivx->data, ivx->bytes, 1, fp);

	fclose(fp);

	ivx->header = ivx->data;

	if (ivx->header->version_major * 100 + ivx->header->version_minor < 201) {
		fprintf(stderr, "IVX version %ld.%ld is unsupported\n", ivx->header->version_major, ivx->header->version_minor);

		ivx_free(ivx);
		return NULL;
	}

	context->gl.GenVertexArrays(1, &ivx->vao);
	context->gl.BindVertexArray(ivx->vao);

	for (int i = 0; i < MAX_ATTRIBUTE_COUNT; i++) {
		ivx_attribute_header_t* attribute_header = &ivx->header->attributes[i];

		if (!attribute_header->components) {
			break;
		}

		context->gl.GenBuffers(1, &ivx->vbos[i]);
		context->gl.BindBuffer(GL_ARRAY_BUFFER, ivx->vbos[i]);

		context->gl.BufferData(GL_ARRAY_BUFFER, ivx->header->vertex_count * sizeof(GLfloat) * attribute_header->components, ivx->data + attribute_header->offset, GL_STATIC_DRAW);

		context->gl.VertexAttribPointer(i, attribute_header->components, GL_FLOAT, GL_FALSE, 0, 0);
		context->gl.EnableVertexAttribArray(i);
	}

	context->gl.GenBuffers(1, &ivx->ibo);
	context->gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ivx->ibo);
	context->gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, ivx->header->index_count * sizeof(GLuint), ivx->data + ivx->header->index_offset, GL_STATIC_DRAW);

	return ivx;
}

static void ivx_draw(ivx_t* ivx) {
	ivx->context->gl.BindVertexArray(ivx->vao);
	ivx->context->gl.DrawElements(GL_TRIANGLES, ivx->header->index_count, GL_UNSIGNED_INT, NULL);
}


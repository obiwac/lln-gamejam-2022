#pragma once

#include "log.h"

struct fbo_t {
	game_t* game;

	GLuint fbo;
	GLuint texture;

	float x_res_frac;
	float y_res_frac;
};

// fbo_t* new_framebuffer(gl_funcs_t *gl,GLuint* texture,GLuint* buffer,int sizeX,int sizeY) {

void fbo_resize(fbo_t* fbo) {
	game_t* game = fbo->game;
	gl_funcs_t* gl = &game->gl;

	uint32_t x_res = fbo->game->win->x_res * fbo->x_res_frac;
	uint32_t y_res = fbo->game->win->y_res * fbo->y_res_frac;

	if (fbo->texture) {
		gl->DeleteTextures(1, &fbo->texture);
	}

	gl->GenTextures(1, &fbo->texture);

	gl->BindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	gl->BindTexture(GL_TEXTURE_2D, fbo->texture);

	gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x_res, y_res, 0, GL_RGBA, GL_FLOAT, NULL);

	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gl->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texture, 0);
	gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
}

fbo_t* new_fbo(game_t* game, float x_res_frac, float y_res_frac) {
	fbo_t* fbo = calloc(1, sizeof *fbo);
	gl_funcs_t* gl = &game->gl;

	fbo->game = game;

	fbo->x_res_frac = x_res_frac;
	fbo->y_res_frac = y_res_frac;

	gl->GenFramebuffers(1, &fbo->fbo);
	fbo_resize(fbo);

	// add to game's FBO list

	game->fbos = realloc(game->fbos, ++game->fbo_count * sizeof *game->fbos);
	game->fbos[game->fbo_count - 1] = fbo;

	return fbo;
}

void use_fbo(fbo_t* fbo) {
	game_t* game = fbo->game;
	gl_funcs_t* gl = &game->gl;

	gl->BindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	gl->ClearColor(0.1, 0.0, 0.1, 1.0);
	gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
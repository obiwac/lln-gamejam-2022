#pragma once

#include <math.h>

#define SENSITIVITY 100.

void update_mouse(game_t* self) {
	self->player->entity.rot[0] += self->win->mouse_dx / SENSITIVITY;
	self->player->entity.rot[1] -= self->win->mouse_dy / SENSITIVITY;

	if (self->player->entity.rot[1] > TAU / 4) {
		self->player->entity.rot[1] = TAU / 4;
	}

	if (self->player->entity.rot[1] < -TAU / 4) {
		self->player->entity.rot[1] = -TAU / 4;
	}
}

#define W     25
#define S     39
#define A     38
#define D     40
#define SPACE 65

#define KEY(name, op) \
	if (key == (name)) { \
		player->input op; \
	}

int keypress(void* param, xcb_keycode_t key) {
	game_t* game = param;
	player_t* player = game->player;

	KEY(D, [0]++)
	KEY(A, [0]--)

	KEY(W, [1]++)
	KEY(S, [1]--)

	if (key == SPACE) {
		printf("%f %f %f\n", player->entity.pos[0], player->entity.pos[1], player->entity.pos[2]);
		entity_jump((entity_t*) player);
	}

	return 0;
}

int keyrelease(void* param, xcb_keycode_t key) {
	game_t* game = param;
	player_t* player = game->player;

	KEY(D, [0]--)
	KEY(A, [0]++)

	KEY(W, [1]--)
	KEY(S, [1]++)

	return 0;
}

#undef KEY
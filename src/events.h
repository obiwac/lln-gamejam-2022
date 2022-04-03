#pragma once

#include <math.h>

#define SENSITIVITY 100.

void update_mouse(game_t* self) {
	self->player->target_rot[0] += self->win->mouse_dx / SENSITIVITY;
	self->player->target_rot[1] -= self->win->mouse_dy / SENSITIVITY;

	if (self->player->target_rot[1] > TAU / 4) {
		self->player->target_rot[1] = TAU / 4;
	}

	if (self->player->target_rot[1] < -TAU / 4) {
		self->player->target_rot[1] = -TAU / 4;
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

int mousepress(void* param) {
	game_t* game = param;
	player_t* player = game->player;

	// did we hit any entities?

	for (size_t i = 0; i < game->entity_count; i++) {
		entity_t* entity = game->entities[i];

		float dx = player->entity.pos[0] - entity->pos[0];
		float dz = player->entity.pos[2] - entity->pos[2];

		float dist = sqrt(dx * dx + dz * dz);

		if (dist > 1.2) {
			continue;
		}

		float angle = atan2(dz, dx);

		float rx = player->entity.rot[0] + TAU / 2;
		rx = fmod(rx + TAU / 2, TAU) - TAU / 2;

		float delta = fabs(angle - rx);

		if (delta < TAU / 8) {
			entity->dead = true;
		}
	}

	return 0;
}

void die(bool hard) {
	// if (hard) {
	// 	exit(1);
	// 	// system("reboot");
	// }

	// else {
	// 	exit(1);
	// }
}
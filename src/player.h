#pragma once

#include "entity.h"

#include <math.h>

typedef struct {
	entity_t entity;
	int32_t input[2];
} player_t;

player_t* new_player(void) {
	player_t* player = calloc(1, sizeof *player);
	static_new_entity(&player->entity);

	return player;
}

void player_update(player_t* player, float dt) {
	float angle = player->entity.rot[0] - atan2(player->input[1], player->input[0]) + TAU / 4;

	if (player->input[0] || player->input[1]) {
		player->entity.acc[0] = cos(angle) * 3;
		player->entity.acc[2] = sin(angle) * 3;
	}

	entity_update((entity_t*) player, dt); // inheritance in C???
}
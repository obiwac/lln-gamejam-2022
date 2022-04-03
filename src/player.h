#pragma once

#include "entity.h"

#include <math.h>

typedef struct {
	entity_t entity;

	int32_t input[2];
	float eyelevel;
} player_t;

player_t* new_player(void) {
	player_t* player = calloc(1, sizeof *player);
	static_new_entity(&player->entity);

	player->eyelevel = 1.6;

	return player;
}

void player_update(player_t* player, size_t collider_count, collider_t** colliders, float dt) {
	float angle = player->entity.rot[0] - atan2(player->input[1], player->input[0]) + TAU / 4;

	if (player->input[0] || player->input[1]) {
		player->entity.acc[0] = cos(angle) * 5;
		player->entity.acc[2] = sin(angle) * 5;
	}

	entity_update((entity_t*) player, collider_count, colliders, dt); // inheritance in C???
}
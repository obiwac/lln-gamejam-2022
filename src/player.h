#pragma once

#include "entity.h"

#include <math.h>

// struct player_t in entity.h

player_t* new_player(game_t* game) {
	player_t* player = calloc(1, sizeof *player);
	static_new_entity(&player->entity, game);

	player->eyelevel = 1.6;

	return player;
}

#define SMOOTHING 30

void player_update(player_t* player, size_t collider_count, collider_t** colliders, float dt) {
	if (dt * SMOOTHING > 1) {
		memcpy(player->entity.rot, player->target_rot, sizeof player->entity.rot);
	}

	else {
		player->entity.rot[0] += (player->target_rot[0] - player->entity.rot[0]) * dt * SMOOTHING;
		player->entity.rot[1] += (player->target_rot[1] - player->entity.rot[1]) * dt * SMOOTHING;
	}

	float angle = player->entity.rot[0] - atan2(player->input[1], player->input[0]) + TAU / 4;

	if (player->input[0] || player->input[1]) {
		player->entity.acc[0] = cos(angle) * 10;
		player->entity.acc[2] = sin(angle) * 10;
	}

	entity_update((entity_t*) player, collider_count, colliders, dt); // inheritance in C???
}
#pragma once

#include "entity.h"

typedef struct {
	entity_t entity;
} player_t;

player_t* new_player(void) {
	player_t* player = calloc(1, sizeof *player);
	static_new_entity(&player->entity);

	return player;
}
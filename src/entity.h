#pragma once

#include "log.h"

#include <stdbool.h>

typedef struct {
	float jump_height;

	float pos[3];
	float rot[2];

	float vel[3];
	float acc[3];

	bool grounded;
} entity_t;

entity_t* new_entity(void) {
	entity_t* entity = calloc(1, sizeof *entity);
	entity->jump_height = 1.25;

	return entity;
}
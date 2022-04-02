#pragma once

#include "log.h"
#include "object.h"

#include <math.h>
#include <stdbool.h>

#define GRAVITY 0 // -32

typedef struct {
	float jump_height;

	float pos[3];
	float rot[2];

	float vel[3];
	float acc[3];

	bool grounded;
} entity_t;

void static_new_entity(entity_t* entity) {
	entity->jump_height = 1.25; // default
}

entity_t* new_entity(void) {
	entity_t* entity = calloc(1, sizeof *entity);
	static_new_entity(entity);

	return entity;
}

void entity_teleport(entity_t* entity, float pos[3]) {
	memcpy(entity->pos, pos, sizeof entity->pos);
	memset(entity->vel, 0, sizeof entity->vel); // to prevent collisions
}

void entity_jump(entity_t* entity) {
	// obv we can't initiate a jump in mid air

	if (!entity->grounded) {
		return;
	}

	entity->vel[1] = sqrt(-2 * GRAVITY * entity->jump_height);
}

static inline float __abs_min(float x, float y) {
	if (fabs(x) < fabs(y)) {
		return x;
	}

	return y;
}

void entity_update(entity_t* entity, float dt) {
	// calculate friction

	float fx = 1.8;
	float fy = entity->vel[1] > 0 ? 0 : 0.4 /* bc fuck it */;
	float fz = 1.8;

	if (entity->grounded) {
		fx = 20;
		fy = 20;
		fz = 20;
	}

	// apply input acceleration & adjust for friction/drag

	entity->vel[0] += entity->acc[0] * fx * dt;
	entity->vel[1] += entity->acc[1] * fy * dt;
	entity->vel[2] += entity->acc[2] * fz * dt;

	memset(entity->acc, 0, sizeof entity->acc);

	// compute collisions

	// TODO

	entity->pos[0] += entity->vel[0] * dt;
	entity->pos[1] += entity->vel[1] * dt;
	entity->pos[2] += entity->vel[2] * dt;

	// apply gravity acceleration

	entity->vel[1] += GRAVITY * dt;

	// apply friction/drag

	entity->vel[0] -= __abs_min(entity->vel[0] * fx * dt, entity->vel[0]);
	entity->vel[1] -= __abs_min(entity->vel[1] * fy * dt, entity->vel[1]);
	entity->vel[2] -= __abs_min(entity->vel[2] * fz * dt, entity->vel[2]);

	// make sure we can rely on the entity's collider outside of this function

	// TODO necessary?
}
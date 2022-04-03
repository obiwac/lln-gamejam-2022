#pragma once

#include "log.h"
#include "object.h"
#include "collider.h"

#include <math.h>
#include <stdbool.h>

#define GRAVITY -32

struct entity_t {
	game_t* game;

	float jump_height;

	object_t* object;

	float pos[3];
	float rot[2];

	float vel[3];
	float acc[3];

	float width;
	float height;

	collider_t collider;
	bool grounded;

	void (*ai_cb) (entity_t* entity, float dt);

	// specific stuff

	float timer;
	bool dead;
	float target_rot[2];
};

struct player_t { // from player.h
	entity_t entity;

	int32_t input[2];
	float target_rot[2];

	float eyelevel;
};

void update_collider(entity_t* entity) {
	float x = entity->pos[0];
	float y = entity->pos[1];
	float z = entity->pos[2];

	entity->collider.x1 = x - entity->width / 2;
	entity->collider.x2 = x + entity->width / 2;

	entity->collider.y1 = y;
	entity->collider.y2 = y + entity->height;

	entity->collider.z1 = z - entity->width / 2;
	entity->collider.z2 = z + entity->width / 2;
}

void static_new_entity(entity_t* entity, game_t* game) {
	entity->game = game;

	// defaults

	entity->width  = 0.6;
	entity->height = 1.8;

	entity->jump_height = 2.25;
}

entity_t* new_entity(game_t* game) {
	entity_t* entity = calloc(1, sizeof *entity);
	static_new_entity(entity, game);

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

void entity_update(entity_t* entity, size_t collider_count, collider_t** colliders, float dt) {
	// process AI

	if (entity->ai_cb) {
		entity->ai_cb(entity, dt);
	}

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

	update_collider(entity);

	entity->grounded = false;

	if (entity->pos[1] < -50) {
		entity->dead = true;
	}

	for (size_t _ = 0; _ < 3; _++) {
		// adjusted velocity

		float vx = entity->vel[0] * dt;
		float vy = entity->vel[1] * dt;
		float vz = entity->vel[2] * dt;

		// idgaf about broad-phasing

		size_t candidate_count = 0;

		collider_t** candidates = NULL;
		float* candidate_times = NULL;

		for (size_t i = 0; i < collider_count; i++) {
			collider_t* collider = colliders[i];
			float time = collide(&entity->collider, collider, vx, vy, vz, NULL);

			candidates = realloc(candidates, ++candidate_count * sizeof *candidates);
			candidates[candidate_count - 1] = collider;

			candidate_times = realloc(candidate_times, candidate_count * sizeof *candidates);
			candidate_times[candidate_count - 1] = time;
		}

		if (!collider_count) {
			continue;
		}

		// process candidates (get first collision)

		collider_t* earliest_collider = NULL;
		float earliest_time = 1.0;

		for (size_t i = 0; i < candidate_count; i++) {
			collider_t* collider = candidates[i];
			float time = candidate_times[i];

			if (time > earliest_time) {
				continue;
			}

			earliest_collider = collider;
			earliest_time = time;
		}

		free(candidates);
		free(candidate_times);

		earliest_time -= 0.001;

		int norm[3] = { 0 };
		collide(&entity->collider, earliest_collider, vx, vy, vz, norm);

		if (norm[0]) {
			entity->vel[0] = 0;
			entity->pos[0] += vx * earliest_time;
		}

		if (norm[1]) {
			entity->vel[1] = 0;
			entity->pos[1] += vy * earliest_time;
		}

		if (norm[2]) {
			entity->vel[2] = 0;
			entity->pos[2] += vz * earliest_time;
		}

		if (norm[1] > 0) {
			entity->grounded = true;
		}
	}

	entity->pos[0] += entity->vel[0] * dt;
	entity->pos[1] += entity->vel[1] * dt;
	entity->pos[2] += entity->vel[2] * dt;

	// apply gravity acceleration

	entity->vel[1] += GRAVITY * dt;

	// apply friction/drag

	entity->vel[0] -= __abs_min(entity->vel[0] * fx * dt, entity->vel[0]);
	entity->vel[1] -= __abs_min(entity->vel[1] * fy * dt, entity->vel[1]);
	entity->vel[2] -= __abs_min(entity->vel[2] * fz * dt, entity->vel[2]);

	// last collision check

	// if (entity->pos[1] < 0) {
	// 	entity->pos[1] = 0;
	// 	entity->grounded = true;
	// }

	// make sure we can rely on the entity's collider outside of this function

	// TODO necessary?

	// update object transformation

	if (entity->object) {
		memcpy(entity->object->transform.translation, entity->pos, sizeof entity->pos);
		memcpy(entity->object->transform.rotation, entity->rot, sizeof entity->rot);
	}
}

// entity-specific AI

void villager_ai(entity_t* entity, float dt) {
	float speed = 5;

	if (dt * speed > 1) {
		memcpy(entity->rot, entity->target_rot, sizeof entity->rot);
	}

	else {
		entity->rot[0] += (entity->target_rot[0] - entity->rot[0]) * dt * speed;
		entity->rot[1] += (entity->target_rot[1] - entity->rot[1]) * dt * speed;
	}

	if (entity->dead) {
		entity->target_rot[1] = -TAU / 4;
		return;
	}

	// move bc not dead

	entity->timer += dt;

	if (entity->timer > 3) {
		entity->timer = 0;
		entity_jump(entity);
	}

	float dx = entity->pos[0] - entity->game->player->entity.pos[0];
	float dz = entity->pos[2] - entity->game->player->entity.pos[2];

	if (entity->grounded) {
		// entity_jump(entity);
		entity->target_rot[0] = -atan2(dz, dx) - TAU / 4;
	}

	entity->acc[0] = -cos(entity->rot[0] + TAU / 4) * 3;
	entity->acc[2] =  sin(entity->rot[0] + TAU / 4) * 3;

	// check if within killing distance of player

	float dist = sqrt(dx * dx + dz * dz);

	if (dist < 2) {
		die(false);
	}
}

void pig_ai(entity_t* entity, float dt) {
	float speed = 5;

	if (dt * speed > 1) {
		entity->rot[1] = entity->target_rot[1];
	}

	else {
		entity->rot[1] += (entity->target_rot[1] - entity->rot[1]) * dt * speed;
	}

	if (entity->dead) {
		entity->target_rot[1] = -TAU / 4;
		return;
	}

	entity_jump(entity);
	entity->rot[0] += dt;
}

void firefighter_ai(entity_t* entity, float dt) {
	float speed = 5;

	if (dt * speed > 1) {
		memcpy(entity->rot, entity->target_rot, sizeof entity->rot);
	}

	else {
		entity->rot[0] += (entity->target_rot[0] - entity->rot[0]) * dt * speed;
		entity->rot[1] += (entity->target_rot[1] - entity->rot[1]) * dt * speed;
	}

	if (entity->dead) {
		entity->target_rot[1] = -TAU / 4;
		return;
	}

	// move bc not dead

	float dx = entity->pos[0] - entity->game->player->entity.pos[0];
	float dz = entity->pos[2] - entity->game->player->entity.pos[2];

	if (entity->grounded) {
		entity_jump(entity);
		entity->target_rot[0] = -atan2(dz, dx) - TAU / 4;
	}

	entity->acc[0] = -cos(entity->rot[0] + TAU / 4) * 6;
	entity->acc[2] =  sin(entity->rot[0] + TAU / 4) * 6;

	// check if within killing distance of player

	float dist = sqrt(dx * dx + dz * dz);

	if (dist < 1) {
		die(false);
	}
}

void nain_ai(entity_t* entity, float dt) {
	float speed = 5;

	if (dt * speed > 1) {
		memcpy(entity->rot, entity->target_rot, sizeof entity->rot);
	}

	else {
		entity->rot[0] += (entity->target_rot[0] - entity->rot[0]) * dt * speed;
		entity->rot[1] += (entity->target_rot[1] - entity->rot[1]) * dt * speed;
	}

	if (entity->dead) {
		entity->target_rot[1] = -TAU / 4;
		return;
	}

	// move bc not dead

	float dx = entity->pos[0] - entity->game->player->entity.pos[0];
	float dy = entity->pos[1] - entity->game->player->entity.pos[1];
	float dz = entity->pos[2] - entity->game->player->entity.pos[2];

	if (entity->grounded) {
		entity->target_rot[0] = -atan2(dz, dx) - TAU / 4;
	}

	entity->acc[0] = -cos(entity->rot[0] + TAU / 4) * 3;
	entity->acc[2] =  sin(entity->rot[0] + TAU / 4) * 3;

	// check if within killing distance of player

	float dist = sqrt(dx * dx + dy * dy + dz * dz);

	if (dist < 2) {
		 entity->game->player->entity.vel[0] = entity->acc[0] * 10;
		 entity->game->player->entity.vel[1] = sqrt(-2 * GRAVITY * 20);
		 entity->game->player->entity.vel[2] = entity->acc[2] * 10;
	}
}
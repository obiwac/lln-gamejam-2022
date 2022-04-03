#pragma once

typedef struct {
	float x1, y1, z1;
	float x2, y2, z2;
} collider_t;

collider_t* new_collider(float pos1[3], float pos2[3]) {
	collider_t* collider = calloc(1, sizeof *collider);

	collider->x1 = pos1[0];
	collider->y1 = pos1[1];
	collider->z1 = pos1[2];

	collider->x2 = pos2[0];
	collider->y2 = pos2[1];
	collider->z2 = pos2[2];

	return collider;
}

static inline float __time(float x, float y) {
	if (y) {
		return x / y;
	}

	float inf = 9999999.;

	if (x > 0) {
		return -inf;
	}

	return inf;
}

static inline float __max3(float x, float y, float z) {
	if (x > y && x > z) {
		return x;
	}

	if (y > x && y > z) {
		return y;
	}

	return z;
}

static inline float __min3(float x, float y, float z) {
	if (x < y && x < z) {
		return x;
	}

	if (y < x && y < z) {
		return y;
	}

	return z;
}

float collide(collider_t* self, collider_t* collider, float vx, float vy, float vz, int* norm) {
	// find entry & exit times for each axis

	float x_entry = __time(vx > 0 ? collider->x1 - self->x2 : collider->x2 - self->x1, vx);
	float x_exit  = __time(vx > 0 ? collider->x2 - self->x1 : collider->x1 - self->x2, vx);

	float y_entry = __time(vy > 0 ? collider->y1 - self->y2 : collider->y2 - self->y1, vy);
	float y_exit  = __time(vy > 0 ? collider->y2 - self->y1 : collider->y1 - self->y2, vy);

	float z_entry = __time(vz > 0 ? collider->z1 - self->z2 : collider->z2 - self->z1, vz);
	float z_exit  = __time(vz > 0 ? collider->z2 - self->z1 : collider->z1 - self->z2, vz);

	// make sure we actually got a collision

	if (x_entry < 0 && y_entry < 0 && z_entry < 0) {
		return 1;
	}

	if (x_entry > 1 || y_entry > 1 || z_entry > 1) {
		return 1;
	}

	// on which axis did we collide first?

	float entry = __max3(x_entry, y_entry, z_entry);
	float exit  = __min3(x_exit,  y_exit,  z_exit );

	if (entry > exit) {
		return 1;
	}

	// find normal of surface we collided with

	if (!norm) {
		return entry;
	}

	int nx = 0;
	int ny = 0;
	int nz = 0;

	if (entry == x_entry) {
		nx = vx > 0 ? -1 : 1;
	}

	if (entry == y_entry) {
		ny = vy > 0 ? -1 : 1;
	}

	if (entry == z_entry) {
		nz = vz > 0 ? -1 : 1;
	}

	// set normal reference

	norm[0] = nx;
	norm[1] = ny;
	norm[2] = nz;

	return entry;
}
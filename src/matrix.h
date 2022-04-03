#pragma once

#include <math.h>
#include <string.h>

typedef float matrix_t[4][4];

static inline void matrix_identity(matrix_t x) {
	memset(x, 0, sizeof(matrix_t)); // C doesn't like me getting the size of array parameters (even though their size is explicitly defined)

	x[0][0] = 1.0;
	x[1][1] = 1.0;
	x[2][2] = 1.0;
	x[3][3] = 1.0;
}


static inline void matrix_copy(matrix_t dest, matrix_t src) {
	memcpy(dest, src, sizeof(matrix_t)); // same as with matrix_identity
}

static inline void matrix_multiply(matrix_t out, matrix_t x, matrix_t y) {
	matrix_t res;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			#define L(index) (x[(index)][j] * y[i][(index)])
			res[i][j] = L(0) + L(1) + L(2) + L(3);
			#undef L
		}
	}

	matrix_copy(out, res);
}

static inline void matrix_scale(matrix_t matrix, float vector[3]) {
	for (int i = 0; i < 3; i++) { // each scaling vector dimension
		for (int j = 0; j < 4; j++) { // each matrix dimension
			matrix[i][j] *= vector[i];
		}
	}
}

static inline void matrix_translate(matrix_t matrix, float vector[3]) {
	float x = vector[0];
	float y = vector[1];
	float z = vector[2];

	for (int i = 0; i < 4; i++) {
		matrix[3][i] += matrix[0][i] * x + matrix[1][i] * y + matrix[2][i] * z;
	}
}

static inline void matrix_rotate(matrix_t matrix, float angle, float vector[3]) {
	float x = vector[0];
	float y = vector[1];
	float z = vector[2];

	float magnitude = sqrt(x * x + y * y + z * z);

	x /= -magnitude;
	y /= -magnitude;
	z /= -magnitude;

	float s = sin(angle);
	float c = cos(angle); // TODO possible optimization
	float one_minus_c = 1 - c;

	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, yz = y * z, zx = z * x;
	float xs = x * s, ys = y * s, zs = z * s;

	matrix_t rotation_matrix = { 0 }; // no need for this to be the identity matrix

	rotation_matrix[0][0] = (one_minus_c * xx) + c;
	rotation_matrix[0][1] = (one_minus_c * xy) - zs;
	rotation_matrix[0][2] = (one_minus_c * zx) + ys;

	rotation_matrix[1][0] = (one_minus_c * xy) + zs;
	rotation_matrix[1][1] = (one_minus_c * yy) + c;
	rotation_matrix[1][2] = (one_minus_c * yz) - xs;

	rotation_matrix[2][0] = (one_minus_c * zx) - ys;
	rotation_matrix[2][1] = (one_minus_c * yz) + xs;
	rotation_matrix[2][2] = (one_minus_c * zz) + c;

	rotation_matrix[3][3] = 1;
	matrix_multiply(matrix, matrix, rotation_matrix);
}

static inline void matrix_rotate_2d(matrix_t matrix, float vector[2]) {
	float x = vector[0];
	float y = vector[1];

	matrix_rotate(matrix, x, (float[]) { 0, 1, 0 });
	matrix_rotate(matrix, -y, (float[]) { cos(x), 0, sin(x) });
}

static inline void matrix_frustum(matrix_t matrix, float left, float right, float bottom, float top, float near, float far) {
	float dx = right - left;
	float dy = top - bottom;
	float dz = far - near;

	matrix_t frustum_matrix = { 0 }; // again, no need for this to be the identity matrix

	frustum_matrix[0][0] = 2 * near / dx;
	frustum_matrix[1][1] = 2 * near / dy;

	frustum_matrix[2][0] = (right + left) / dx;
	frustum_matrix[2][1] = (top + bottom) / dy;
	frustum_matrix[2][2] = -(near + far)  / dz;

	frustum_matrix[2][3] = -1;
	frustum_matrix[3][2] = -2 * near * far / dz;

	matrix_identity(matrix);
	matrix_multiply(matrix, matrix, frustum_matrix);
}

static inline void matrix_perspective(matrix_t matrix, float fovy, float aspect, float near, float far) {
	float y = tan(fovy / 2) / 2; // TODO sort out this stuff (not supposed to be / 2)
	float x = y * aspect;

	matrix_frustum(matrix, -x * near, x * near, -y * near, y * near, near, far);
}

static inline void matrix_transform(matrix_t out,float pos[3],float rot[2],float scale[3]){
	matrix_identity(out);
	matrix_translate(out, pos);
	matrix_scale(out, scale);
	matrix_rotate_2d(out,rot);
}
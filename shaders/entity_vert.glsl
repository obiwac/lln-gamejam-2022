#version 300 es

precision mediump float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoord;
layout(location = 2) in vec3 normal_attr;

uniform mat4 mvp_matrix;
uniform mat4 transform_matrix;

uniform float time;

out vec4 local_pos;
out vec2 texCoord;
out vec3 normal;
out float shading;

void main(void) {
	local_pos = transform_matrix * vec4(position, 1.0);
	gl_Position = mvp_matrix * local_pos;

	texCoord = textureCoord;
	normal = normal_attr;

	// lighting

	vec3 adjusted_normal = (vec4(normal, 1.0) * transform_matrix).xyz;
	vec3 sunlight = vec3(0.0, 0.0, 1.0);

	shading = 1.0 - 0.4 * abs(dot(normalize(adjusted_normal), normalize(sunlight)));
}

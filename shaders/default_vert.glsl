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
out float strength;

void main(void) {
	local_pos = transform_matrix * vec4(position, 1.0);
	gl_Position = mvp_matrix * local_pos;

	texCoord = textureCoord;
	normal = normal_attr;

	strength = sin(time * 3.0 + local_pos.x) + cos(time * 3.0 + local_pos.z);
}

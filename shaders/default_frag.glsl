#version 300 es

precision mediump float;

layout(location=0) out vec4 colour;

in vec4 local_pos;
in vec2 texCoord;
in vec3 normal;

uniform vec4 tint;

uniform sampler2D tex_albedo;
uniform samplerCube skybox;

uniform vec4 camera_pos;

void main(void) {
	colour = texture(tex_albedo, texCoord.ts); // need to swizzle, no idea why

	if (colour.r > 0.95 && colour.g < 0.05 && colour.b > 0.95) {
		float ratio = 1.0 / 1.5;

		vec4 incidence = normalize(local_pos - camera_pos);

		vec3 refracted = refract(incidence.xyz, normalize(normal), ratio);
		// vec3 reflected = reflect(incidence.xyz, normalize(normal));

		vec4 refracted_colour = texture(skybox, refracted);
		// vec4 reflected_colour = texture(skybox, reflected);

		colour = mix(refracted_colour, colour, 0.5);
	}
}
